#pragma once

namespace aurora
{

struct material_data
{
    string albedo_tex;
    string explicit_material;
};

struct material_loader
{
    material_loader(string filename);
 
    optional<material_data> get_material_data(string mat_name);

private:
    std::map<std::string, material_data> materials_;
};

}