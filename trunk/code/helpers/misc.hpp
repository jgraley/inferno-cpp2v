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

// concatenate sequences by adding them, like strings etc
template<typename T>
Sequence<T> operator+( Sequence<T> &s1, Sequence<T> &s2 )
{
    Sequence<T> sr=s1;
    FOREACH( shared_ptr<T> t, s2 )
        sr.push_back(t);
    return sr;    
}

#endif
