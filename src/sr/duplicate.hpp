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
                                           XLink source_terminus_xlink = XLink(),
                                           TreePtr<Node> dest_terminus = TreePtr<Node>(),
                                           int *terminus_hit_count = nullptr );
};

}

#endif
