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
            ASSERTF( !!x )("Found NULL pointer in tree at ")( w.Path() );
            w.Advance(); 
        }            
    }
};

#endif
