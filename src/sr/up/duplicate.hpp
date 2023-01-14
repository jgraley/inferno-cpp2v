#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "../link.hpp"
#include "updater.hpp"

namespace SR
{ 
class SCREngine;

class Duplicate
{
public:
    class DirtyGrassUpdateInterface
    {
    public:
        virtual bool IsDirtyGrass( TreePtr<Node> node ) const = 0;
        virtual void AddDirtyGrass( TreePtr<Node> node ) const = 0;
    };
    
    struct TerminusInfo
    {
        // Actions on finding terminus:
        // 1. Terminus is overwritten with dest, which can be NULL.
        // 2. dest is set to NULL so you can check we got them all.
        // 3. If dest was NULL (=placeholder value), updater is created on it.
        TreePtr<Node> dest;
        shared_ptr<Updater> updater;
    };
    
    typedef map<XLink, TerminusInfo> TerminiiMap;

    static TreePtr<Node> DuplicateNode( const DirtyGrassUpdateInterface *dirty_grass,
                                        TreePtr<Node> pattern,
                                        bool force_dirty );

    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_base_xlink );

    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           TreePtr<Node> source_base );

    // terminii maps source tree terminii to destination terminii. We deep copy the
    // source tree up to the source terminii which are the keys in the map. See TerminusInfo
    // for what happens next.
    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink,
                                           TerminiiMap &terminii_map );
                                           
    static TreePtr<Node> DuplicateSubtreeWorker( const DirtyGrassUpdateInterface *dirty_grass,
                                                 TreePtr<Node> source,
                                                 TerminiiMap &terminii_map );
};

}

#endif
