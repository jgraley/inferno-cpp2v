#ifndef MISC_HPP
#define MISC_HPP

#include <tree/tree.hpp>

// A possible weakness in the present tree is that the route from a Declaration
// to its identifier varies depending on whether an object or type is being
// declared 
inline shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d )
{
    if( shared_ptr<ObjectDeclaration> od = dynamic_pointer_cast< ObjectDeclaration >(d) )
        return od->object; // ObjectDeclaration points to the object (old-school object declaration)
    else if( shared_ptr<UserType> ut = dynamic_pointer_cast< UserType >(d) )
        return ut; // UserType declares itself
    else if( shared_ptr<Object> o = dynamic_pointer_cast< Object >(d) )
        return o; // Object declares itself (new-school object declaration)
    ASSERT(0 && "Unknown kind of Declaration");
}

#endif
