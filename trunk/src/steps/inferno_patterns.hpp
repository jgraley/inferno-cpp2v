#ifndef INFERNO_PATTERNS_HPP
#define INFERNO_PATTERNS_HPP

#include "sr/search_replace.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"

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
struct BuildIdentifierAgent : public virtual AgentCommon
{
    BuildIdentifierAgent( string s, int f=0 ) : format(s), flags(f) {}
    virtual void PatternQueryImpl() const {}
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const { return true; }    
    Sequence<CPPTree::Identifier> sources;
    string GetNewName();
    string format;
    int flags;
};


struct BuildInstanceIdentifier : Special<CPPTree::InstanceIdentifier>,                             
                                 BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS
    BuildInstanceIdentifier( string s, int f=0 ) : BuildIdentifierAgent(s,f) {}
    BuildInstanceIdentifier() : BuildIdentifierAgent("unnamed") {}
private:
    
	TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode ) 
	{
		INDENT("%");
		if( !GetCoupled() )
		{
			// Call the soft pattern impl 
			string newname = GetNewName();
			keynode = TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) );
			DoKey( keynode );
		}
		// Note that the keynode could have been set via coupling - but still not
		// likely to do anything sensible, so explicitly check
		return DuplicateSubtree(keynode);   
    }                                                   
};


struct BuildTypeIdentifier : Special<CPPTree::TypeIdentifier>,                             
                             BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS
    BuildTypeIdentifier( string s="Unnamed", int f=0 ) : BuildIdentifierAgent(s,f) {}
private:
	TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode ) 
	{
		INDENT("%");
		if( !GetCoupled() )
		{
			// Call the soft pattern impl 
			string newname = GetNewName();
			keynode = TreePtr<CPPTree::SpecificTypeIdentifier>( new CPPTree::SpecificTypeIdentifier( newname ) );
			DoKey( keynode );
		}
		// Note that the keynode could have been set via coupling - but still not
		// likely to do anything sensible, so explicitly check
		return DuplicateSubtree(keynode);   
    }                                                   
};


struct BuildLabelIdentifier : Special<CPPTree::LabelIdentifier>,                             
                              BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS
    BuildLabelIdentifier() : BuildIdentifierAgent("UNNAMED") {}
    BuildLabelIdentifier( string s, int f=0 ) : BuildIdentifierAgent(s,f) {}
private:
	TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode ) 
	{
		INDENT("%");
		if( !GetCoupled() )
		{
			// Call the soft pattern impl 
			string newname = GetNewName();
			keynode = TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) );
			DoKey( keynode );
		}
		// Note that the keynode could have been set via coupling - but still not
		// likely to do anything sensible, so explicitly check
		return DuplicateSubtree(keynode);   
    }                                                   
};


// These can be used in search pattern to match a SpecificIdentifier by name.
// (cannot do this using a SpecificIdentifier in the search pattern because
// the address of the node would be compared, not the name string). TODO document
struct IdentifierByNameAgent : public virtual AgentCommon
{
    IdentifierByNameAgent( string n ) : name(n) {}
    virtual void PatternQueryImpl() const {}
    bool IsMatch( const TreePtrInterface &x ) const;
    string name;
};


struct InstanceIdentifierByName : Special<CPPTree::InstanceIdentifier>,                             
                                  IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    InstanceIdentifierByName() : IdentifierByNameAgent(string()) {}    
    InstanceIdentifierByName( string n ) : IdentifierByNameAgent(n) {}
private:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const                                  
    {
        return IsMatch( x );
    }                                
};


struct TypeIdentifierByName : Special<CPPTree::TypeIdentifier>,                             
                              IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    TypeIdentifierByName() : IdentifierByNameAgent(string()) {}    
    TypeIdentifierByName( string n ) : IdentifierByNameAgent(n) {}
private:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const                                  
    {
        return IsMatch( x );
    }                                
};


struct LabelIdentifierByName : Special<CPPTree::LabelIdentifier>,                             
                               IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    LabelIdentifierByName() : IdentifierByNameAgent(string()) {}    
    LabelIdentifierByName( string n ) : IdentifierByNameAgent(n) {}
private:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const                                  
    {
        return IsMatch( x );
    }                                
};


// Base class for special nodes that match nested nodes
struct NestedAgent : public virtual AgentCommon
{
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;                                
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const = 0;
    TreePtr<Node> terminus; 
    TreePtr<CPPTree::String> depth;    
};


// Recurse through a number of nested Array nodes, but only by going through
// the "element" member, not the "size" member. So this will get you from the type
// of an instance to the type of the eventual element in a nested array decl.
struct NestedArray : NestedAgent, Special<CPPTree::Type>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};


// Recurse through a number of Subscript nodes, but only going through
// the base, not the index. Thus we seek the instance that contains the 
// data we started with. Also go through member field of Lookup nodes.
struct NestedSubscriptLookup : NestedAgent, Special<CPPTree::Expression>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};


// Something to get the size of the Collection matched by a Star as a SpecificInteger
struct BuildContainerSize : SoftAgent,
                            Special<CPPTree::Integer>
{
    SPECIAL_NODE_FUNCTIONS
    shared_ptr< StarAgent > container;
private:
    virtual void PatternQueryImpl() const {}
    virtual TreePtr<Node> MyBuildReplace();
}; 


struct IsLabelReached : SoftAgent, Special<CPPTree::LabelIdentifier>
{
	SPECIAL_NODE_FUNCTIONS	
	virtual void FlushCache() 
	{
        ASSERT(0);
	    cache.clear();
	}
    virtual void PatternQueryImpl() const {}
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
    virtual bool MyCompare( const TreePtrInterface &xx );
    TreePtr<CPPTree::Expression> pattern;           
           
private:
    bool CanReachExpr( Set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                         TreePtr<CPPTree::LabelIdentifier> x, 
                         TreePtr<CPPTree::Expression> y ); // y is expression. Can it yield label x?
    
    bool CanReachVar( Set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                      TreePtr<CPPTree::LabelIdentifier> x, 
                      TreePtr<CPPTree::InstanceIdentifier> y ); // y is instance identifier. Can expression x be assigned to it?
    
    struct Reaching
    {
        Reaching( TreePtr<CPPTree::LabelIdentifier> f, TreePtr<CPPTree::InstanceIdentifier> t ) : from(f), to(t) {}
        const TreePtr<CPPTree::LabelIdentifier> from;
        const TreePtr<CPPTree::InstanceIdentifier> to;
        bool operator<( const Reaching &other ) const 
        {
            return from==other.from ? to<other.to : from<other.from;
        }
    };
    Map<Reaching, bool> cache; 
};

#endif
