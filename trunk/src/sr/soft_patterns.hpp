#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "search_container_agent.hpp"

namespace SR
{

struct NotMatchBase {}; // needed for graph plotter

// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
struct NotMatch : Special<PRE_RESTRICTION>,
                 SoftPattern,
                 NotMatchBase,
                 CouplingSlave
{
	SPECIAL_NODE_FUNCTIONS
	// Pattern is an abnormal context. Fascinatingly, we can supply any node here because there
    // is no type-correctness limitation with *excluding* a kind of node
    TreePtr<Node> pattern;
    CompareReplace comp; // TODO only want the Compare
private:
    virtual bool MyCompare( const TreePtrInterface &x ) 
    {
        INDENT("!");
        ASSERT( pattern );
    	if( IsCanKey() )
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
    	    comp.pcontext = GetContext();
    		bool r = comp.Compare( x, pattern );
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
                  virtual SoftPattern, 
                  MatchAllBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
    MatchAll() : initialised( false ) {}
private:
    mutable bool initialised;
    mutable TreePtr<Node> modifier_pattern;
    virtual bool MyCompare( const TreePtrInterface &x )
    {
        INDENT("&");
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		bool r = NormalCompare( x, i );
    	    if( !r )
    	    	return false;
    	}
        return true;
    }

    virtual TreePtr<Node> MyBuildReplace()
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
            Walk e(source_pattern);
            FOREACH( TreePtr<Node> n, e )
            {
                if( (dynamic_pointer_cast<SoftPattern>(n) && !dynamic_pointer_cast<SoftPattern>(n)) || 
                    dynamic_pointer_cast<OverlayAgent>(n) ||
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
                 SoftPattern,
                 MatchAnyBase
{
	SPECIAL_NODE_FUNCTIONS
	// Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual bool MyCompare( const TreePtrInterface &x ) 
    {
        INDENT("|");
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		bool r = AbnormalCompare( x, i );
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
                  SoftPattern,
                  MatchOddBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual bool MyCompare( const TreePtrInterface &x ) 
    {
        INDENT("^");
    	int tot=0;
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    	    bool r = AbnormalCompare( x, i );
    	    if( r )
    	    	tot++;
    	}
        return (tot%2) ? true : false;
    }
};


struct TransformOfBase : SoftPatternSpecialKey,
                         TerminusBase
{
    TreePtr<Node> pattern; 
    Transformation *transformation;
    TransformOfBase( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

private:
    virtual shared_ptr<Key> MyCompare( const TreePtrInterface &x );
    
protected: 
    TransformOfBase() {}    
};

template<class PRE_RESTRICTION>
struct TransformOf : TransformOfBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS	
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfBase(t, p) 
    {
    }
};


struct PointerIsBase
{
};

/** Make an architype of the pointed-to type and compare that.
    So if in the program tree we have a->b and the search pattern is
    x->PointerIsBase->y, then a must match x, and the type of the pointer
    in a that points to b must match y. */
template<class PRE_RESTRICTION>
struct PointerIs : Special<PRE_RESTRICTION>,
                   SoftPattern,
                   PointerIsBase // TODO document
{
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> pointer;
    virtual bool MyCompare( const TreePtrInterface &x )
    {
        INDENT("@");
        
        // Note: using MakeValueArchitype() means we need to be using the 
        // TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
        // Thus, it musat be passed around via const TreePtrInterface &
        
        TreePtr<Node> ptr_arch = x.MakeValueArchitype();
        
        return NormalCompare( ptr_arch, pointer );
    }
};

};
#endif
