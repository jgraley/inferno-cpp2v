#ifndef MISC_HPP
#define MISC_HPP

#include "tree/tree.hpp"
#include "walk.hpp"

shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d );

shared_ptr<UserType> GetDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id );

// Look for a record, skipping over typedefs. Returns NULL if not a record.
shared_ptr<Record> GetRecordDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id );

shared_ptr<Instance> GetDeclaration( shared_ptr<Program> program, shared_ptr<SpecificInstanceIdentifier> id ); // TODO InstanceIdentifier not Specific

// Hunt through a record and its bases to find the named member
shared_ptr<Instance> FindMemberByName( shared_ptr<Program> program, shared_ptr<Record> r, string name );

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
