#include <string>
#include <iostream>
#include <vector>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <metabot.h>
#include <bot.h>

namespace metabot
{
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
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage:" << std::endl;
        std::cout << argv[0] << " bot_name.bot" << std::endl;
        return 1;
    }

    metabot::bot *Bot = new metabot::bot(argv[1]);
    
    return 0;
}
