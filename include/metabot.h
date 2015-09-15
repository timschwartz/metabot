#ifndef METABOT_H
#define METABOT_H
namespace metabot
{
    class bot;

    const std::vector<std::string> explode(const std::string& s, const char& c);
    std::string implode(std::vector<float> data, char c);
    std::vector<float> stofv(std::string data);
    json_object *get_json_data(std::string command);
    typedef void (*ScriptFunction)(bot *b, std::string); // function pointer type
    typedef std::map<std::string, ScriptFunction> script_map;
}
#endif
