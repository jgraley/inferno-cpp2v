#ifndef MISC_HPP
#define MISC_HPP

#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

// Really, Declaration should point to an Identifier and that would be that, but at present
// there are differnent kinds of Declaration that point to different kinds of Node.
// TODO refactor this away
AugTreePtr<CPPTree::Identifier> GetIdentifierOfDeclaration( AugTreePtr<CPPTree::Declaration> d );

class BaseDeclarationOf : public Transformation
{
public:
    class DeclarationNotFound : public ::Mismatch {};
    
    AugTreePtr<Node> TryApplyTransformation( const TransKit &kit, 
                                             AugTreePtr<Node> node ) const override;
};


// Version for use with instance ids, returns Instance etc
class DeclarationOf : public BaseDeclarationOf
{
public:
    static DeclarationOf instance;
};


// Version for use with type ids, returns Record, Typedef etc
// At present the only reason we have differently named classes is to differentiate 
// during parsing, so we know if we're working on a type or a normal.
// But real differences could develop, no reason for them not to.
class TypeDeclarationOf : public BaseDeclarationOf
{
public:
    static TypeDeclarationOf instance;
};


// Look for a record, skipping over typedefs. Returns nullptr if not a record.
// TODO make this a Transformation
AugTreePtr<CPPTree::Record> TryGetRecordDeclaration( const TransKit &kit, AugTreePtr<CPPTree::TypeIdentifier> id );

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

