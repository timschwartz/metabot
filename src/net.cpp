#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#ifdef __WIN32__
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif
#include <sys/types.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <thread>
#include <vector>
#include <list>

#include <metabot.h>
#include <net.h>

#ifdef __WIN32__
const char* inet_ntop(int af, const void* src, char* dst, int cnt){

    struct sockaddr_in srcaddr;

    memset(&srcaddr, 0, sizeof(struct sockaddr_in));
    memcpy(&(srcaddr.sin_addr), src, sizeof(srcaddr.sin_addr));

    srcaddr.sin_family = af;
    if (WSAAddressToString((struct sockaddr*) &srcaddr, sizeof(struct sockaddr_in), 0, dst, (LPDWORD) &cnt) != 0) {
        DWORD rv = WSAGetLastError();
        printf("WSAAddressToString() : %d\n",rv);
        return NULL;
    }
    return dst;
}

WSADATA wsa_data;
#endif

namespace metabot 
{
    static bool SSL_init = false;

    std::string ip_tostring(const struct addrinfo *ip)
    {
        void *sa;
        struct in6_addr IPv6_addr;
        struct in_addr IPv4_addr;
        std::string output;
        std::stringstream ss;
        char *s = new char[INET6_ADDRSTRLEN];

        switch(ip->ai_family)
        {
            case AF_INET6:
                sa = (struct sockaddr_in6 *)ip->ai_addr;
                IPv6_addr = ((struct sockaddr_in6 *)sa)->sin6_addr;
                inet_ntop(AF_INET6, &IPv6_addr, s, INET6_ADDRSTRLEN);
                output = s;
                break;
            case AF_INET:
                sa = (struct sockaddr_in *)ip->ai_addr;
                IPv4_addr = ((struct sockaddr_in *)sa)->sin_addr;
                inet_ntop(AF_INET, &IPv4_addr, s, INET6_ADDRSTRLEN);
                output = s;
                break;
        }

        return output;
    }

    bool init_ssl_lib()
    {
        if(SSL_init == false)
        {
            std::cout << "Initializing SSL." << std::endl;

            OpenSSL_add_all_algorithms();
            ERR_load_BIO_strings();
            ERR_load_crypto_strings();
            SSL_load_error_strings();

            if(SSL_library_init() < 0)
            {
                std::cout << "Could not initialize the OpenSSL library !" << std::endl;
            }
            else SSL_init = true;
        }

        return SSL_init;
    }


    net::net(std::string hostname, int port, int socket_type, bool encrypted)
    {
#ifdef __WIN32__
        if(WSAStartup(MAKEWORD(2,2), &wsa_data)) throw std::string("Couldn't initialize winsock.");
#endif
        this->ip = this->resolve(hostname, port);
        this->port = port;
        this->encrypted = encrypted;
        void *sa;
        struct in6_addr IPv6_addr;
        struct in_addr IPv4_addr;
    
        switch(this->ip->ai_family)
        {
            case AF_INET6:
                std::cout << "IPv6 ";
                if((this->sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) throw std::string("Couldn't create IPv6 socket.");
                break;
            case AF_INET:
                std::cout << "IPv4 ";
                if((this->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) throw std::string("Couldn't create IPv4 socket.");
                break;
        }

        if((connect(sock, this->ip->ai_addr, this->ip->ai_addrlen)) < 0)
        {
            printf("Couldn't connect to %s:%d. Error: %d\n", hostname.c_str(), port, errno);
            throw std::string("Error");
        }

        std::cout << "Connected to " << hostname << " [" << ip_tostring(this->ip) << "]:" << port << std::endl;

        if(encrypted)
        {
            if(!init_ssl_lib()) goto thread_start;
            if(!this->ssl_ctx()) goto thread_start;
            this->cert = SSL_get_peer_certificate(this->ssl);
            if(this->cert == NULL) std::cout << "Error: Could not get a certificate from: " << hostname << std::endl;

            this->certname = X509_NAME_new();
            this->certname = X509_get_subject_name(this->cert);

            X509_NAME_print_ex(this->outbio, this->certname, 0, 0);
            std::cout << std::endl;

            this->t_net = std::thread(&net::thread_ssl, this);
            this->t_net.detach();
        }
        else
        {
        thread_start:
            this->t_net = std::thread(&net::thread, this);
            this->t_net.detach();
        }
    }

    net::~net()
    {
        if(this->ssl) SSL_free(this->ssl);
        if(this->cert) X509_free(this->cert);
        if(this->ctx) SSL_CTX_free(this->ctx);
        if(this->sock) ::close(this->sock);

#ifdef __WIN32__
        WSACleanup();
#endif
    }

    void net::close()
    {
        shutdown(this->sock, 2);
        this->quit = true;
    }

    void net::thread_ssl()
    {
        std::cout << "Net::thread_ssl()" << std::endl;
        char buffer[6000];
        std::vector<std::string> commands;
        std::string partial_message;

        memset(buffer, 0, sizeof(buffer));

        this->quit = false;
        while(!this->quit)
        {
            if(::SSL_read(this->ssl, buffer, 6000) < 0) std::cout <<  "recv failed" << std::endl;

            partial_message += buffer;

            if(partial_message.size() && (partial_message.back() == '\n'))
            {
                commands = explode(partial_message.c_str(), '\n');
                for(auto n:commands) this->input_buffer.push_back(n);
                
                partial_message = "";
            }

            memset(buffer, 0, sizeof(buffer));
            usleep(250);
        }
    }


    void net::thread()
    {
        std::cout << "net::thread()" << std::endl;
        char buffer[6000];
        std::vector<std::string> commands;
        std::string partial_message;

        memset(buffer, 0, sizeof(buffer));

        this->quit = false;
        while(!this->quit)
        {
            if(::read(this->sock, buffer, 6000) < 0) std::cout <<  "recv failed" << std::endl;

            partial_message += buffer;

            if(partial_message.size() && (partial_message.back() == '\n'))
            {
                commands = metabot::explode(partial_message.c_str(), '\n');
                for(auto n:commands)
                {
                    this->input_buffer.push_back(n);
                }
                partial_message = "";
            }

            memset(buffer, 0, sizeof(buffer));
            usleep(250);
        }
    }

    bool net::ssl_ctx()
    {
        this->outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

        this->method = SSLv23_client_method();

        if((this->ctx = SSL_CTX_new(this->method)) == NULL)
        {
            BIO_printf(this->outbio, "Unable to create a new SSL context structure.\n");
            return false;
        }

        SSL_CTX_set_options(this->ctx, SSL_OP_NO_SSLv2);
        this->ssl = SSL_new(this->ctx);
        SSL_set_fd(this->ssl, this->sock);
        int ret;
        if(ret = SSL_connect(this->ssl) < 0)
        {
            std::cout << "Error: Could not build an SSL session." << std::endl;
            std::cout << SSL_get_error(this->ssl, ret) << std::endl;
            return false;
        }

        std::cout << "Successfully enabled SSL/TLS session." << std::endl;
        return true;
    }

    const struct addrinfo *net::resolve(std::string hostname, int port)
    {
        uint32_t i;

        struct addrinfo hints, *server, *temp = NULL;

        memset(&hints, 0, sizeof(hints));

        hints.ai_flags = 0;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;

        i = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &server);

        if(i)
        {
            printf("getaddrinfo() failed: %d\n", i);
            return NULL;
        }

        return server;
    }

    void net::send(std::string message)
    {
        message += '\n';
        if(this->encrypted) ::SSL_write(this->ssl, message.c_str(), message.size());
        else ::write(this->sock, message.c_str(), message.size());
    }

    std::string net::read()
    {
        std::string message = "";

        if(!this->input_buffer.size()) return message;
        message = this->input_buffer.front();
        this->input_buffer.pop_front();
        return message;
    }
}
