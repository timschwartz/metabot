#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
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
#include <net.h>

static bool SSL_init = false;

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

    net::net(std::string hostname, int port, int socket_type, bool ssl)
    {
#ifdef __WIN32__
        if(WSAStartup(MAKEWORD(2,2), &wsa_data)) throw std::string("Couldn't initialize winsock.");
#endif
        this->ip = this->resolve(hostname, port);
        this->port = port;
        this->ssl = ssl;
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

    }

    net::~net()
    {
#ifdef __WIN32__
        WSACleanup();
#endif
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
}
