#ifndef PARAMETER_MAP_H
#define PARAMETER_MAP_H

/// COMPONENT
#include "parameter.h"

/// SYSTEM
#include <map>
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>

namespace vision
{

class ParameterMap
{
public:
    ParameterMap();

    Parameter& operator[] (const std::string& name);
    const Parameter& at(const std::string& name) const;
    Parameter& at(const std::string& name);

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /*file_version*/) {
        ar & map_;
    }

private:
    std::map<std::string, Parameter> map_;
};

}

#endif // PARAMETER_MAP_H
