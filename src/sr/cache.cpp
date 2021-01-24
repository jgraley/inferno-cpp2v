#include "cache.hpp"
    
#include "node/node.hpp"    
    
using namespace SR;    
    
CacheByLocation::Value CacheByLocation::operator()( Key key, CannonicalOperation op )
{  
    if( cache.count( key ) )
        return cache.at( key );
        
    Value x = op( key ); // note: could throw TODO cache the exception object!
    
    cache[key] = x;
    return x;
}


void CacheByLocation::Reset()
{
    cache.clear();
}
