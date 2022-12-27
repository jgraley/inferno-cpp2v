#ifndef DUPLICATE_HPP
#define DUPLCIATE_HPP

#include "link.hpp"

namespace SR
{ 
class SCREngine;

class Duplicate
{
public:
    static TreePtr<Node> DuplicateNode( const SCREngine *scr_engine,
                                        TreePtr<Node> pattern,
                                        bool force_dirty );
    static TreePtr<Node> DuplicateSubtree( const SCREngine *scr_engine,
                                           XLink source_xlink,
                                           XLink source_terminus_xlink = XLink(),
                                           TreePtr<Node> dest_terminus = TreePtr<Node>(),
                                           int *terminus_hit_count = nullptr );
};

}

#endif
