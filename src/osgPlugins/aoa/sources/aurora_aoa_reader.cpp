#include "aurora_aoa_reader.h"
#include "aurora_read_processor.h"

namespace aurora
{

dict_t aoa_to_dict(std::string const& path)
{
    std::ifstream file(path, std::ios_base::binary);

    std::string key, value;
    bool reading_key = false;
    bool reading_value = false;

    dict_t result;

    std::stack<dict_t*> dicts({ &result });

    while(true)
    {
        char c = file.get();
        if(c == std::string::traits_type::eof())
            break;

        assert((!reading_key && !reading_value) || reading_key != reading_value);

        switch(c)
        {
        case '#':
            if(!reading_key)
            {
                reading_key = true;
                key.clear();
                key.push_back(c);
            }
            break;
        case '{':
            if(reading_key)
            {
                reading_key = false;
                dicts.push(&dicts.top()->add_child(key.c_str()));
            }
            break;
        case ' ':
        case '\t':
            if(reading_key)
            {
                reading_key = false;
                dicts.push(&dicts.top()->add_child(key.c_str()));
            }
            else if(reading_value)
            {
                value.push_back(c);
            }
            break;
        case '\n':
            if(reading_value)
            {
                reading_value = false;
                dicts.top()->data().assign(value.begin(), value.end());
                dicts.pop();
            }
            else if(reading_key)
            {
                reading_key = false;
                dicts.top()->add_child(key.c_str());
            }
            break;
        case '}':
            dicts.pop();
            break;
        case '\r':
            break;
        default:
            if(!reading_key && !reading_value)
            {
                reading_value = true;
                value.clear();
            }

            if(reading_key)
                key.push_back(c);
            else if(reading_value)
                value.push_back(c);

            break;
        }
    }

    assert(dicts.size() == 1);
    return result;
}

refl::aurora_format read_aoa(std::string const& path)
{
    refl::aurora_format result;
    auto aoa_dict = aoa_to_dict(path);
    read_processor p(aoa_dict);
    reflect(p, result);
    return result;
}

}