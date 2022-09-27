#include "cache.hpp"
    
#include "node/node.hpp"    
    
using namespace SR;    
    
CacheByLocation::Value CacheByLocation::operator()( Key key, CannonicalOperation op )
{  
    if( lines.count( key ) )
        return lines.at( key );
        
    Value x = op( key ); // note: could throw TODO cache the exception object!
    
    lines[key] = x;
    return x;
}


CacheByLocation::Value CacheByLocation::at( Key key ) const
{
    ASSERT( lines.count(key) > 0 );
    return lines.at(key);
}


CacheByLocation::Value &CacheByLocation::operator[]( Key key )
{
    return lines[key];
}


void CacheByLocation::Reset()
{
    lines.clear();
}
