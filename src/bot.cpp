#include <iostream>
#include <fstream>
#include <vector>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <map>
#include <thread>
#include <list>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <unistd.h>
#include <cmath>

#ifdef __WIN32__
  #define NOCRYPT
  #include <mingw.thread.h>
#endif

#include <config.h>
#include <md5.h>
#include <metabot.h>
#include <net.h>
#include <bot.h>

namespace metabot
{
    avatar::avatar() {}

    void output(std::string text)
    {
        std::string color;
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[4096];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        color = "\033[1;30m";
        if(text[0] == ':') color = "\033[1;31m";
        if(text[0] == '|') color = "\033[1;34m";
        strftime(buffer, 4095, "%F %T", timeinfo);

        std::cout << buffer << " " << color << text << "\033[0m" <<std::endl;
    }

    namespace janusvr_server
    {
        void user_moved(bot *b, std::string command)
        {
            std::string pos_string, dir_string;
            std::string userId;
            float x, y , z, d;

            json_object *json_command, *temp, *temp2, *temp3;
            json_command = json_tokener_parse(command.c_str());
            if(!json_object_object_get_ex(json_command, "data", &temp)) output("No data!\n" + command);

            if(!json_object_object_get_ex(temp, "userId", &temp2))  output("No userId\n" + command);
            userId = json_object_get_string(temp2);

            if(!json_object_object_get_ex(temp, "position", &temp2))  output("No position\n" + command);

            if(!json_object_object_get_ex(temp2, "pos", &temp3))  output("No pos\n" + command);
            pos_string = json_object_get_string(temp3);

            if(!json_object_object_get_ex(temp2, "dir", &temp3))  output("No dir\n" + command);
            dir_string = json_object_get_string(temp3);

            if(b->players[userId] == NULL)
            {
                metabot::avatar *new_av = new metabot::avatar();
                b->players[userId] = new_av;
                b->players[userId]->userId = userId;
            }

            b->players[userId]->pos = metabot::stofv(pos_string);
            b->players[userId]->dir = metabot::stofv(dir_string);
            b->players[userId]->last_update = time(0);

            json_object_put(json_command);
            json_object_put(temp);
            json_object_put(temp2);
            json_object_put(temp3);
        }

        void user_chat(bot *b, std::string command)
        {
            std::string data, userId, toUserId, roomId, message;
            json_object *json_data, *temp;
            json_data = get_json_data(command);

            data = json_object_get_string(json_data);
            if(!json_object_object_get_ex(json_data, "userId", &temp)) return;
            userId = json_object_get_string(temp);

            if(json_object_object_get_ex(json_data, "roomId", &temp))
                roomId = json_object_get_string(temp);

            if(json_object_object_get_ex(json_data, "toUserId", &temp))
                toUserId = json_object_get_string(temp);

            if(!json_object_object_get_ex(json_data, "message", &temp)) return;
            message = json_object_get_string(temp);

            json_object_put(json_data);
            json_object_put(temp);

            if(toUserId.size()) output("| " + userId + "> " + message);
            else output(userId + "> " + message);
        }

        void okay(bot *b, std::string command)
        {
            output(":Okay");
        }

        void user_disconnected(bot *p, std::string command)
        {
            std::string data;
            json_object *json_data, *temp;

            json_data = get_json_data(command);
            if(!json_object_object_get_ex(json_data, "userId", &temp)) return;

            data = json_object_get_string(temp);
            json_object_put(json_data);
            json_object_put(temp);
            output(":"+data+" disconnected.");
        }

        void user_enter(bot *p, std::string command)
        {
            std::string data;
            json_object *json_data, *temp;

            json_data = get_json_data(command);
            if(!json_object_object_get_ex(json_data, "userId", &temp)) return;

            data = json_object_get_string(temp);
            json_object_put(json_data);
            json_object_put(temp);
            output(":"+data+" entered the room.");
        }

        void user_leave(bot *p, std::string command)
        {
            std::string data;
            json_object *json_data, *temp;

            json_data = get_json_data(command);
            if(!json_object_object_get_ex(json_data, "userId", &temp)) return;

            data = json_object_get_string(temp);
            json_object_put(json_data);
            json_object_put(temp);
            output(":"+data+" left the room.");
        }

        void error(bot *b, std::string command)
        {
            output(command);
        }

        void users_online(bot *b, std::string command)
        {
            json_object *json_data, *temp;

            json_data = get_json_data(command);
            output(json_object_get_string(json_data));

            json_object_put(json_data);
        }

        void user_portal(bot *b, std::string command)
        {
            std::string userId;
            std::string url;
            std::string roomId;
            json_object *json_data, *temp;

            json_data = get_json_data(command);

            if(!json_object_object_get_ex(json_data, "url", &temp)) return;
            url = json_object_get_string(temp);

            if(!json_object_object_get_ex(json_data, "userId", &temp)) return;
            userId = json_object_get_string(temp);

            if(!json_object_object_get_ex(json_data, "roomId", &temp)) return;
            roomId = json_object_get_string(temp);

            json_object_put(json_data);
            json_object_put(temp);

            output(":"+userId+" created a portal ("+md5(url)+") to "+url+".");
        }

    }

    bot::~bot()
    {
        std::cout << "Shutting down " << this->name << std::endl;
        for(auto & n: this->janus_servers)
        {
            std::cout << "Closing connection to " << n.first << std::endl;
            this->janus_servers[n.first]->close();
            delete this->janus_servers[n.first];
        }
        std::cout << "Bot destroyed" << std::endl;
    }

    bot::bot(std::string filename)
    {
        bool encrypted = true;
        std::cout << "Opening " << filename << std::endl;
        try
        {
            this->load(filename);
        } 
        catch (std::string e) 
        {
            throw e;
        }

        if(this->port != 5567) encrypted = false;
        this->quit = false;
        this->janus_servers[this->server] = new metabot::net(this->server, this->port, 0, encrypted);
        this->bot_thread = std::thread(&bot::thread, this);
        this->bot_thread.detach();
        this->logon(md5(this->current_room));
    }

    void bot::thread()
    {
        std::string message;

        while(!this->quit)
        {
            message = this->janus_servers[this->server]->read();
            if(message.size())
            {
                json_object *json_command = NULL, *temp = NULL;
                json_command = json_tokener_parse(message.c_str());

                std::string method;

                try
                {
                    if(!json_object_object_get_ex(json_command, "method", &temp))  throw "'method' not found in command: " + message;
                }
                catch(std::string e)
                {
                    output(e);
                    json_object_put(json_command);
                    json_object_put(temp);
                    return;
                }
                method = json_object_get_string(temp);

                json_object_put(json_command);
                json_object_put(temp);

                if(this->server_method[method] != NULL) (this->server_method[method])(this, message);
                else output("Unknown method " + method);
            }

            this->update_pos();
            usleep(200);
        }
    }

    void bot::load(std::string filename)
    {
        json_object *bot_data, *temp, *temp2;
        enum json_type type;

        std::fstream config_file;
        std::string config;

        config_file.open(filename, std::fstream::in | std::fstream::out);

        if(!config_file.good())
        {
            throw "Couldn't read config file " + filename;
        }

        config_file.seekg(0, std::ios::end);
        config.resize(config_file.tellg());
        config_file.seekg(0, std::ios::beg);
        config_file.read(&config[0], config.size());
        config_file.close();

        bot_data = json_tokener_parse(config.c_str());
        if(!json_object_object_get_ex(bot_data, "server", &temp)) throw "server not found in config file " + filename;
        this->server = json_object_get_string(temp);

        if(!json_object_object_get_ex(bot_data, "port", &temp)) throw "port not found in config file " + filename;
        this->port = json_object_get_int(temp);

        if(!json_object_object_get_ex(bot_data, "name", &temp)) throw "name not found in config file " + filename;
        this->name = json_object_get_string(temp);

        if(json_object_object_get_ex(bot_data, "password", &temp)) this->password = json_object_get_string(temp);

        if(!json_object_object_get_ex(bot_data, "owner", &temp)) throw "owner not found in config file " + filename;
        this->owner = json_object_get_string(temp);

        if(!json_object_object_get_ex(bot_data, "avatar_file", &temp)) throw "avatar_file not found in config file " + filename;
        std::string avatar_file_name = json_object_get_string(temp);

        if(!avatar_file_name.size())
        {
            throw "No avatar file specified in config file " + filename;
        }

        std::cout << "Opening avatar file " << avatar_file_name << std::endl;

        std::fstream avatar_file;
        avatar_file.open(avatar_file_name, std::fstream::in | std::fstream::out);

        if(!avatar_file.good())
        {
            throw "Couldn't read avatar file " + avatar_file_name;
        }

        avatar_file.seekg(0, std::ios::end);
        this->avatar.avatar_template.resize(avatar_file.tellg());
        avatar_file.seekg(0, std::ios::beg);
        avatar_file.read(&this->avatar.avatar_template[0], this->avatar.avatar_template.size());
        avatar_file.close();

        this->update_avatar();

        if(!json_object_object_get_ex(bot_data, "position", &temp)) throw "position not found in config file " + filename;

        if(!json_object_object_get_ex(temp, "pos", &temp2)) throw "pos not found in config file " + filename;
        this->avatar.pos = metabot::stofv(json_object_get_string(temp2));

        if(!json_object_object_get_ex(temp, "dir", &temp2)) throw "dir not found in config file " + filename;
        this->avatar.dir = metabot::stofv(json_object_get_string(temp2));

        if(!json_object_object_get_ex(temp, "view_dir", &temp2)) throw "view_dir not found in config file " + filename;
        this->avatar.view_dir = metabot::stofv(json_object_get_string(temp2));

        if(!json_object_object_get_ex(temp, "up_dir", &temp2)) throw "up_dir not found in config file " + filename;
        this->avatar.up_dir = metabot::stofv(json_object_get_string(temp2));

        if(!json_object_object_get_ex(temp, "head_pos", &temp2)) throw "head_pos not found in config file " + filename;
        this->avatar.head_pos = metabot::stofv(json_object_get_string(temp2));

        if(!json_object_object_get_ex(bot_data, "room", &temp)) throw "room not found in config file " + filename;
        this->current_room = json_object_get_string(temp);

        json_object_put(bot_data);
        json_object_put(temp);
        json_object_put(temp2);

        std::cout << "Bot '" << this->name << "' loaded." << std::endl;

        this->server_method.insert(std::make_pair("user_moved", &janusvr_server::user_moved));
        this->server_method.insert(std::make_pair("user_chat", &janusvr_server::user_chat));
        this->server_method.insert(std::make_pair("okay", &janusvr_server::okay));
        this->server_method.insert(std::make_pair("user_disconnected", &janusvr_server::user_disconnected));
        this->server_method.insert(std::make_pair("user_enter", &janusvr_server::user_enter));
        this->server_method.insert(std::make_pair("user_leave", &janusvr_server::user_leave));
        this->server_method.insert(std::make_pair("error", &janusvr_server::error));
        this->server_method.insert(std::make_pair("users_online", &janusvr_server::users_online));
        this->server_method.insert(std::make_pair("user_portal", &janusvr_server::user_portal));
        return;
    }

    void bot::update_pos()
    {
        float x, y, z;
        json_object *jobj = json_object_new_object();
        json_object *data = json_object_new_object();
        std::string method = "move";
 
        if(this->follow.size() && this->players[this->follow] != NULL)
        {
            this->avatar.pos = this->players[this->follow]->pos;
            this->avatar.pos[0] +=2;
            x = this->avatar.pos[0] - this->players[this->follow]->pos[0];
            y = this->avatar.pos[1] - this->players[this->follow]->pos[1];
            z = this->avatar.pos[2] - this->players[this->follow]->pos[2];
            this->avatar.dir[0] = -x; // this->players[this->follow]->dir;
            this->avatar.dir[1] = y;
            this->avatar.dir[2] = z;
        }

        json_object_object_add(jobj, "method", json_object_new_string(method.c_str()));
        json_object_object_add(data, "pos", json_object_new_string(implode(this->avatar.pos, ' ').c_str()));
        json_object_object_add(data, "dir", json_object_new_string(implode(this->avatar.dir, ' ').c_str()));
        json_object_object_add(data, "view_dir", json_object_new_string(implode(this->avatar.view_dir, ' ').c_str()));
        json_object_object_add(data, "up_dir", json_object_new_string(implode(this->avatar.up_dir, ' ').c_str()));
        json_object_object_add(data, "head_pos", json_object_new_string(implode(this->avatar.head_pos, ' ').c_str()));

        json_object_object_add(data, "avatar", json_object_new_string(this->avatar.avatar_file.c_str()));
        json_object_object_add(jobj, "data", data);

        std::string command = json_object_to_json_string(jobj);
        // std::cout << command << std::endl;
        this->janus_servers[this->server]->send(command);
        json_object_put(jobj);
        json_object_put(data);
        return;
    }

    void bot::update_avatar()
    {
        this->avatar.avatar_file = this->avatar.avatar_template;
        metabot::replace(this->avatar.avatar_file, "BOTNAME", this->name);
    }

    void bot::users_online(int results, std::string roomId)
    {
        json_object *jobj = json_object_new_object();
        json_object *data = json_object_new_object();
        std::string method = "users_online";

        json_object_object_add(jobj, "method", json_object_new_string(method.c_str()));
        if(results > 0) json_object_object_add(data, "maxResults", json_object_new_int(results));
        if(roomId.size()) json_object_object_add(data, "roomId", json_object_new_string(roomId.c_str()));
        json_object_object_add(jobj, "data", data);

        this->janus_servers[this->server]->send(json_object_to_json_string(jobj));
        output(json_object_to_json_string(jobj));
        json_object_put(jobj);
        json_object_put(data);
        return;
    }

 
    void bot::logon(std::string room)
    {
        json_object *jobj = json_object_new_object();
        json_object *data = json_object_new_object();
        std::string method = "logon";
        std::string version = PACKAGE_STRING;

        json_object_object_add(jobj, "method", json_object_new_string(method.c_str()));
        json_object_object_add(data, "userId", json_object_new_string(this->name.c_str()));
        if(this->password.size()) json_object_object_add(data, "password", json_object_new_string(this->password.c_str()));
        json_object_object_add(data, "version", json_object_new_string(version.c_str()));
        json_object_object_add(data, "roomId", json_object_new_string(room.c_str()));
        json_object_object_add(jobj, "data", data);

        this->janus_servers[this->server]->send(json_object_to_json_string(jobj));
        json_object_put(jobj);
        json_object_put(data);
        return;
    }

    void bot::chat(std::string toUserId, std::string message)
    {
        json_object *jobj = json_object_new_object();
        json_object *data = json_object_new_object();
        std::string method = "chat";

        json_object_object_add(jobj, "method", json_object_new_string(method.c_str()));
        json_object_object_add(data, "message", json_object_new_string(message.c_str()));

        if(toUserId.size()) json_object_object_add(data, "toUserId", json_object_new_string(toUserId.c_str()));

        json_object_object_add(jobj, "data", data);

        this->janus_servers[this->server]->send(json_object_to_json_string(jobj));

        output(this->name + "> " + message);
        return;
    }
}
