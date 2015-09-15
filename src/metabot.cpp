#include <string>
#include <iostream>
#include <vector>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <map>
extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}
#include <thread>
#include <list>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <md5.h>
#include <metabot.h>
#include <net.h>
#include <bot.h>

namespace metabot
{
    /// Array containing bots.
    std::map<std::string, class bot *> bots;

    /** Splits a string by a character.
     *  @param [in] s The string to split.
     *  @param [in] c The character to split the string on.
     *  @return Results of the split as a vector of strings.
     */
    const std::vector<std::string> explode(const std::string& s, const char& c)
    {
            std::string buff{""};
            std::vector<std::string> v;

            for(auto n:s)
            {
                    if(n != c) buff+=n; else
                    if(n == c && buff != "") { v.push_back(buff); buff = ""; }
            }
            if(buff != "") v.push_back(buff);

            return v;
    }

    std::string implode(std::vector<float> data, char c)
    {
        std::string temp;
        for(auto n:data)
        {
            temp += std::to_string(n) + c;
        }
        return temp;
    }

    std::vector<float> stofv(std::string data)
    {
        std::vector<float> coords;

        std::vector<std::string> x = explode(data, ' ');
        for(auto n:x)  coords.push_back(std::stof(n, NULL));
        return coords;
    }

    json_object *get_json_data(std::string command)
    {
        json_object *json_command, *temp;
        json_command = json_tokener_parse(command.c_str());
        if(!json_object_object_get_ex(json_command, "data", &temp)) throw  "No 'data' in command.\n" + command;
        return temp;
    }

    namespace console
    {
        lua_State *L;
        std::string selected_bot;
        bool running = true;

        void select(std::string name)
        {
            selected_bot = name;

            std::cout << "'" << name << "' selected." << std::endl;
        }

        static int say(lua_State *L)
        {   
            std::string message = std::string(lua_tostring(L, -1));
            metabot::bots[console::selected_bot]->chat("", message);
            return 0;
        }

        static int openbot(lua_State *L)
        {
            metabot::bot *Bot;
            std::string filename = std::string(lua_tostring(L, -1));
            try
            {
              Bot = new metabot::bot(filename);
            }
            catch(std::string message)
            {
                std::cout << message << std::endl;
                return 0;
            }

            metabot::bots[Bot->name] = Bot;
            console::select(Bot->name);
            return 0;
        }

        static int logon(lua_State *L)
        {
            std::string room = std::string(lua_tostring(L, -1));
            metabot::bots[console::selected_bot]->logon(md5(room));
        }

        static int listbots(lua_State *L)
        {
            std::cout << "Loaded bots:" << std::endl;
            for(auto & b: metabot::bots) std::cout << b.first << std::endl;
            return 0;
        }

        static int selectbot(lua_State *L)
        {
            std::string name = std::string(lua_tostring(L, -1));
            if(!metabot::bots[name])
            {
                std::cout << "'" << name << "' not loaded." << std::endl;
                return 0;
            }

            console::select(name);
            return 0;
        }

        static int killbot(lua_State *L)
        {
            if(!metabot::bots[selected_bot])
            {
                std::cout << "'" << selected_bot << "' not loaded." << std::endl;
                return 0;
            }

            metabot::bots[selected_bot]->quit = true;

            delete metabot::bots[selected_bot];
            metabot::bots.erase(selected_bot);
            selected_bot = "-";
            return 0;
        }

        static int quit(lua_State *L)
        {
            console::running = false;
            return 0;
        }

        bool start(void)
        {
            std::string command;

            L = lua_open();
            luaL_openlibs(L);

            lua_register(L, "say", say);
            lua_register(L, "open", openbot);
            lua_register(L, "list", listbots); 
            lua_register(L, "select", selectbot);
            lua_register(L, "kill", killbot);
            lua_register(L, "quit", quit);
            lua_register(L, "logon", logon);

            while(console::running)
            {   
                std::cout << "\n[" << selected_bot << "]> ";
                std::getline(std::cin, command);
                if(luaL_dostring(L, command.c_str()))
                {
                    std::cout << "lua error" << std::endl;
                    std::cout << lua_tostring(L, -1) << std::endl;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    metabot::console::start();

    return 0;
}
