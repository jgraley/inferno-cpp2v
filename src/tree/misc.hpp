#ifndef MISC_HPP
#define MISC_HPP

#include "tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

TreePtr<Identifier> GetIdentifier( TreePtr<Declaration> d );

class GetDeclaration : public OutOfPlaceTransformation
{
public:
    virtual TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root );
    static GetDeclaration instance;
private:
	TreePtr<UserType> Get( TreePtr<Node> context, TreePtr<TypeIdentifier> id );
	TreePtr<Instance> Get( TreePtr<Node> context, TreePtr<InstanceIdentifier> id );
};

// Look for a record, skipping over typedefs. Returns NULL if not a record.
// TODO make this a Transformation
TreePtr<Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<TypeIdentifier> id );

// Hunt through a record and its bases to find the named member
TreePtr<Instance> FindMemberByName( TreePtr<Program> program, TreePtr<Record> r, string name );

// concatenate sequences by adding them, like strings etc
// TODO move to common/
template<typename T>
Sequence<T> operator+( Sequence<T> &s1, Sequence<T> &s2 )
{
    Sequence<T> sr=s1;
    FOREACH( TreePtr<T> t, s2 )
        sr.push_back(t);
    return sr;    
}

// Really, Declaration should point to an Identifier and that would be that, but at present
// there are differnent kinds of Declaration that point to different kinds of Node.
// TODO refactor this away
TreePtr<Identifier> GetIdentifierOfDeclaration( TreePtr<Declaration> d );

#endif
