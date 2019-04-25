#include "material_loader.h"

namespace aurora
{
namespace
{

std::istream & split_next_line(std::istream & str, std::vector<std::string>& result)
{
    result.clear();
    std::string                line;
    std::getline(str, line);

    std::stringstream          line_stream(line);
    std::string                cell;

    while(std::getline(line_stream, cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if(!line_stream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return str;
}

}

material_loader::material_loader(string filename)
{
    std::ifstream input(filename);

    std::string line;
    // throw away header
    std::getline(input, line);

    vector<string> fields;
    while(split_next_line(input, fields))
    {
        assert(fields.size() >= 2);
        material_data mat_data;
        mat_data.albedo_tex = fs::path(fields[1]).lexically_normal().string();
        if(fields.size() > 2)
        {
            mat_data.explicit_material = fields[2];
        }

        materials_.emplace(fields[0], mat_data);
    }
}

optional<material_data> material_loader::get_material_data(string mat_name)
{
    auto it = materials_.find(mat_name);
    if(it != materials_.end())
        return it->second;
    else
        return boost::none;
}

}
