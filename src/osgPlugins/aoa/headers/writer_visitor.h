/*
* Copyright (C) 2016 SimLabs LLC - All rights reserved.
* Unauthorized copying of this file or its part is strictly prohibited.
* For any information on this file or its part please contact: support@sim-labs.com
*/

 #pragma once

#include <osg/Node>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include "resource_manager.h"
#include "osgSim/LightPointNode"
#include "osgSim/MultiSwitch"
#include "post_optimization/markings.h"

namespace scg
{

class writer_visitor: public osg::NodeVisitor {

public:
    writer_visitor(std::ostream& fout, resource_manager_ptr resource_manager)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)// TODO: for switch TRAVERSE_ALL_CHILDREN
        , fout_(fout)
        , resource_manager_(resource_manager)
        , passive_switch_(0)
    {
    }

    void apply(osg::Geode &node)     override;
    void apply(osg::Group &node)     override;
    void apply(osg::Transform &node) override;
    void apply(osg::LOD &node)       override;

    void apply(osg::Node &node) override;
    // Note: not derived from NodeVisitor
    void apply(osgSim::LightPointNode &node);
    void apply(osgSim::MultiSwitch    &node);
    void apply(markings_group         &markings);

private:
    std::string get_unique_name(const std::string& defaultvalue);

    std::ostream&                        fout_;
    std::list<std::string>               name_stack_;
    std::map<std::string, unsigned int>  name_map_;

    resource_manager_ptr resource_manager_;

    unsigned passive_switch_;
};

}
