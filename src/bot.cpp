#include <iostream>
#include <fstream>
#include <vector>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <map>
#include <metabot.h>
#include <net.h>
#include <bot.h>

namespace metabot
{
    avatar::avatar() {}

    bot::bot(std::string filename)
    {
        std::cout << "Opening " << filename << std::endl;
      try
      {
        this->load(filename);
      } 
      catch (std::string e) 
      {
          std::cout << e << std::endl;
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

        return;
    }
}
