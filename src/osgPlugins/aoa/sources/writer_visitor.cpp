///*
//* Copyright (C) 2016 SimLabs LLC - All rights reserved.
//* Unauthorized copying of this file or its part is strictly prohibited.
//* For any information on this file or its part please contact: support@sim-labs.com
//*/
//
//#include "writer_visitor.h"
//
//#include "osg/io_utils"
//#include "osg/MatrixTransform"
//#include "osg/LOD"
//#include "osgSim/DOFTransform"
//#include "osgSim/MultiSwitch"
//
//#include "common_includes.h"
//#include "utils.h"
//
//using namespace scg;
//
//
//void writer_visitor::apply(osg::Geode &node)
//{
//    if (passive_switch_)
//        return;
//
//    resource_manager_->process_geode(node);
//    resource_manager_->process_geode(node, getNodePath());
//
//    name_stack_.push_back(node.getName());
//    
//    unsigned level = name_stack_.size() - 1;
//    fout_ << std::string(level, ' ');
//
//    fout_ << "Mesh /Name = \"" << node.getName() << "\" " 
//          << "/ChunkRes= \"" << resource_manager_->get_chunk_resource() <<"\" /Chunks= (";
//
//    vector<size_t> chunks = resource_manager_->chunks_indicies(node, getNodePath());
//    for (int ch = 0; ch < chunks.size(); ch++)
//        fout_ << chunks[ch] << (ch < (chunks.size() - 1) ? ", " : "");
//
//    fout_ << ")\n";
//
//    name_stack_.pop_back();
//}
//
//void writer_visitor::apply(osg::Group &node)
//{
//    if (auto *as_markings = dynamic_cast<markings_group*>(&node))
//    {
//        std::cout << "HERE2" << std::endl;
//        apply(*as_markings);  
//        return;
//    }
//
//    bool is_root = name_stack_.empty();
//
//    std::string  name = node.getName().empty() ? node.className() : node.getName();
//    name_stack_.push_back(name);
//
//    unsigned level = name_stack_.size() - 1;
//
//    string unique_name = get_unique_name(node.getName());
//
//    string node_type = is_root ? "Root" : "Group";
//    fout_ << std::string(level, ' ');
//    fout_ << node_type << " /Name= \"" << unique_name << "\"";
//    if (is_root)
//        fout_ << "/AnimRes=\"" << resource_manager_->get_anim_resource() << "\"";
//    fout_ << "\n";
//
//    fout_ << std::string(level, ' ') << "{\n";
//
//    if (auto light_idx = resource_manager_->extract_light_group(node))
//    {
//        fout_ << std::string(level + 1, ' ');
//        fout_ << "LightGroup";
//        fout_ <<  " /Name= \"" << unique_name << "\"";
//        fout_ << " /LightsRes= \"" << resource_manager_->get_light_resource() << "\"";
//        fout_ << " /GroupIndex= " << *light_idx << "\n";
//    }
//
//    if (auto *as_multi_switch = dynamic_cast<osgSim::MultiSwitch*>(&node))
//    {
//        apply(*as_multi_switch);
//    }
//    else
//    {
//        traverse(node);
//    }
//
//    fout_ << std::string(level, ' ')<< "}\n";
//
//    name_stack_.pop_back();
//}
//
//bool is_root(osg::Node const& node)
//{
//    return node.getName() == "root";
//}
//
//bool is_lightmast(osg::Node const& node)
//{
//    return node.getName() == "mast"; // TODO
//}
//
//void fill_transform(geom::transform_4f &curTransform, geom::point_3f &translationCur, geom::point_3f &scaleCur, const geom::matrix_4f &tmatrix)
//{
//    geom::matrix_4f tmatrix2;
//
//    if (s_cfg().params.addGlobalTransform)
//    {
//        tmatrix2 = (matrices::xyz_to_xzy_inverse() * tmatrix * matrices::xyz_to_xzy()).transpose();
//    }
//    else
//    {
//        tmatrix2 = tmatrix;
//        tmatrix2.transpose();
//    }
//
//    curTransform = geom::transform_4f(tmatrix2, geom::ss_autodetect);
//    translationCur.x = curTransform.translation().x;
//    translationCur.y = curTransform.translation().y;
//    translationCur.z = curTransform.translation().z;
//    scaleCur = curTransform.scale();
//}
//
//void fill_transform(geom::cprf &eulerAnglesCur, geom::point_3f &translationCur, geom::point_3f &scaleCur, const geom::matrix_4f &tranform)
//{
//    geom::transform_4f curTransform;
//    fill_transform(curTransform, translationCur, scaleCur, tranform);
//    eulerAnglesCur.course = curTransform.rotation().cpr().course;
//    eulerAnglesCur.pitch = curTransform.rotation().cpr().pitch;
//    eulerAnglesCur.roll = curTransform.rotation().cpr().roll;
//}
//
//void writer_visitor::apply(osg::Transform &node)
//{
//    name_stack_.push_back(node.getName());
//
//    unsigned level = name_stack_.size() - 1;
//
//
//    transform_4f tr;
//
//    auto dof = dynamic_cast<osgSim::DOFTransform*>(&node);
//    auto mt  = node.asMatrixTransform();
//
//    Verify(mt || dof);
//    if (dof)
//    {
//        tr = make_transform(dof->getPutMatrix());
//        tr *= make_transform(dof->getCurrentHPR(), dof->getCurrentTranslate(), dof->getCurrentScale());
//        tr *= make_transform(dof->getInversePutMatrix());
//    }
//    if (mt)
//    {
//        tr = make_transform(mt->getMatrix());
//    }
//
//    fout_ << std::string(level, ' ');
//    fout_ << "Transform";
//    fout_ << " /Name= \"" << get_unique_name(node.getName()) 
//        << "\" /translate = (" << tr.translation().x         << ", " << tr.translation().y        << ", " << tr.translation().z << ") "
//        << " /rotate = ("      << tr.rotation().cpr().course << ", " << tr.rotation().cpr().pitch << ", " << tr.rotation().cpr().roll << ") "
//        << " /scale = ("       << tr.scale().x               << ", " << tr.scale().y              << ", " << tr.scale().z << ") "
//        << "\n";
//    fout_ << std::string(level, ' ') << "{\n";
//    osg::NodeVisitor::traverse(node);
//    fout_ << std::string(level, ' ') << "}\n";
//
//    name_stack_.pop_back();
//}
//
//void writer_visitor::apply(osg::LOD &node)
//{
//    float min_range = std::numeric_limits<float>::max();
//    unsigned detailed_child = 0;
//    for (unsigned i = 0; i < node.getNumChildren(); ++i)
//    {
//        float mr = node.getMinRange(i);
//        if (mr < min_range)
//        {
//            detailed_child = i;
//            min_range = mr;
//        }
//    }
//
//    if (detailed_child >= node.getNumChildren())
//        return;
//
//    node.getChild(detailed_child)->accept(*this);
//}
//
//void writer_visitor::apply(osg::Node &node)
//{
//    auto *as_light_point = dynamic_cast<osgSim::LightPointNode*>(&node);
//    if (as_light_point)
//    {
//        apply(*as_light_point);
//        return;
//    }
//
//    NodeVisitor::apply(node);
//}
//
//void writer_visitor::apply(osgSim::LightPointNode &/*node*/)
//{
//    return;
////    size_t idx = resource_manager_->extract_light_group(node);
////    unsigned level = name_stack_.size();
////
////    fout_ << string(level, ' ');
////    fout_ << "LightGroup";
////    fout_ <<  " /Name= \"" << get_unique_name(node.getName()) << "\"";
////    fout_ << " /LightsRes= \"" << resource_manager_->get_light_resource() << "\"";
////    fout_ << " /GroupIndex= " << idx << "\n";
//}
//
//void writer_visitor::apply(osgSim::MultiSwitch &node)
//{
//    if (node.getActiveSwitchSet() < node.getSwitchSetList().size())
//    {
//        for(unsigned int pos=0; pos < node.getNumChildren();++pos)
//        {
//            if (!node.getSwitchSetList()[node.getActiveSwitchSet()][pos])
//            {
//                ++passive_switch_;
//                node.getChild(pos)->accept(*this);
//                --passive_switch_;
//            }
//            else
//            {
//                node.getChild(pos)->accept(*this);
//            }
//        }
//    }
//}
//
//void writer_visitor::apply(markings_group &markings)
//{
//    name_stack_.push_back(markings.getName());
//
//    unsigned level = name_stack_.size() - 1;
//    fout_ << std::string(level, ' ');
//    fout_ << "MarkingGroup /Name= \"" << get_unique_name(markings.getName()) << "\"\n" ;
//
//    fout_ << std::string(level, ' ') << "{\n";
//
//    traverse(markings);
//
//    fout_ << std::string(level, ' ') << "}\n";
//}
//
//std::string writer_visitor::get_unique_name(const std::string& default_value)
//{
//    std::string name = default_value;
//
//    if (name_map_.count(name) != 0)
//        name += "_" + to_string(name_map_[name]);    
//    
//    ++name_map_[default_value];
//    return name;
//}
