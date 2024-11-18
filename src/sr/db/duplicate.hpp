#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "../link.hpp"
#include "free_terminus.hpp"

namespace SR
{ 
class SCREngine;

class Duplicate
{
public:
    struct TerminusInfo
    {
        // Actions on finding terminus:
        // 1. FreeTerminus is overwritten with dest, which can be NULL.
        // 2. dest is set to NULL so you can check we got them all.
        // 3. If dest was NULL (=placeholder value), updater is created on it.
        TreePtr<Node> dest;
        shared_ptr<FreeTerminus> updater;
    };
    
    typedef map<XLink, TerminusInfo> TerminiiMap;

    static TreePtr<Node> DuplicateNode( TreePtr<Node> pattern );

    static TreePtr<Node> DuplicateSubtree( XLink source_base_xlink );

    static TreePtr<Node> DuplicateSubtree( TreePtr<Node> source_base );

    // terminii maps source tree terminii to destination terminii. We deep copy the
    // source tree up to the source terminii which are the keys in the map. See TerminusInfo
    // for what happens next.
    static TreePtr<Node> DuplicateSubtree( XLink source_xlink,
                                           TerminiiMap &terminii_map );
                                           
    static TreePtr<Node> DuplicateSubtreeWorker( TreePtr<Node> source,
                                                 TerminiiMap &terminii_map );
};

}

#endif
