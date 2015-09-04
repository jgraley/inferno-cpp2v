#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "soft_agent.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"

namespace SR
{
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
    virtual bool MyCompare( const TreePtrInterface &x ) const;
    
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
    virtual bool MyCompare( const TreePtrInterface &x ) const
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
