/// HEADER
#include <utils_vision/config/types.h>

/// SYSTEM
#include <algorithm>
#include <iostream>
#include <string>
#include <typeinfo>

namespace Types {

template <class T>
typename Parameter<T>::SubType Parameter<T>::read(const std::string& type)
{
    return instance().readName(type);
}
template <class T>
std::string Parameter<T>::write(SubType type)
{
    return instance().writeName(type);
}
template <class T>
std::string Parameter<T>::write(int type)
{
    return instance().writeName(type);
}

template <class T>
typename Parameter<T>::SubType Parameter<T>::readName(const std::string& type)
{
    std::string type_upper = type;
    std::transform(type_upper.begin(), type_upper.end(), type_upper.begin(), ::toupper);

    for(int i = T::FIRST; i != T::COUNT; ++i) {
        if(map[static_cast<SubType>(i)] == type_upper) {
            return (SubType) i;
        }
    }

    ERROR(typeid(T).name() << " does not recognize Parameter " << type_upper << ". Check spelling!");
    throw IllegalException();
}

void StrategyImp::initialize(std::map<ID, std::string> &map)
{
    map[NAIVE] = "FAST";
    map[BIN] = "BIN";
    map[BAG] = "BAG";
    map[BOW] = "BOW";
}


/// INSTANCIATION
template class Parameter<StrategyImp>;

}
