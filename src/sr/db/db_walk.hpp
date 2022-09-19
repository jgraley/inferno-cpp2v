#ifndef DB_WALK_HPP
#define DB_WALK_HPP

#include "../link.hpp"
#include "common/standard.hpp"


namespace SR 
{

class DBWalk
{   
   enum SubtreeMode
    {
        // Behaviour for main domain population: we will check uniqueness
        // of the XLinks we meet during our recursive walk.
        REQUIRE_SOLO,
        
        // Behaviour for domain extensions. We will continue as long as 
        // nodes are not already in the domain. If a node is in the 
        // domain, we don't recurse into it since everything under it
        // will also be in the domain.
        // https://github.com/jgraley/inferno-cpp2v/issues/213#issuecomment-728266001
        STOP_IF_ALREADY_IN
    };

    void AddAtRoot( SubtreeMode mode,  
                    XLink root_xlink );
    void AddLink( SubtreeMode mode, 
                  XLink xlink );
    void AddChildren( SubtreeMode mode, XLink xlink );
    void AddSingularNode( SubtreeMode mode, 
                          const TreePtrInterface *p_x_singular, 
                          XLink xlink );
    void AddSequence( SubtreeMode mode, 
                      SequenceInterface *x_seq, 
                      XLink xlink );
    void AddCollection( SubtreeMode mode, 
                        CollectionInterface *x_col, 
                        XLink xlink );
};    
    
}

#endif
