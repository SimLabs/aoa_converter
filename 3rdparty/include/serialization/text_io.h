#pragma once 

namespace text_io
{
    struct section_t
    {
        section_t()
        {}

        section_t(string const& name, string const& body)
            : name(name)
            , body(body)
        {}

        section_t(section_t&&) noexcept = default;
        section_t &operator=(section_t&&) noexcept = default;

        section_t(section_t const &) = default;
        section_t &operator=(section_t const &) = default;

        string name;
        string body;
    };

    typedef 
        vector<section_t> 
        sections_t;

    std::ostream& write_name(std::ostream& os, section_t const& sec);
    std::ostream& write_body(std::ostream& os, section_t const& sec);
    
    bool operator==(section_t const& lhs, section_t const& rhs);
    std::ostream& operator<<(std::ostream& os, section_t const& sec);
    //

    
    sections_t  extract_sections(fs::path file, vector<size_t>* body_starts = nullptr);
    void        write_sections  (fs::path file, sections_t const& sections, vector<size_t>* body_starts = nullptr);
    section_t&  get_section     (sections_t& sections, string name);


//////////////////////////////////////////////////////////////////////////
// Implementation 
//////////////////////////////////////////////////////////////////////////

    inline std::ostream& write_name(std::ostream& os, section_t const& sec)
    {
        os << "[" << sec.name << "]" << "\n";
        return os;
    }

    inline std::ostream& write_body(std::ostream& os, section_t const& sec)
    {
        os << sec.body << "\n";

        if (!sec.body.empty())
            os << "\n";

        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, section_t const& sec)
    {
        write_name(os, sec);
        write_body(os, sec);

        return os;
    }

    inline bool operator==(section_t const& lhs, section_t const& rhs)
    {
        return 
            lhs.name == rhs.name &&
            lhs.body == rhs.body;
    }
    
    namespace details 
    {
        inline string read_content(fs::path file)
        {
            Assert(is_regular_file(file));
            ifstream ifs(file.string(), std::ios::binary);

            size_t sz = file_size(file);

            string content;
            content.resize(sz);

            ifs.read(const_cast<char*>(content.data()), sz);
            return content;
        }
    }

    inline sections_t extract_sections(fs::path file, vector<size_t>* body_starts)
    {
        string content = details::read_content(file);

        size_t close_bracket = 0;
        size_t open_bracket = content.find_first_of('[', close_bracket);

        sections_t sections;
        set<string> names; // just for verification

        while (open_bracket != string::npos)
        {
            close_bracket = content.find_first_of(']', open_bracket + 1);

            VerifyMsg(close_bracket != string::npos,
                "Unable to find matching closing bracket for opening one at: " << open_bracket);

            section_t sec;

            sec.name = content.substr(open_bracket + 1, close_bracket - (open_bracket + 1));
            boost::trim(sec.name);

            open_bracket = content.find_first_of('[', close_bracket + 1);

            sec.body = content.substr(close_bracket + 1, open_bracket - (close_bracket + 1));
            boost::trim(sec.body);

            VerifyMsg(names.count(sec.name) == 0, "Duplicated section " << sec.name);
            sections.emplace_back(move(sec));

            if (body_starts)
                body_starts->push_back(close_bracket + 2); // including '\n'
        }

        return sections;
    }

    inline void write_sections(fs::path file, sections_t const& sections, vector<size_t>* body_starts)
   {
        ofstream ofs(file.string(), std::ios::binary);

        if (body_starts)
            body_starts->reserve(sections.size());

        for (auto const& sec : sections)
        {
            write_name(ofs, sec);

            if (body_starts)
                body_starts->push_back(ofs.tellp());

            write_body(ofs, sec);
        }
    }

    inline section_t& get_section(sections_t& sections, string name)
    {
        auto it = find_if(sections.begin(), sections.end(), [&name](text_io::section_t const& sec){ return sec.name == name; });
        Verify(it != sections.end());

        return *it;
    }

} // text_io