#include "tesselate_visitor.h"

#include <osgDB/WriteFile>
#include "osg/TriangleFunctor"
#include "osg/TriangleIndexFunctor"

namespace
{
    template<typename T>
    T *try_use_array(osg::Array *arr)
    {
        if (!arr || arr->getBinding() == osg::Array::BIND_OFF || arr->getBinding() == osg::Array::BIND_OVERALL || arr->getBinding() == osg::Array::BIND_UNDEFINED)
            return nullptr;

        return dynamic_cast<T *>(arr);
    }

    template<typename T, osg::Array::Type ARRAYTYPE, int DataSize, int DataType>
    void try_add_middle(osg::TemplateArray<T, ARRAYTYPE, DataSize, DataType> *arr, unsigned int ia, unsigned int ib)
    {
        if (arr)
            arr->push_back((arr->at(ia) + arr->at(ib)) / 2);
    }
}

namespace aurora
{

void tesselate_visitor::apply(osg::Geometry &geometry)
{
    struct EdgeSplitter
    {
        float size_threshold;
        osg::Vec3Array *vertices;
        osg::Vec3Array *normals;
        osg::Vec4Array *colors;
        osg::Vec4Array *secondary_colors;
        osg::FloatArray *fog;
        osg::Geometry::ArrayList *tex_coords_list;
        osg::DrawElementsUInt *out_indices;

        void operator()(unsigned int ia, unsigned int ib, unsigned int ic)
        {
            const osg::Vec3 &a = vertices->at(ia);
            const osg::Vec3 &b = vertices->at(ib);
            const osg::Vec3 &c = vertices->at(ic);

            if (triangle_needs_split(a, b, c))
            {
                if ((a - b).length2() > std::max((b - c).length2(), (a - c).length2()))
                {
                    unsigned int iab = split_edge(ia, ib);
                    operator()(ia, iab, ic);
                    operator()(ib, ic, iab);
                }
                else if ((b - c).length2() > (a - c).length2())
                {
                    unsigned int ibc = split_edge(ib, ic);
                    operator()(ia, ib, ibc);
                    operator()(ibc, ic, ia);
                }
                else
                {
                    unsigned int iac = split_edge(ia, ic);
                    operator()(ia, ib, iac);
                    operator()(ib, ic, iac);
                }
            }
            else
            {
                out_indices->push_back(ia);
                out_indices->push_back(ib);
                out_indices->push_back(ic);
            }
        }

    private:
        unsigned int split_edge(unsigned int ia, unsigned int ib)
        {
            unsigned int iab = vertices->size();

            try_add_middle(vertices, ia, ib);
            try_add_middle(normals, ia, ib); // TODO: slerp
            try_add_middle(colors, ia, ib);
            try_add_middle(secondary_colors, ia, ib);
            try_add_middle(fog, ia, ib);
            for (int i = 0; i < tex_coords_list->size(); ++i)
            {
                osg::Vec2Array *tex_coords = try_use_array<osg::Vec2Array>(tex_coords_list->at(i));
                try_add_middle(tex_coords, ia, ib);
            }

            return iab;
        }

        bool triangle_needs_split(const osg::Vec3 &a, const osg::Vec3 &b, const osg::Vec3 &c)
        {
            return segment_needs_split(a, b) || segment_needs_split(b, c) || segment_needs_split(c, a);
        }

        bool segment_needs_split(const osg::Vec3 &a, const osg::Vec3 &b)
        {
            return !(in_adjacent_cells(a.x(), b.x()) && in_adjacent_cells(a.y(), b.y())); //  && in_adjacent_cells(a.z(), b.z())
        }

        bool in_adjacent_cells(const float a, const float b)
        {
            return abs(static_cast<int>(a / size_threshold) - static_cast<int>(b / size_threshold)) <= 1;
        }
    };

    osg::TriangleIndexFunctor<EdgeSplitter> splitter;
    splitter.size_threshold = size_threshold;
    splitter.vertices = try_use_array<osg::Vec3Array>(geometry.getVertexArray());
    splitter.normals = try_use_array<osg::Vec3Array>(geometry.getNormalArray());
    splitter.colors = try_use_array<osg::Vec4Array>(geometry.getColorArray());
    splitter.secondary_colors = try_use_array<osg::Vec4Array>(geometry.getSecondaryColorArray());
    splitter.fog = try_use_array<osg::FloatArray>(geometry.getFogCoordArray());
    splitter.tex_coords_list = &geometry.getTexCoordArrayList();
    // TODO: VertexAttribArray?
    splitter.out_indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    geometry.accept(splitter);
    geometry.removePrimitiveSet(0, geometry.getNumPrimitiveSets());
    geometry.addPrimitiveSet(splitter.out_indices);
}

}
