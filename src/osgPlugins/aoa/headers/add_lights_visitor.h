#pragma once

#include <osg/NodeVisitor>
#include <osg/LightSource>

namespace aurora
{
    struct detect_light_node_visitor: osg::NodeVisitor
    {
        detect_light_node_visitor()
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS)
        {}

        void apply(osg::Node& node) override
        {
            if(node.getName().find("light") != std::string::npos)
            {
                is_light_ = true;
            } 
            else
            {
                traverse(node);
            }
        }

        bool get_result()
        {
            return is_light_;
        }

        bool is_light_ = false;
    };



    struct add_lights_visitor: osg::NodeVisitor
    {
        add_lights_visitor()
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {}


        void apply(osg::Node& node) override
        {
            traverse(node);
        }

        void apply(osg::Geode& node) override
        {
            // traverse parents to find if we are in the part of the tree that describes light sources
            detect_light_node_visitor detect_light;
            node.accept(detect_light);
            if(detect_light.get_result())
            {
                if(node.getNumDrawables() == 0)
                    return;
                else
                    nodes_to_replace_.emplace(&node, light_group_for_geode(node));
            }
        }

        osg::ref_ptr<osg::Group> light_group_for_geode(osg::Geode& node)
        {
            auto num_drawables = node.getNumDrawables();
            osg::ref_ptr<osg::Group> result = new osg::Group;
            for(unsigned i = 0; i < num_drawables; ++i)
            {
                result->addChild(light_for_drawable(*node.getDrawable(i)));

                // rotate by 180 degrees about z axis
                auto light_source_2 = light_for_drawable(*node.getDrawable(i));
                osg::Matrix mat;
                mat.setRotate(osg::Quat(geom::pi, osg::Vec3(0, 0, 1)));
                light_source_2->getLight()->setDirection(light_source_2->getLight()->getDirection()*mat);
                result->addChild(light_source_2);

                // add omni
                auto light_source_3 = light_for_drawable(*node.getDrawable(i));
                light_source_2->getLight()->setDirection(osg::Vec3());
                result->addChild(light_source_3);
            }
            return result;
        }

        osg::ref_ptr<osg::LightSource> light_for_drawable(osg::Drawable& node)
        {
            auto box = get_rect_from_drawable(dynamic_cast<osg::Geometry&>(node));
            auto box_center = box.center();
            osg::ref_ptr<osg::Light> light = new osg::Light();
            light->setDirection(osg::Vec3(1, 0, 0));
            light->setDiffuse(osg::Vec4(255, 255, 255, 1));
            light->setPosition(osg::Vec4(box_center.x, box_center.y, box_center.z, 1.));
            osg::ref_ptr<osg::LightSource> light_node = new osg::LightSource;
            light_node->setLight(light);
            return light_node;
        }

        geom::rectangle_3 get_rect_from_drawable(osg::Geometry& node)
        {
            auto& vertices = dynamic_cast<osg::Vec3Array&>(*node.getVertexArray());
            auto num_vertices = vertices.getNumElements();

            double min_x=std::numeric_limits<double>::infinity(), max_x=-std::numeric_limits<double>::infinity();
            double min_y=std::numeric_limits<double>::infinity(), max_y=-std::numeric_limits<double>::infinity();
            double min_z=std::numeric_limits<double>::infinity(), max_z=-std::numeric_limits<double>::infinity();

            for(unsigned i = 0; i < num_vertices; ++i)
            {
                double x, y, z;
                x = vertices[i][0];
                y = vertices[i][1];
                z = vertices[i][2];

                min_x = (std::min(min_x, x));
                min_y = (std::min(min_y, y));
                min_z = (std::min(min_z, z));

                max_x = (std::max(max_x, x));
                max_y = (std::max(max_y, y));
                max_z = (std::max(max_z, z));

                double tol = 0.01;

                if(max_x - min_x > tol && 
                   max_y - min_y > tol && 
                   max_z - min_z > tol) 
                   break;
            }
            return geom::rectangle_3(geom::point_3(min_x, min_y, min_z), geom::point_3(max_x, max_y, max_z));
        }

        void add_lights()
        {
            for(auto& p: nodes_to_replace_)
            {
                p.first->getParent(0)->replaceChild(p.first, p.second);
            }
            nodes_to_replace_.clear();
        }

        std::map<osg::ref_ptr<osg::Node>, osg::ref_ptr<osg::Node>> nodes_to_replace_; 
    };


}
