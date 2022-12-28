#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "link.hpp"

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

    static TreePtr<Node> DuplicateNode( const DirtyGrassUpdateInterface *dirty_grass,
                                        TreePtr<Node> pattern,
                                        bool force_dirty );
    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink,
                                           XLink source_terminus_xlink,
                                           TreePtr<Node> dest_terminus,
                                           int *terminus_hit_count = nullptr );

    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink );

    // terminii maps source tree terminii to destination terminii. We deep copy the
    // source tree up to the source terminii which are the keys in the map, and then
    // shallow-copy the dest terminii which are the values in the map. It's a non-const
    // ref to the map, and we remove terminii from it when found. Caller should check
    // that the map is empty afterwards.
    static TreePtr<Node> DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink,
                                           map<XLink, TreePtr<Node>> &terminii );
};

}

#endif
