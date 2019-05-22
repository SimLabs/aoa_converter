#pragma once

#include "json_io.h"
#include <osg/Matrix>

namespace aurora
{
    struct plugin_config
    {
        struct node_ref_settings
        {
            struct add_argument
            {
                enum arg_type
                {
                    FLOAT
                };

                ENUM_DECL_INNER(arg_type)
                    ENUM_DECL_ENTRY(FLOAT)
                ENUM_DECL_END()

                string   channel;
                arg_type type;
                float    value;

                REFL_INNER(add_argument)
                    REFL_ENTRY(channel)
                    REFL_ENTRY(type)
                    REFL_ENTRY(value)
                REFL_END()
            };

            vector<string>       find_rules;
            string               ref_node;
            vector<add_argument> add_arguments;

            REFL_INNER(node_ref_settings)
                REFL_ENTRY(find_rules)
                REFL_ENTRY(ref_node)
                REFL_ENTRY(add_arguments)
            REFL_END()
        };

        osg::Matrix get_full_transform()
        {
            if(!flip_YZ)
                return transform;
            osg::Matrix result = flip_YZ_matrix;
            result.postMult(transform);
            return result;
        }

        // key1 (channel prefix) -> key2 (channel name) -> ref node settings
        // so you can type something like setArgFloat("key1|key2", val) in aurora
        map<string, map<string, node_ref_settings>> lights;
        osg::Matrix transform;
        bool flip_YZ = false;
        string channel_file;

        static const osg::Matrix flip_YZ_matrix;
        static const osg::Matrix reverse_flip_YZ_matrix;

        REFL_INNER(plugin_config)
            REFL_ENTRY(lights)
            {
                auto& lobj_transform_array = *reinterpret_cast<std::array<osg::Matrix::value_type, 16>*>(&lobj.transform);
                auto& robj_transform_array = *reinterpret_cast<std::array<osg::Matrix::value_type, 16>*>(&robj.transform);
                proc(lobj_transform_array, robj_transform_array, "transform");
            }
            REFL_ENTRY(flip_YZ)
            REFL_ENTRY(channel_file);
        REFL_END()
    };

    plugin_config& get_config(optional<string> path = boost::none);
}