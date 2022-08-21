#ifndef MISC_HPP
#define MISC_HPP

#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

// Really, Declaration should point to an Identifier and that would be that, but at present
// there are differnent kinds of Declaration that point to different kinds of Node.
// TODO refactor this away
TreePtr<CPPTree::Identifier> GetIdentifierOfDeclaration( TreePtr<CPPTree::Declaration> d );

class HasDeclaration : public Transformation
{
public:
    class TypeDeclarationNotFound : public ::Mismatch {};
    class InstanceDeclarationNotFound : public ::Mismatch {};
    
    virtual TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> node );
    virtual TreePtr<Node> operator()( const TreeKit &kit, TreePtr<Node> node );
    static HasDeclaration instance;
};

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
// TODO make this a Transformation
TreePtr<CPPTree::Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<CPPTree::TypeIdentifier> id );

// Hunt through a record and its bases to find the named member
TreePtr<CPPTree::Instance> FindMemberByName( TreePtr<Node> context, TreePtr<CPPTree::Record> r, string name );

// concatenate sequences by adding them, like strings etc
// TODO move to common/
template<typename T>
Sequence<T> operator+( Sequence<T> &s1, Sequence<T> &s2 )
{
    Sequence<T> sr=s1;
    for( TreePtr<T> t : s2 )
        sr.push_back(t);
    return sr;    
}


#endif

