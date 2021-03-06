#pragma once

#include <vector>
#include <string>

namespace std
{

static inline std::string to_string(std::string& s)
{
    return s;
}

}

template<typename T>
std::string containerToString(T container)
{
    std::string res;
    
    for(auto& elem : container)
    {
        res += std::to_string(elem);
        res += " ";
    }
    
    if(res.size() != 0)
        res.pop_back();
    
    return res;
}
