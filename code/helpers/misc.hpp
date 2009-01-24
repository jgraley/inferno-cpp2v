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

shared_ptr<Instance> FindMemberByName( shared_ptr<Record> r, string name )
{
    TRACE("Record \"%s\" has %d members\n", r->name.c_str(), r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    FOREACH( shared_ptr<Declaration> d, r->members )
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(d) )
            if( i->name == name )
                return i;
                
    // Try recursing through the base classes, if there are any
    if( shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>( r ) )
        FOREACH( shared_ptr<Base> b, ir->bases )
            if( shared_ptr<Instance> i = FindMemberByName( b->record, name ) )
                return i;
                
    // We failed. Hang our head in shame.                
    return shared_ptr<Instance>();
}                

#endif
