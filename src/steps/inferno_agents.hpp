#ifndef INFERNO_AGENTS_HPP
#define INFERNO_AGENTS_HPP

#include "sr/search_replace.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"

using namespace SR;
// TODO pollutes client namespace
#define BYPASS_WHEN_IDENTICAL 1

/// Agents specific to the inferno C++ and SystemC trees.
class InfernoAgent : public virtual AgentCommon
{
};

//---------------------------------- BuildIdentifier ------------------------------------    

/// Make an identifer based on an existing set, `sources` and a printf
/// format string, `format`. The new identfier is named using
/// `sprintf( format, sources[0].name, source[1].name, etc )`
/// You must key the source identifier to somehting in the search pattern (so it
/// will get substitued to the real Specific identifier found in the tree) and
/// you may couple the `BuildIdentifierAgent` in the replace pattern if you need
/// to specify it in more than one place. Note that `BuildIdentifierAgent` is stateless
/// and cannot therefore keep track of uniqueness - you'll get a new one each time
/// and must rely on a replace coupling to get multiple reference to the same
/// new identifier. Rule is: ONE of these per new identifier. 
/// `flags` permits special behaviour when nonzero, as follows.
/// `BYPASS_WHEN_IDENTICAL` means if all the names of the source nodes are the
/// same, that name is used. This reduces verbosity and is a good fit when
/// in some sense you are "merging" objects with identifiers.
struct BuildIdentifierAgent : public virtual InfernoAgent
{
	// TODO do this via a transformation as with TransformOf/TransformOf
    BuildIdentifierAgent( string s, int f=0 ) : format(s), flags(f) {}
    virtual PatternQueryResult PatternQuery() const 
    { 
        return PatternQueryResult(); 
    }
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const 
    { 
        return DecidedQueryResult(); 
    }   
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
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

//---------------------------------- IdentifierByName ------------------------------------    

/// These can be used in search pattern to match a SpecificIdentifier by name.
/// The identifier must have a name that matches the string in `name`. One
/// cannot do this using a SpecificIdentifier in the search pattern because
/// the address of the node would be compared, not the name string.
/// Wildcarding is not supported and use of this node should be kept to
/// a minimum due to the risk of co-incidentla unwanted matches and the 
/// general principle that identifier names should not be important (it is
/// the identiy proprty itself that matters with identifiers). 
struct IdentifierByNameAgent : public virtual InfernoAgent
{
    IdentifierByNameAgent( string n ) : name(n) {}
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    virtual PatternQueryResult PatternQuery() const 
    { 
        return PatternQueryResult(); 
    }
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
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px, 
                                             const AgentQuery::Choices &choices ) const                                  
    {
        DecidedQueryResult r;
        r.AddLocalMatch( IsMatch( *px ) );
        return r;
    }                                
};


struct TypeIdentifierByName : Special<CPPTree::TypeIdentifier>,                             
                              IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    TypeIdentifierByName() : IdentifierByNameAgent(string()) {}    
    TypeIdentifierByName( string n ) : IdentifierByNameAgent(n) {}
private:
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px, 
                                             const AgentQuery::Choices &choices ) const                                  
    {
        DecidedQueryResult r;
        r.AddLocalMatch( IsMatch( *px ) );
        return r;
    }                                
};


struct LabelIdentifierByName : Special<CPPTree::LabelIdentifier>,                             
                               IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    LabelIdentifierByName() : IdentifierByNameAgent(string()) {}    
    LabelIdentifierByName( string n ) : IdentifierByNameAgent(n) {}
private:
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px, 
                                             const AgentQuery::Choices &choices ) const                                  
    {
        DecidedQueryResult r;
        r.AddLocalMatch( IsMatch( *px ) );
        return r;
    }                                
};

//---------------------------------- Nested ------------------------------------    

/// Matching for the nested nature of array and struct nodes, both when declaring and 
/// when accessing arrays. The `terminus` is the node to be found at the end of
/// the recursion and `depth` is a string matching the steps taken to 
/// reach the terminus.
struct NestedAgent : public virtual InfernoAgent
{
    virtual PatternQueryResult PatternQuery() const;
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const {} // TODO give own appearance
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const = 0;
    TreePtr<Node> terminus; 
    TreePtr<CPPTree::String> depth;    
};


/// Recurse through a number of nested `Array` nodes, but only by going through
/// the "element" member, not the "size" member. So this will get you from the type
/// of an instance to the type of the eventual element in a nested array decl.
/// `depth` is not checked.
struct NestedArray : NestedAgent, Special<CPPTree::Type>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};


/// Recurse through a number of `Subscript` nodes, but only going through
/// the base, not the index. Thus we seek the instance that contains the 
/// data we started with. Also go through member field of `Lookup` nodes.
/// The `depth` pointer must match a `String` node corresponding to the
/// nodes that were seen during traversal, where the letter `S` corresponds 
/// to a `Substript` and `L` corresponds to a `Lookup`.
struct NestedSubscriptLookup : NestedAgent, Special<CPPTree::Expression>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n, string *depth ) const;
};

//---------------------------------- BuildContainerSize ------------------------------------    

/// `BuildContainerSize` is used in replace context to create an integer-valued
/// constant that is the size of a `Star` node pointed to by `container`. The
/// container should couple the star node.
struct BuildContainerSize : public virtual InfernoAgent,
                            Special<CPPTree::Integer>
{
    SPECIAL_NODE_FUNCTIONS
    shared_ptr< StarAgent > container;
private:
    virtual PatternQueryResult PatternQuery() const 
    { 
        return PatternQueryResult(); 
    }
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const 
    { 
        return DecidedQueryResult(); 
    }   
	TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode );
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const {} // TODO give own appearance
}; 

//---------------------------------- IsLabelReached ------------------------------------    

/// `IsLabelReached` matches a `LabelIdentifier` if that label is used
/// anywhere in the expression pointed to by `pattern`.
/// TODO generalise to more than just labels.
struct IsLabelReached : public virtual InfernoAgent, 
                        Special<CPPTree::LabelIdentifier>
{
	SPECIAL_NODE_FUNCTIONS	
	virtual void FlushCache() const 
	{
        ASSERT(0);
	    cache.clear();
	}
    virtual PatternQueryResult PatternQuery() const 
    { 
        return PatternQueryResult(); 
    }
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const {} // TODO give own appearance
    TreePtr<CPPTree::Expression> pattern;           
           
private:
    bool CanReachExpr( Set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                         TreePtr<CPPTree::LabelIdentifier> x, 
                         TreePtr<CPPTree::Expression> y ) const; // y is expression. Can it yield label x?
    
    bool CanReachVar( Set< TreePtr<CPPTree::InstanceIdentifier> > *f,
                      TreePtr<CPPTree::LabelIdentifier> x, 
                      TreePtr<CPPTree::InstanceIdentifier> y ) const; // y is instance identifier. Can expression x be assigned to it?
    
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
    mutable Map<Reaching, bool> cache; // it's a cache, so sue me
};

#endif
