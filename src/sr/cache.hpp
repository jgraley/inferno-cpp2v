#ifndef CACHE_HPP
#define CACHE_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include <map>
#include <functional>

namespace SR
{ 
class Agent;
class LocatedLink;

/**
 * Cache for local normal links: agents are required to register the same
 * x every time when registering normal links. The cache will ensure this.
 * It is OK to also cache links into the input tree if for example unsure 
 * (TypeOf may or may not, for example).
 */ 
class CacheByLocation
{
public:       
    typedef TreePtr<Node> Value;
    typedef TreePtr<Node> Key;
    typedef function<Value(Key key)> CannonicalOperation;

    /**
     * Invoke a std::function (eg a lambda) with caching. The op will only
     * be run once for each distinct value of location - successive calls
     * will read the cached value. This function guarantees to return
     * the same x for each value of location. op is not stored, so can
     * capture by reference.
     */
    Value operator()( Key key, CannonicalOperation op );
        
private:
    map< Key, Value > cache;
};

};

#endif