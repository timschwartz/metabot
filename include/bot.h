#ifndef BOT_H
#define BOT_H
namespace metabot
{
    class avatar
    {
      public:
        std::vector<float> pos;
        std::vector<float> dir;
        std::vector<float> view_dir;
        std::vector<float> up_dir;
        std::vector<float> head_pos;
        std::string userId;
        uint32_t last_update;

        /// Template for avatar
        std::string avatar_template;

        /// avatar_template merged with avatar data
        std::string avatar_file;
        avatar();
    };

    class bot
    {
      public:
        bot(std::string filename);
        ~bot();
        void thread();
        void update_pos();
        void update_avatar();
        void load(std::string filename);
        void logon(std::string room);
        void chat(std::string toUserId, std::string message);

        std::string server;
        int port;
        std::string name;
        std::string password;
        std::string owner;
        std::string follow;
        metabot::avatar avatar;
        std::string current_room;
        bool quit;
        std::thread bot_thread;

        /// Array containing janus server connections.
        std::map<std::string, class net *> janus_servers;

        std::map<std::string, class avatar *> players;

        script_map server_method;
    };
}
#endif
