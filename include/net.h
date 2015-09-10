#ifndef METABOT_NET_H
#define METABOT_NET_H
namespace metabot 
{
    class net
    {
      public:
        net(std::string hostname, int port, int socket_type, bool ssl);
        ~net();
        const struct addrinfo *resolve(std::string hostname, int port);

        const struct addrinfo *ip;
        int port;
        int sock;
        bool ssl;
    };
}
#endif
