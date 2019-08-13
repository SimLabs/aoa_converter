#pragma once

template<class T>
struct FbxPropTraits;

template<>
struct FbxPropTraits<double>
{
    static constexpr std::initializer_list<unsigned> FbxTypes = { eFbxDouble , eFbxFloat };
};

template<>
struct FbxPropTraits<float>
{
    static constexpr std::initializer_list<unsigned> FbxTypes = { eFbxDouble , eFbxFloat };
};

template<const unsigned FbxTypeId>
struct FbxTypeByTypeId;

template<>
struct FbxTypeByTypeId<eFbxFloat>
{
    using Type = FbxFloat;
};

template<>
struct FbxTypeByTypeId<eFbxDouble>
{
    using Type = FbxDouble;
};

inline FbxProperty findFbxProperty(FbxObject* o, const char* s)
{
    FbxProperty lProperty = o->GetFirstProperty();
    while(lProperty.IsValid())
    {
        if(lProperty.GetName() == FbxString(s))
        {
            return lProperty;
        }
        lProperty = o->GetNextProperty(lProperty);
    }
    return lProperty;
}

template<class T, unsigned i = 0>
T getFbxPropValue(FbxProperty prop, bool& found)
{
    FbxDataType lPropertyDataType = prop.GetPropertyDataType();
    if constexpr(std::next(FbxPropTraits<T>::FbxTypes.begin(), i) != FbxPropTraits<T>::FbxTypes.end())
    {
        constexpr auto typeId = *std::next(FbxPropTraits<T>::FbxTypes.begin(), i);
        if(unsigned(lPropertyDataType.GetType()) == typeId)
        {
            found = true;
            return prop.Get<FbxTypeByTypeId<typeId>::Type>();
        }
        else
            return getFbxPropValue<T, i + 1>(prop, found);
    }
    else
        return T();
}

template<class T>
T getFbxObjPropValue(FbxObject* o, const char* propName, bool& found)
{
    found = false;
    FbxProperty prop = findFbxProperty(o, propName);
    if(prop.IsValid())
    {
        return getFbxPropValue<T>(prop, found);
    }
    return T();
}