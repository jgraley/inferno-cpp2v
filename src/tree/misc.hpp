#ifndef MISC_HPP
#define MISC_HPP

#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

// Really, Declaration should point to an Identifier and that would be that, but at present
// there are differnent kinds of Declaration that point to different kinds of Node.
// TODO refactor this away
AugTreePtr<CPPTree::Identifier> GetIdentifierOfDeclaration( AugTreePtr<CPPTree::Declaration> d );

class HasDeclaration : public Transformation
{
public:
    class DeclarationNotFound : public ::Mismatch {};
    
    AugTreePtr<Node> ApplyTransformation( const TransKit &kit, 
                                          AugTreePtr<Node> node ) const override;
    static HasDeclaration instance;
};

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
// TODO make this a Transformation
AugTreePtr<CPPTree::Record> GetRecordDeclaration( const TransKit &kit, AugTreePtr<CPPTree::TypeIdentifier> id );

// Hunt through a record and its bases to find the named member
AugTreePtr<CPPTree::Instance> FindMemberByName( const TransKit &kit, AugTreePtr<CPPTree::Record> r, string name );

// concatenate sequences by adding them, like strings etc
// TODO move to common/
template<typename T>
Sequence<T> operator+( Sequence<T> &s1, Sequence<T> &s2 )
{
    Sequence<T> sr=s1;
    for( auto t : s2 )
        sr.push_back(t);
    return sr;    
}


#endif

