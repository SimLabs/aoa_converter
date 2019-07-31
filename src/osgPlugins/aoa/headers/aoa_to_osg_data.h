#pragma once

#include "aurora_format.h"
#include "packed.h"
#include "geometry/half.h"
#include <osg/Array>

namespace aurora
{

template<class Type, unsigned Size>
osg::ref_ptr<osg::Array> create_osg_array(size_t initial_size)
{
    if constexpr(std::is_same_v<Type, float>)
    {
        switch(Size)
        {
            case 1:
                return new osg::FloatArray(initial_size);
            case 2:
                return new osg::Vec2Array(initial_size);
            case 3:
                return new osg::Vec3Array(initial_size);
            case 4:
                return new osg::Vec4Array(initial_size);
            default:
                assert(false);
        }
    }
    else
        assert(false);

    return nullptr;
}


template<class Type, unsigned Size>
struct simple_array_to_osg
{   
    template<class It>
    osg::ref_ptr<osg::Array> operator()(It b, It e, unsigned offset, unsigned stride)
    {
        using raw_data_type = Type (*)[Size];
        auto len = std::distance(b, e) / stride;
        auto result = create_osg_array<Type, Size>(len);
        auto result_data = (raw_data_type)(result->getDataPointer());
        std::advance(b, offset);
        //unsigned i = 0;
        for(unsigned i = 0; i < len; std::advance(b, std::min(stride, unsigned(std::distance(b, e)))), i++)
        {
            const Type* val = reinterpret_cast<const Type*>(&(*b));
            for(unsigned j = 0; j < Size; j++)
            {
                result_data[i][j] = val[j];
            }
        }
        //assert(i == result->getNumElements());
        return result;
    }
};

template<class, unsigned Size>
struct half_array_to_osg
{
    template<class It>
    osg::ref_ptr<osg::Array> operator()(It b, It e, unsigned offset, unsigned stride)
    {
        using raw_data_type = float(*)[Size];
        auto result = create_osg_array<float, Size>(std::distance(b, e) / stride);
        auto result_data = (raw_data_type)(result->getDataPointer());
        std::advance(b, offset);
        unsigned i = 0;
        for(; b < e; std::advance(b, stride), i++)
        {
            const geom::half* val = reinterpret_cast<const geom::half*>(&(*b));
            for(unsigned j = 0; j < Size; j++)
            {
                result_data[i][j] = val[j];
            }
        }
        assert(i == result->getNumElements());
        return result;
    }
};

template<class It>
osg::ref_ptr<osg::Array> uint_10_10_10_2_to_osg(It b, It e, unsigned offset, unsigned stride)
{
    auto len = std::distance(b, e) / stride;
    auto result = new osg::Vec3Array(len);
    std::advance(b, offset);
    for(unsigned i = 0; i < len; std::advance(b, std::min(stride, unsigned(std::distance(b,e)))), i++)
    {
        unsigned val = *reinterpret_cast<const unsigned*>(&(*b));
        Packed::float4 unpacked = Packed::int_to_sf4<10, 10, 10, 2>(val);
        (*result)[i][0] = unpacked.v[0];
        (*result)[i][1] = unpacked.v[1];
        (*result)[i][2] = unpacked.v[2];
    }
    //assert(i == result->getNumElements());
    return result;
}

template<class Type, template<class, unsigned> class F, unsigned Size = 1, unsigned MaxSize = 4, class It>
osg::ref_ptr<osg::Array> select_size(It b, It e, unsigned size, unsigned offset, unsigned stride)
{
    if(size == Size)
        return F<Type, Size>()(b, e, offset, stride);
    else
    {
        if constexpr(Size < MaxSize)
            return select_size<Type, F, Size + 1>(b, e, size, offset, stride);
        else
        {
            assert(false);
            return nullptr;
        }    
    }
}

template<class It>
osg::ref_ptr<osg::Array> attribute_array_to_osg(It b, It e, unsigned type, unsigned size, unsigned offset, unsigned stride)
{
    if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_INT_2_10_10_10_REV ||
                 type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::INT_2_10_10_10_REV)
    {
        return uint_10_10_10_2_to_osg(b, e, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::FLOAT)
    {
        return select_size<float, simple_array_to_osg>(b, e, size, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_INT)
    {   
        return select_size<unsigned, simple_array_to_osg>(b, e, size, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::INT)
    {
        return select_size<int, simple_array_to_osg>(b, e, size, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_BYTE)
    {
        return select_size<unsigned char, simple_array_to_osg>(b, e, size, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::BYTE)
    {
        return select_size<char, simple_array_to_osg>(b, e, size, offset, stride);
    }
    else if(type == refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::HALF_FLOAT)
    {
        return select_size<void, half_array_to_osg>(b, e, size, offset, stride);
    }
    else
    {
        assert(false);
        return nullptr;
    }
}


template<class Type, class It>
osg::ref_ptr<osg::DrawElements> elements_to_osg_impl(It b, It e, unsigned offset, unsigned count, unsigned base_vertex)
{
    osg::ref_ptr<osg::DrawElementsUInt> result;

    std::advance(b, offset * sizeof(Type));

    if constexpr(std::is_same_v<Type, uint16_t>)
    {
        osg::ref_ptr<osg::DrawElementsUShort> tmp = new osg::DrawElementsUShort(GL_TRIANGLES, count, (Type*) &(*b));
        result = new osg::DrawElementsUInt(GL_TRIANGLES, count);
        std::copy(tmp->begin(), tmp->end(), result->begin());
    }
    else if constexpr(std::is_same_v<Type, uint32_t>)
    {
        result = new osg::DrawElementsUInt(GL_TRIANGLES, count, (Type*) &(*b));
    }
    else
    {
        assert(false);
    }

#ifndef NDEBUG
    //if(result->getNumIndices() > 0)
    //{
    //    auto max_index = *std::max_element((Type*) result->getDataPointer(), ((Type*) result->getDataPointer()) + result->getNumIndices());
    //    assert(size_t(base_vertex) + max_index < std::numeric_limits<Type>::max());
    //}
#endif

    if(base_vertex > 0)
        result->offsetIndices(base_vertex);
    return result;
}

template<class It>
osg::ref_ptr<osg::DrawElements> elements_array_to_osg(It b, It e, unsigned max_index, unsigned offset, unsigned count, unsigned base_vertex)
{
    if(max_index <= std::numeric_limits<uint16_t>::max())
    {
        return elements_to_osg_impl<uint16_t>(b, e, offset, count, base_vertex);
    } 
    else
    {
        return elements_to_osg_impl<uint32_t>(b, e, offset, count, base_vertex);
    }
}

}
