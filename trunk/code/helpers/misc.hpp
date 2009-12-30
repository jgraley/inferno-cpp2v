#ifndef MISC_HPP
#define MISC_HPP

#include "tree/tree.hpp"
#include "walk.hpp"
#include "transformation.hpp"
#include "search_replace.hpp"

shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d );

class GetDeclaration : public Transformation, public TransformTo<InstanceIdentifier>
{
public:
	NODE_FUNCTIONS
	GetDeclaration() : TransformTo<InstanceIdentifier>( this ) {}
    virtual shared_ptr<Node> operator()( shared_ptr<Node> context, shared_ptr<Node> root );
private:
	shared_ptr<UserType> Get( shared_ptr<Node> context, shared_ptr<TypeIdentifier> id );
	shared_ptr<Instance> Get( shared_ptr<Node> context, shared_ptr<InstanceIdentifier> id );
};


// Look for a record, skipping over typedefs. Returns NULL if not a record.
shared_ptr<Record> GetRecordDeclaration( shared_ptr<Node> context, shared_ptr<TypeIdentifier> id );

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
