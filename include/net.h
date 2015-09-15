#ifndef METABOT_NET_H
#define METABOT_NET_H
namespace metabot 
{
    class net
    {
      public:
        net(std::string hostname, int port, int socket_type, bool encrypted);
        ~net();
        const struct addrinfo *resolve(std::string hostname, int port);
        void thread();
        void thread_ssl();
        bool ssl_ctx();
        std::string read();
        void send(std::string message);
        void close();

        const struct addrinfo *ip;
        int port;
        int sock;
        bool encrypted;
        bool quit;
        SSL_CTX *ctx;
        SSL *ssl;
        std::thread t_net;
        std::list<std::string> input_buffer;
        BIO *certbio, *outbio;
        X509 *cert;
        X509_NAME *certname;
        const SSL_METHOD *method;
    };
}
#endif
