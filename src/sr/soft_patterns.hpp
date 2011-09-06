#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "tree/cpptree.hpp" // TODO this dep means we must move this into steps/ or at least parts
#include "helpers/transformation.hpp"


struct NotMatchBase {}; // needed for graph plotter

// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
struct NotMatch : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 NotMatchBase,
                 CouplingSlave
{
	SPECIAL_NODE_FUNCTIONS
	// Pattern is an abnormal context
    TreePtr<PRE_RESTRICTION> pattern;
    CompareReplace comp; // TODO only want the Compare
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) 
    {
        INDENT("!");
        ASSERT( pattern );
    	if( can_key )
    	{
    		// Don't do a subtree search while keying - we'll only end up keying the wrong thing
    		// or terminating with false prematurely
    		return true;
    	}
    	else
    	{
    	    // Do not use the present conjecture since we would mess it up because
    	    // a. We didn't recurse during KEYING pass and
    	    // b. Search under not can terminate with false, but parent search will continue
    	    // Consequently, we go in at Compare level, which creates a new conjecture.
    	    comp.pcontext = sr->pcontext;
    		bool r = comp.Compare( x, pattern, true );
			TRACE("NotMatch pattern=")(*pattern)(" x=")(*x)(" got %d, returning the opposite!\n", (int)r);
    		if( r==false )
				return true;
			else
				return false;
    	}
    }
    virtual void SetCouplingsMaster( CouplingKeys *ck )
    {
        comp.coupling_keys.SetMaster( ck ); 
    }
};

struct MatchAllBase : virtual Node
{
    virtual CollectionInterface &GetPatterns() = 0;
};

// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
struct MatchAll : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 CompareReplace::SoftReplacePattern, 
                 MatchAllBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
    MatchAll() : initialised( false ) {}
private:
    mutable bool initialised;
    mutable TreePtr<Node> modifier_pattern;
    virtual bool DecidedCompare( const CompareReplace *sr,
                                   TreePtr<Node> x,
                                   bool can_key,
                                   Conjecture &conj ) 
    {
        INDENT("&");
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		bool r = sr->DecidedCompare( x, TreePtr<Node>(i), can_key, conj );
    	    if( !r )
    	    	return false;
    	}
        return true;
    }

    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
        return TreePtr<Node>();
        //ASSERTFAIL("MatchAll in replace - it should be coupled because it should be in the search pattern\n");
    }

    void EnsureInitialised()
    {
        if( initialised )
            return;
    
        // MatchAll can appear in replace path; if so, see whether any patterns contain
        // modifiers, which change the tree relative to the present coupling. If so, 
        // overlay that over the coupling. There must not be more than one modifier, 
        // because behaviour would be indeterminate (patterns are unordered).
        //TRACE("Coupled MatchAll: dest=")(*dest)("\n");
        // TODO can we not arrange for the replace path to go directly to the desired child?
        // maybe by inserting an Overlay? Then we could make MatchAll be search-only

        FOREACH( TreePtr<Node> source_pattern, GetPatterns() )
        {                
            Expand e(source_pattern);
            FOREACH( TreePtr<Node> n, e )
            {
                if( dynamic_pointer_cast<SoftReplacePattern>(n) || 
                    dynamic_pointer_cast<OverlayBase>(n) ||
                    dynamic_pointer_cast<SlaveBase>(n) ) // TODO common base class for these called Modifier
                {
                    ASSERT( !modifier_pattern )("MatchAll coupled into replace must have no more than one modifying pattern:")
                          (" first saw ")(*modifier_pattern)(" and now got ")(*n)("\n");   
                    modifier_pattern = source_pattern;
                    break;
                }
            }                     
        }
        initialised = true;
    }

    virtual TreePtr<Node> GetOverlayPattern()
    {
        EnsureInitialised();
        return modifier_pattern;
    } 
    
    CollectionInterface &GetPatterns() { return patterns; } // TODO try covariant?
};

struct MatchAnyBase {};

// Match zero or more of the supplied patterns (between you and me, it's an OR)
template<class PRE_RESTRICTION>
struct MatchAny : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 MatchAnyBase
{
	SPECIAL_NODE_FUNCTIONS
	// Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) 
    {
        INDENT("|");
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		bool r = sr->Compare( x, i, false );
    	    if( r )
    	    	return true;
    	}
        return false;
    }
};


struct MatchOddBase {};

// Match an odd number of patterns (between you and me, it's an EOR)
template<class PRE_RESTRICTION>
struct MatchOdd : Special<PRE_RESTRICTION>,
                  CompareReplace::SoftSearchPattern,
                  MatchOddBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) 
    {
        INDENT("^");
    	int tot=0;
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    	    bool r = sr->Compare( x, i, false );
    	    if( r )
    	    	tot++;
    	}
        return (tot%2) ? true : false;
    }
};


struct TransformOfBase : CompareReplace::SoftSearchPattern 
{
    TreePtr<Node> pattern; 
    Transformation *transformation;
    TransformOfBase( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

private:
    virtual bool DecidedCompare( const CompareReplace *sr,
    		                                            TreePtr<Node> x,
    		                                            bool can_key,
    		                                            Conjecture &conj ) ;
};

template<class PRE_RESTRICTION>
struct TransformOf : TransformOfBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS	
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfBase(t, p) 
    {
    }
};


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
    BuildInstanceIdentifier( string s, int f=0 ) : BuildIdentifierBase(s,f) {}
    BuildInstanceIdentifier() : BuildIdentifierBase("unnamed") {}
    SPECIAL_NODE_FUNCTIONS
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
    BuildTypeIdentifier( string s="Unnamed", int f=0 ) : BuildIdentifierBase(s,f) {}
    SPECIAL_NODE_FUNCTIONS
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
    BuildLabelIdentifier( string s, int f=0 ) : BuildIdentifierBase(s,f) {}
    BuildLabelIdentifier() : BuildIdentifierBase("UNNAMED") {}
    SPECIAL_NODE_FUNCTIONS
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
	string newname = GetNewName( sr );
	return TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) );
    }                                               
};



// These can be used in search pattern to match a SpecificIdentifier by name.
// (cannot do this using a SpecificIdentifier in the search pattern because
// the address of the node would be compared, not the name string).
struct IdentifierByNameBase : CompareReplace::SoftSearchPattern
{
    IdentifierByNameBase( string n ) : name(n) {}
    bool IsMatch( const CompareReplace *sr, TreePtr<Node> x );
    string name;
};

struct InstanceIdentifierByName : Special<CPPTree::InstanceIdentifier>,                             
                                 IdentifierByNameBase
{
    InstanceIdentifierByName( string n ) : IdentifierByNameBase(n) {}
    SPECIAL_NODE_FUNCTIONS
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                TreePtr<Node> x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};

struct TypeIdentifierByName : Special<CPPTree::TypeIdentifier>,                             
                             IdentifierByNameBase
{
    TypeIdentifierByName( string n ) : IdentifierByNameBase(n) {}
    SPECIAL_NODE_FUNCTIONS
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                TreePtr<Node> x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};

struct LabelIdentifierByName : Special<CPPTree::LabelIdentifier>,                             
                              IdentifierByNameBase
{
    LabelIdentifierByName( string n ) : IdentifierByNameBase(n) {}

    SPECIAL_NODE_FUNCTIONS
private:
    virtual bool DecidedCompare( const CompareReplace *sr,
                                TreePtr<Node> x,
                                bool can_key,
                                Conjecture &conj )
    {
        return IsMatch( sr, x );
    }                                
};







#endif
