#pragma once
#include <osg/NodeVisitor>
#include "plugin_config.h"

namespace aurora
{

struct aoa_writer;

struct lights_generation_visitor: osg::NodeVisitor
{
    lights_generation_visitor(aoa_writer& writer)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , aoa_writer_(writer)
    {}

    void apply(osg::Geode &geode);
    void generate_lights();

private:
    aoa_writer&                        aoa_writer_;
    map<light_type, set<osg::Geode *>> lights_;
};

}