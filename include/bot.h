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
        void load(std::string filename);

        std::string server;
        int port;
        std::string name;
        std::string password;
        std::string owner;
        metabot::avatar avatar;
        std::string current_room;

        /// Array containing janus server connections.
        std::map<std::string, class net *> janus_servers;
    };
}
#endif
