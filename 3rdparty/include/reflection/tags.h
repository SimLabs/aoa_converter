#pragma once

#include "common/font_descr.h"
#include "lat_lon_enum.h"
#include "geometry/primitives/color_fwd.h"

namespace attr
{

enum editor_kind_t
{
    E_VARIANT = 0,
    E_UOM,
    E_NUMERIC,
    E_TIME,
    E_DATE,
    E_DATE_TIME,
    E_LAT_LON,
    E_ENUM,
    E_FLAG_ENUM,
    E_TIME_ENUM,
    E_DATE_ENUM,
    E_STRING_ENUM,
    E_ALTITUDE,

    E_SIZE
};

inline const char* to_string(editor_kind_t kind)
{
    static const std::vector<const char*> names = {
        "variant"
        , "uom"
        , "numeric"
        , "time"
        , "date"
        , "date_time"
        , "lat_lon"
        , "enum"
        , "flag_enum"
        , "plot"
        , "time_enum"
        , "date_enum"
        , "string_enum"
        , "altitude"
    };

    Assert(kind < names.size());
    return names[kind];
}

typedef boost::variant<bool, int, uint32_t, double, float, std::string, geom::colorab, font_descr> attr_variant;

struct altitude_editor_data
{
    using value_type = double;
    enum { editor_kind = E_ALTITUDE };

    altitude_editor_data(bool read_only = false)
        : read_only(read_only)
    {}

    altitude_editor_data(
        optional<double> min,
        optional<double> max,
        optional<double> step,
        bool read_only
    )
        : min(min)
        , max(max)
        , step(step)
        , read_only(read_only)
    {}

    optional<double> min;
    optional<double> max;
    optional<double> step;
    bool read_only;

    REFL_INNER(altitude_editor_data)
        REFL_ENTRY(min)
        REFL_ENTRY(max)
        REFL_ENTRY(step)
        REFL_ENTRY(read_only)
    REFL_END()
};

struct lat_lon_editor_data
{
    typedef double value_type;
    enum { editor_kind = E_LAT_LON };

    lat_lon_editor_data() : kind(ui_cfg::LL_LAT), read_only(false) {}

    lat_lon_editor_data(ui_cfg::lat_lon_kind kind, bool read_only)
        :kind(kind), read_only(read_only)
    {}

    ui_cfg::lat_lon_kind kind;
    bool read_only;

    REFL_INNER(lat_lon_editor_data)
        REFL_ENTRY(kind)
        REFL_ENTRY(read_only)
    REFL_END()
};

struct date_editor_data
{
    typedef uint32_t value_type;
    enum { editor_kind = E_DATE };

    bool read_only = false;

    REFL_INNER(date_editor_data)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct date_time_editor_data
{
    typedef uint32_t value_type;
    enum { editor_kind = E_DATE_TIME };

    bool read_only = false;

    REFL_INNER(date_time_editor_data)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct enum_editor_data
{
    typedef int value_type;
    enum { editor_kind = E_ENUM };

    enum_editor_data() : values(NULL), read_only(false) {}
    enum_editor_data(std::vector< std::pair<string, int /* enum value */> > const& values, bool read_only)
        : values(values)
        , read_only(read_only)
    {}

    std::vector< std::pair<string, int /* enum value */> > values;
    bool read_only;

    REFL_INNER(enum_editor_data)
        REFL_ENTRY(values)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct string_enum_editor_data
{
    typedef string value_type;
    enum { editor_kind = E_STRING_ENUM };

    string_enum_editor_data() : read_only(false) {}
    string_enum_editor_data(std::vector<string> const& values, bool read_only)
        : values(values)
        , read_only(read_only)
    {}

    std::vector<string> values;
    bool read_only;

    REFL_INNER(string_enum_editor_data)
        REFL_ENTRY(values)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct date_enum_editor_data
{
    typedef uint32_t value_type;
    enum { editor_kind = E_DATE_ENUM };

    date_enum_editor_data()
        : values(NULL)
        , read_only(false)
    {}

    date_enum_editor_data(std::vector<value_type> const& values, bool read_only)
        : values(values)
        , read_only(read_only)
    {}

    std::vector<value_type> values;
    bool read_only;

    REFL_INNER(date_enum_editor_data)
        REFL_ENTRY(values)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct flag_enum_editor_data
{
    typedef int value_type;
    enum { editor_kind = E_FLAG_ENUM };

    flag_enum_editor_data() : values(NULL), read_only(false) {}
    flag_enum_editor_data(std::vector<std::string> const& values, bool read_only)
        : values(values)
        , read_only(read_only)
    {}

    std::vector<std::string> values;
    bool read_only;

    REFL_INNER(flag_enum_editor_data)
        REFL_ENTRY(values)
        REFL_ENTRY(read_only)
        REFL_END()
};

struct variant_editor_data
{
    typedef attr_variant value_type;
    enum { editor_kind = E_VARIANT };

    variant_editor_data(bool read_only = false)
        : read_only(read_only)
    {}

    void add_attribute(string const& name, attr_variant const& var)
    {
        attributes.insert(std::make_pair(name, var));
    }

    std::map<string, attr_variant> attributes;
    bool read_only;

    REFL_INNER(variant_editor_data)
        REFL_ENTRY(attributes)
        REFL_ENTRY(read_only)
        REFL_END()
};

// type traits
template<class T>
struct is_simple_type
{
    enum {
        value =
        std::is_same<T, std::string>::value ||
        std::is_same<T, geom::colorab>::value ||
        std::is_same<T, font_descr>::value ||
        boost::is_arithmetic<T>::value
    };
};

template<class T, class A = void>
struct default_editor_for_type;

template<class T>
struct default_editor_for_type<T, typename std::enable_if<is_simple_type<T>::value>::type>
{
    enum { value = E_VARIANT };
};

template<> struct default_editor_for_type < string > { enum { value = E_VARIANT }; };

template<class T, class A = void>
struct is_default_variant_editor
{
    enum { value = false };
};

template<class T>
struct is_default_variant_editor<T, typename std::enable_if<default_editor_for_type<T>::value == E_VARIANT>::type>
{
    enum { value = true };
};

} // attr

// ==========================================================================
// ============================== EDITOR TAGS ===============================
// ==========================================================================

namespace prop_attr
{

struct date_editor_tag
{
    typedef ::attr::date_editor_data data_type;

    date_editor_tag() {}

    data_type const& data() const { return data_; }

private:
    data_type data_;
};

struct date_time_editor_tag
{
    typedef ::attr::date_time_editor_data data_type;

    date_time_editor_tag() {}

    data_type const& data() const { return data_; }

private:
    data_type data_;
};

template<class T>
T opt_def_func() { return T(); }

template<class editor_tag, typename editor_tag::data_type::value_type(*def_func)() = &opt_def_func<typename editor_tag::data_type::value_type>>
struct opt_editor_tag
{
    typedef typename editor_tag::data_type::value_type inner_value_type;

    opt_editor_tag(editor_tag const& e, inner_value_type def = def_func())
        : next(e)
        , def(def)
    {}


    inner_value_type def;
    editor_tag next;
};


struct read_only_tag {};

struct date_enum_editor_tag
{
    using data_type = ::attr::date_enum_editor_data;

    date_enum_editor_tag(vector<uint32_t> const& values = {}, bool read_only = false)
        : values_(values)
        , read_only_(read_only)
    {}

    data_type data() const
    {
        return data_type(values_, read_only_);
    }

private:
    vector<uint32_t> const& values_;
    bool read_only_;
};

template<class T>
struct numeric_editor_tag
{
    typedef ::attr::variant_editor_data data_type;

    numeric_editor_tag(T min, T max, optional<T> step = boost::none, optional<int> decimals = boost::none, bool read_only = false)
        : data_(read_only)
    {
        data_.add_attribute("minimum", min);
        data_.add_attribute("maximum", max);
        if(step)
            data_.add_attribute("singleStep", *step);

        if(decimals)
            data_.add_attribute("decimals", *decimals);
        else if(step)
        {
            int num_digits = (int)std::floor(log10(*step));
            data_.add_attribute("decimals", num_digits > 0 ? 0 : std::abs(num_digits));
        }
    }

    data_type const& data() const { return data_; }

    numeric_editor_tag(bool read_only = false)
        : data_(read_only)
    {}


private:
    data_type data_;
};

struct text_editor_tag
{
    typedef ::attr::variant_editor_data data_type;

    text_editor_tag(uint32_t max_length, bool read_only = false)
        :text_editor_tag(read_only)
    {
        data_.add_attribute("max_length", max_length);
    }

    data_type const& data() const { return data_; }

    text_editor_tag(bool read_only = false)
        : data_(read_only)
    {
        data_.add_attribute("is_textarea", true);
    }

private:
    data_type data_;
};

struct flag_enum_editor_tag
{
    typedef ::attr::flag_enum_editor_data data_type;

    flag_enum_editor_tag(std::initializer_list<string> const& values, bool read_only = false)
        : values_(values)
        , read_only_(read_only)
    {}

    ::attr::flag_enum_editor_data data() const
    {
        return ::attr::flag_enum_editor_data(values_, read_only_);
    }

private:
    std::initializer_list<string> const& values_;
    bool read_only_;
};

struct enum_editor_tag
{
    typedef ::attr::enum_editor_data data_type;

    enum_editor_tag(std::initializer_list<string> const& values, bool read_only = false)
        : read_only_(read_only)
    {
        values_.reserve(values.size());

        int index = 0;
        for(string const& value : values)
        {
            values_.emplace_back(value, index++);
        }
    }

    enum_editor_tag(std::map<string, int /* enum index */> const& values, bool read_only = false)
        : read_only_(read_only)
    {
        values_.reserve(values.size());
        std::copy(values.begin(), values.end(), std::back_inserter(values_));
    }

    ::attr::enum_editor_data data() const
    {
        return ::attr::enum_editor_data(values_, read_only_);
    }

private:
    std::vector< std::pair<string, int /* enum index */> > values_;
    bool read_only_;
};

struct string_enum_editor_tag
{
    typedef ::attr::string_enum_editor_data data_type;

    string_enum_editor_tag(std::initializer_list<string> const& values, bool read_only = false)
        : values_(values)
        , read_only_(read_only)
    {}

    ::attr::string_enum_editor_data data() const
    {
        return ::attr::string_enum_editor_data(values_, read_only_);
    }

private:
    std::initializer_list<string> const& values_;
    bool read_only_;
};

struct altitude_tag
{
    typedef ::attr::altitude_editor_data data_type;

    altitude_tag(optional<double> min, optional<double> max, optional<double> step = boost::none, bool read_only = false)
        : data_(min, max, step, read_only)
    {}

    altitude_tag(bool read_only = false)
        : data_(boost::none, boost::none, boost::none, read_only)
    {}

    data_type const& data() const { return data_; }

private:
    data_type data_;
};

template<size_t k>
struct lat_lon_editor_tag
{
    typedef ::attr::lat_lon_editor_data data_type;

    lat_lon_editor_tag(bool read_only = false)
        : data_((ui_cfg::lat_lon_kind)k, read_only)
    {}

    data_type const& data() const { return data_; }

private:
    data_type data_;
};

} // prop_attr
