#ifndef INFERNO_PATTERNS_HPP
#define INFERNO_PATTERNS_HPP

#include "sr/search_replace.hpp"
#include "tree/cpptree.hpp" // TODO this dep means we must move this into steps/ or at least parts
#include "helpers/transformation.hpp"


// Make an identifer based on an existing one. New identfier is named using
// sprintf( format, source->name )
// You must key the source identifier to somehting in the search pattern (so it
// will get substitued to the real Specific identifier found in the tree) and
// you may couple the SoftMakeIdentifier in the replace pattern if you need
// to specify it in more than one place. Note that SoftMakeIdentifier is stateless
// and cannot therefore keep track of uniqueness - you'll get a new one each time
// and must rely on a replace coupling to get multiple reference to the same
// new identifier. Rule is: ONE of these per new identifier.

// TODO do this via a transformation as with TransformOf/TransformOf
#define BYPASS_WHEN_IDENTICAL 1
struct BuildIdentifierBase : CompareReplace::SoftReplacePattern
{
    BuildIdentifierBase( string s, int f=0 ) : format(s), flags(f) {}
    Sequence<CPPTree::Identifier> sources;
    string GetNewName( const CompareReplace *sr );
    string format;
    int flags;
};

struct BuildInstanceIdentifier : Special<CPPTree::InstanceIdentifier>,                             
                                 BuildIdentifierBase
{
    SPECIAL_NODE_FUNCTIONS
    BuildInstanceIdentifier( string s, int f=0 ) : BuildIdentifierBase(s,f) {}
    BuildInstanceIdentifier() : BuildIdentifierBase("unnamed") {}
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
    string newname = GetNewName( sr );
    return TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) );
    }                                                   
};

struct BuildTypeIdentifier : Special<CPPTree::TypeIdentifier>,                             
                             BuildIdentifierBase
{
    SPECIAL_NODE_FUNCTIONS
    BuildTypeIdentifier( string s="Unnamed", int f=0 ) : BuildIdentifierBase(s,f) {}
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
    string newname = GetNewName( sr );
    return TreePtr<CPPTree::SpecificTypeIdentifier>( new CPPTree::SpecificTypeIdentifier( newname ) );
    }                                               
};

struct BuildLabelIdentifier : Special<CPPTree::LabelIdentifier>,                             
                              BuildIdentifierBase
{
    SPECIAL_NODE_FUNCTIONS
    BuildLabelIdentifier() : BuildIdentifierBase("UNNAMED") {}
    BuildLabelIdentifier( string s, int f=0 ) : BuildIdentifierBase(s,f) {}
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
    string newname = GetNewName( sr );
    return TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) );
    }                                               
};



// These can be used in search pattern to match a SpecificIdentifier by name.
// (cannot do this using a SpecificIdentifier in the search pattern because
// the address of the node would be compared, not the name string). TODO document
struct IdentifierByNameBase : CompareReplace::SoftSearchPattern
{
    IdentifierByNameBase( string n ) : name(n) {}
    bool IsMatch( const CompareReplace *sr, const TreePtrInterface &x );
    string name;
};

struct InstanceIdentifierByName : Special<CPPTree::InstanceIdentifier>,                             
                                 IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    InstanceIdentifierByName() : IdentifierByNameBase(string()) {}    
    InstanceIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                const TreePtrInterface &x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};

struct TypeIdentifierByName : Special<CPPTree::TypeIdentifier>,                             
                             IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    TypeIdentifierByName() : IdentifierByNameBase(string()) {}    
    TypeIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                const TreePtrInterface &x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};

struct LabelIdentifierByName : Special<CPPTree::LabelIdentifier>,                             
                              IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    LabelIdentifierByName() : IdentifierByNameBase(string()) {}    
    LabelIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                const TreePtrInterface &x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};

#endif
