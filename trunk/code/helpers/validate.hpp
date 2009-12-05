#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include "walk.hpp"

class Validate
{
public:
    void operator()( shared_ptr<Node> root )
    {
        Walk w( root );
        while(!w.Done())
        {
            shared_ptr<Node> x = w.Get();
            
            // NULL pointers not allowed in program tree (though they are allowed in search/replace patterns)
            ASSERT( x )("Found NULL pointer in tree at ")( w );
            w.AdvanceInto(); 
        }            
    }
};

// TODO
// - NODE_FUNCTIONS must be present in all nodes
// - no local nodes
// - Identifiers to have exactly one decl ref and many usage refs


#endif
