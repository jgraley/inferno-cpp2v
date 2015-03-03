#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "soft_agent.hpp"
#include "overlay_agent.hpp"
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
    		bool r = AbnormalCompare( x, pattern );
			TRACE("NotMatch pattern=")(*pattern)(" x=")(*x)(" got %d, returning the opposite!\n", (int)r);
    		if( r==false )
				return true;
			else
				return false;
    	}
    }
};

/// Match all of the supplied patterns (between you and me, it's an AND)
class MatchAllBase {};

/// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
class MatchAll : public Special<PRE_RESTRICTION>,
                 public virtual SoftAgent, 
                 public MatchAllBase
{
public:
    SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
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
                  public PointerIsBase 
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
