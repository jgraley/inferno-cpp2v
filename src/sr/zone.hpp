#ifndef ZONE_HPP
#define ZONE_HPP

#include "link.hpp"
#include "common/standard.hpp"

namespace SR 
{
// Don't say "XLink" or "Node" or "TreePtrInterface" etc here since
// subject to change!
class Zone
{ 
public:
    Zone( XLink base );
      
    XLink GetBase() const;
    
private:
    XLink base;
};
    
}

#endif
