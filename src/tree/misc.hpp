#ifndef MISC_HPP
#define MISC_HPP

#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

TreePtr<CPPTree::Identifier> GetIdentifier( TreePtr<CPPTree::Declaration> d );

class GetDeclaration : public OutOfPlaceTransformation
{
public:
    class TypeDeclarationNotFound : public ::Mismatch
    {
    };
    class InstanceDeclarationNotFound : public ::Mismatch
    {
    };
    virtual TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root );
    static GetDeclaration instance;
private:
	TreePtr<CPPTree::UserType> Get( TreePtr<Node> context, TreePtr<CPPTree::TypeIdentifier> id );
	TreePtr<CPPTree::Instance> Get( TreePtr<Node> context, TreePtr<CPPTree::InstanceIdentifier> id );
};

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
// TODO make this a Transformation
TreePtr<CPPTree::Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<CPPTree::TypeIdentifier> id );

// Hunt through a record and its bases to find the named member
TreePtr<CPPTree::Instance> FindMemberByName( TreePtr<CPPTree::Program> program, TreePtr<CPPTree::Record> r, string name );

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
TreePtr<CPPTree::Identifier> GetIdentifierOfDeclaration( TreePtr<CPPTree::Declaration> d );

#endif

