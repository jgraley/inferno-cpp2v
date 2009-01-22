#ifndef MISC_HPP
#define MISC_HPP

#include <tree/tree.hpp>

// sort of depracated - there's no particular reason to use this
inline shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d )
{
    if( shared_ptr<Identifier> i = dynamic_pointer_cast< Identifier >(d) )
        return i; 
        
    ASSERT(0 && "Declaration is anonymous");
}

#endif
