#ifndef INFERNO_PATTERNS_HPP
#define INFERNO_PATTERNS_HPP

#include "sr/search_replace.hpp"
#include "tree/cpptree.hpp" // TODO this dep means we must move this into steps/ or at least parts
#include "helpers/transformation.hpp"

using namespace SR;
// TODO pollutes client namespace

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
struct BuildIdentifierBase : SoftAgent
{
    BuildIdentifierBase( string s, int f=0 ) : format(s), flags(f) {}
    Sequence<CPPTree::Identifier> sources;
    string GetNewName();
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
    
    virtual TreePtr<Node> MyBuildReplace()
    {
        string newname = GetNewName();
        return TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) );
    }                                                   
};


struct BuildTypeIdentifier : Special<CPPTree::TypeIdentifier>,                             
                             BuildIdentifierBase
{
    SPECIAL_NODE_FUNCTIONS
    BuildTypeIdentifier( string s="Unnamed", int f=0 ) : BuildIdentifierBase(s,f) {}
private:
    virtual TreePtr<Node> MyBuildReplace()
    {
    string newname = GetNewName();
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
    virtual TreePtr<Node> MyBuildReplace()
    {
    string newname = GetNewName();
    return TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) );
    }                                               
};


// These can be used in search pattern to match a SpecificIdentifier by name.
// (cannot do this using a SpecificIdentifier in the search pattern because
// the address of the node would be compared, not the name string). TODO document
struct IdentifierByNameBase : SoftAgent
{
    IdentifierByNameBase( string n ) : name(n) {}
    bool IsMatch( const TreePtrInterface &x ) const;
    string name;
};


struct InstanceIdentifierByName : Special<CPPTree::InstanceIdentifier>,                             
                                  IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    InstanceIdentifierByName() : IdentifierByNameBase(string()) {}    
    InstanceIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool MyCompare( const TreePtrInterface &x ) const
    {
        return IsMatch( x );
    }                                
};


struct TypeIdentifierByName : Special<CPPTree::TypeIdentifier>,                             
                             IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    TypeIdentifierByName() : IdentifierByNameBase(string()) {}    
    TypeIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool MyCompare( const TreePtrInterface &x ) const
    {
        return IsMatch( x );
    }                                
};


struct LabelIdentifierByName : Special<CPPTree::LabelIdentifier>,                             
                              IdentifierByNameBase
{
    SPECIAL_NODE_FUNCTIONS

    LabelIdentifierByName() : IdentifierByNameBase(string()) {}    
    LabelIdentifierByName( string n ) : IdentifierByNameBase(n) {}
private:
    virtual bool MyCompare( const TreePtrInterface &x ) const
    {
        return IsMatch( x );
    }                                
};


// Base class for special nodes that match nested nodes
struct NestedBase : SoftAgent
{
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const = 0;
    virtual bool MyCompare( const TreePtrInterface &x ) const;
    TreePtr<Node> terminus; 
    TreePtr<CPPTree::String> depth;    
};


// Recurse through a number of nested Array nodes, but only by going through
// the "element" member, not the "size" member. So this will get you from the type
// of an instance to the type of the eventual element in a nested array decl.
struct NestedArray : NestedBase, Special<CPPTree::Type>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};


// Recurse through a number of Subscript nodes, but only going through
// the base, not the index. Thus we seek the instance that contains the 
// data we started with. Also go through member field of Lookup nodes.
struct NestedSubscriptLookup : NestedBase, Special<CPPTree::Expression>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};


#endif
