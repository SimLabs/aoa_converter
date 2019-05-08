#pragma once

namespace serialization
{

template<typename T>
T make_default()
{
    return T();
}


} // namespace serialization

#define ENABLE_SERIALIZATION_DEFAULT(type) template<typename T> friend T serialization::make_default();