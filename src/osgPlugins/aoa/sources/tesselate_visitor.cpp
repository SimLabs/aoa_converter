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
    void try_add_middle(osg::TemplateArray<T, ARRAYTYPE, DataSize, DataType> *arr, unsigned int ia, unsigned int ib, float alpha)
    {
        if (arr)
            arr->push_back(arr->at(ia) * (1 - alpha) + arr->at(ib) * alpha);
    }

    const float EPS = 1e-3;
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

        typedef unsigned int vertex_id_t;

        void operator()(vertex_id_t ia0, vertex_id_t ib0, vertex_id_t ic0)
        {
            std::deque<std::tuple<vertex_id_t, vertex_id_t, vertex_id_t>> q;
            q.emplace_back(ia0, ib0, ic0);

            while (!q.empty())
            {
                auto[ia, ib, ic] = q.front();
                q.pop_front();

                auto a = vertices->at(ia);
                auto b = vertices->at(ib);
                auto c = vertices->at(ic);
                auto rotate_counter_clockwise = [&a, &b, &c, &ia, &ib, &ic]()
                {
                    std::swap(ia, ib);
                    std::swap(ic, ib);
                    std::swap(a, b);
                    std::swap(c, b);
                };

                while ((a - b).length2() < std::max((a - c).length2(), (b - c).length2()))
                    rotate_counter_clockwise();

                bool found = false;
                for (int rotations = 0; rotations < 2 && !found; ++rotations) {
                    if (over_the_edge(a.x(), b.x()) || over_the_edge(a.y(), b.y()))
                    {
                        float alpha = 1.0f;
                        auto is_better_alpha = [&alpha](float new_alpha)
                        {
                            return EPS < new_alpha && new_alpha < 1 - EPS && abs(new_alpha - 0.5) < abs(alpha - 0.5);
                        };

                        if (const float alpha_x = get_alpha(a.x(), b.x()); is_better_alpha(alpha_x))
                            alpha = alpha_x;
                        if (const float alpha_y = get_alpha(a.y(), b.y()); is_better_alpha(alpha_y))
                            alpha = alpha_y;

                        if (EPS < alpha && alpha < 1 - EPS)
                        {
                            vertex_id_t iab = split_edge(ia, ib, alpha);
                            q.emplace_back(ic, ia, iab);
                            q.emplace_back(ib, ic, iab);
                            found = true;
                        }
                    }

                    rotate_counter_clockwise();
                }

                if (!found) {
                    out_indices->push_back(ia);
                    out_indices->push_back(ib);
                    out_indices->push_back(ic);
                }
            }
        }

    private:
        float triangle_area(osg::Vec3 &a, osg::Vec3 &b, osg::Vec3 &c) const
        {
            return abs(((a - b) ^ (a - c)).length());
        }

        float get_border(const float a, const float b) const
        {
            if (a < b)
                return (static_cast<int>((a + EPS) / size_threshold) + 1) * size_threshold;
            return (static_cast<int>((b + EPS) / size_threshold) + 1) * size_threshold;
        }

        float get_alpha(const float a, const float b) const
        {
            return (get_border(a, b) - a) / (b - a);
        }

        bool over_the_edge(const float a, const float b) const
        {
            const float border = get_border(a, b);
            return a < border - EPS && border + EPS < b || b < border - EPS && border + EPS < a;
        }

        vertex_id_t split_edge(const vertex_id_t ia, const vertex_id_t ib, const float alpha) const
        {
            const vertex_id_t iab = vertices->size();

            try_add_middle(vertices, ia, ib, alpha);
            try_add_middle(normals, ia, ib, alpha); // TODO: slerp
            try_add_middle(colors, ia, ib, alpha);
            try_add_middle(secondary_colors, ia, ib, alpha);
            try_add_middle(fog, ia, ib, alpha);
            for (auto& i : *tex_coords_list)
            {
                auto *tex_coords = try_use_array<osg::Vec2Array>(i);
                try_add_middle(tex_coords, ia, ib, alpha);
            }

            return iab;
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
