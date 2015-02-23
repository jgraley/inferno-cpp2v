#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "search_container_agent.hpp"
#include "slave_agent.hpp"

namespace SR
{

/// Match if the supplied patterns does not match (between you and me, it's just a NOT)
struct NotMatchBase {}; // needed for graph plotter

/// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
class NotMatch : public Special<PRE_RESTRICTION>,
                 public SoftAgent,
                 public NotMatchBase
{
public:
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
    virtual void MyConfigure()
    {
        comp.coupling_keys.SetMaster( GetCouplingKeys() ); 
    }
};

/// Match all of the supplied patterns (between you and me, it's an AND)
class MatchAllBase : public virtual Node
{
public:
    virtual CollectionInterface &GetPatterns() = 0;
};

/// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
class MatchAll : public Special<PRE_RESTRICTION>,
                 public virtual SoftAgent, 
                 public MatchAllBase
{
public:
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
                if( (dynamic_pointer_cast<SoftAgent>(n) && !dynamic_pointer_cast<NotMatchBase>(n)) || 
                    dynamic_pointer_cast<OverlayAgent>(n) ||
                    dynamic_pointer_cast<SlaveAgent>(n) ) // TODO common base class for these called Modifier
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

    // NOTE: this is cheating slightly by overloading the Agent interface directly
    // but we need to be able to progress a general replace through the selected 
    // modifier pattern. The way to resolve is to make this an agent directly, not soft.
    TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode ) 
    {
        EnsureInitialised();
        ASSERT(modifier_pattern);
        return AsAgent(modifier_pattern)->BuildReplace(); 
    }
    
    CollectionInterface &GetPatterns() { return patterns; } // TODO try covariant?
};

/// Match zero or more of the supplied patterns (between you and me, it's an OR)
class MatchAnyBase {};

/// Match zero or more of the supplied patterns (between you and me, it's an OR)
template<class PRE_RESTRICTION>
class MatchAny : public Special<PRE_RESTRICTION>,
                 public SoftAgent,
                 public MatchAnyBase
{
public:
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


/// Match an odd number of patterns (between you and me, it's an EOR)
class MatchOddBase {};

/// Match an odd number of patterns (between you and me, it's an EOR)
template<class PRE_RESTRICTION>
class MatchOdd : public Special<PRE_RESTRICTION>,
                 public SoftAgent,
                 public MatchOddBase
{
public:
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


/// Match the output of some transformation against the child pattern 
class TransformOfBase : public SoftAgent
{
public:
    TreePtr<Node> pattern; 
    Transformation *transformation;
    TransformOfBase( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

private:
    virtual bool MyCompare( const TreePtrInterface &x );
    
protected: 
    TransformOfBase() {}    
};


/// Match the output of some transformation against the child pattern 
template<class PRE_RESTRICTION>
class TransformOf : public TransformOfBase,
                    public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS	
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfBase(t, p) 
    {
    }
};


/// Match based on the type of a child pointer
class PointerIsBase
{
};

/** Make an architype of the pointed-to type and compare that.
    So if in the program tree we have a->b and the search pattern is
    x->PointerIsBase->y, then a must match x, and the type of the pointer
    in a that points to b must match y. */
/// Match based on the type of a child pointer
template<class PRE_RESTRICTION>
class PointerIs : public Special<PRE_RESTRICTION>,
                  public SoftAgent,
                  public PointerIsBase // TODO document
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> pointer;
    virtual bool MyCompare( const TreePtrInterface &x )
    {
        INDENT("@");
        
        // Note: using MakeValueArchitype() means we need to be using the 
        // TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
        // Thus, it musat be passed around via const TreePtrInterface &
        
        TreePtr<Node> ptr_arch = x.MakeValueArchitype();
        
        return AbnormalCompare( ptr_arch, pointer );
        // TODO this used to use NormalCompare(), but that was keying the architype
        // which is not generally a final node hence invalid key. Fid a suitable keying
        // policy if one exists.
    }
};

};
#endif
