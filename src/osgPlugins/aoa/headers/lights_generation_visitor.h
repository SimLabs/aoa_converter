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
    void remove_light_nodes();
    aoa_writer&                                    aoa_writer_;
    map<string, map<string, set<osg::Drawable *>>> lights_;
    set<osg::Group*>                               light_nodes_;
};

}