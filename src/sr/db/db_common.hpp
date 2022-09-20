#ifndef DB_COMMON_HPP
#define DB_COMMON_HPP

#include "../link.hpp"
#include "common/standard.hpp"

namespace SR 
{
class DBCommon
{
public:  
    typedef int OrdinalType;
    // Domain ordered by depth-first walk
    // Don't use a vector for this:
    // (a) you'd need the size in advance otherwise the iterators in
    // the xlink_table will go bad while populating and
    // (b) incremental domain update will be hard
    typedef list<XLink> DepthFirstOrderedIndex;    
    typedef DepthFirstOrderedIndex::const_iterator DepthFirstOrderedIt;    
};    
    
}

#endif
