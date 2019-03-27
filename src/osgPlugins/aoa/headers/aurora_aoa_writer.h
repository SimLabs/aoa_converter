#pragma once

namespace aurora
{

struct geometry_visitor;

struct aoa_writer
{
    static const uint32_t c_GP_Version = 300;
    static const uint32_t c_GP_BaseFileMarker = ('D' << 8) | 'O';
    static const uint32_t c_GP_HeaderSize = sizeof(c_GP_Version) + sizeof(c_GP_BaseFileMarker) + 4 * sizeof(uint32_t);

    aoa_writer(string const& name)
        : file_(name, std::ios_base::binary | std::ios_base::out)
        , filename_(name)
    {
        unsigned zeros[4] = {};
        write(&c_GP_BaseFileMarker, sizeof(unsigned));
        write(&c_GP_Version, sizeof(unsigned));
        write(zeros, sizeof(zeros));
    }

    void save_data(geometry_visitor const& v);

private:
    void write(const void* data, size_t size)
    {
        file_.write(reinterpret_cast<const char*>(data), size);
    }

private:
    ofstream file_;
    string   filename_;
};


}