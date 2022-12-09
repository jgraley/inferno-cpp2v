#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class Transformation;

class DependencyReporter
{
public:	
	// We actually require a pointer to the parent node's TreePtr 
	// to the node in question.
	virtual void ReportTreeNode( const TreePtrInterface *p_tree_ptr ) = 0;
};

class NavigationUtils
{
public:	
    class UnknownNode : public Exception {};
	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;

    virtual bool IsRequireReports() const = 0;
	virtual set<LinkInfo> GetParents( TreePtr<Node> node ) const = 0;
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;
    virtual ~NavigationUtils();
};

struct TreeKit
{
    const NavigationUtils *nav;
    DependencyReporter *dep_rep;
};


// The augmented tree pointer is designed to act like a normal TreePtr
// (to an extent) while hepling to meet the requirements of 
template<class VALUE_TYPE>
class AugTreePtr : public TreePtr<VALUE_TYPE>
{
public:
    AugTreePtr() :
        p_tree_ptr(nullptr)
    {
    }
    
    // Explicit constructor
    template<class OTHER_VALUE_TYPE>
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr_, const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_) : 
        TreePtr<VALUE_TYPE>(tree_ptr_), 
        p_tree_ptr(p_tree_ptr_),
        dep_rep( dep_rep_ )
    {
        ASSERTS( *p_tree_ptr );
        // Not a local automatic please, we're going to hang on to it.
        ASSERTS( !ON_STACK(p_tree_ptr_) );
        if( dep_rep )
			dep_rep->ReportTreeNode( p_tree_ptr );
    }
        
    // Tree style constructor: if a pointer was provided we keep the pointer. Usage style
    // is typically AugTreePtr<NodeType>( &parent->child_ptr );
    template<class OTHER_VALUE_TYPE>
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> *p_tree_ptr_, DependencyReporter *dep_rep_ ) : 
        TreePtr<VALUE_TYPE>(*p_tree_ptr_), 
        p_tree_ptr(p_tree_ptr_),
        dep_rep( dep_rep_ )
    {
        ASSERTS( *p_tree_ptr );
        // Not a local automatic please, we're going to hang on to it.
        ASSERTS( !ON_STACK(p_tree_ptr_) );
        if( dep_rep )
			dep_rep->ReportTreeNode( p_tree_ptr );
    }
        
    // Free style constructor: if a value was provided the pointer is NULL 
    template<class OTHER_VALUE_TYPE>
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        p_tree_ptr(nullptr),
        dep_rep( nullptr )
    {
    }
    
private:
	// For the dyncast
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr, const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        p_tree_ptr(other.p_tree_ptr), 
        dep_rep(other.dep_rep)
    {
    }

public:
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        TreePtr<VALUE_TYPE>(other), 
        p_tree_ptr(other.p_tree_ptr), 
        dep_rep(other.dep_rep)
    {
    }
            
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<VALUE_TYPE> &other ) = default;

    template<class OTHER_VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<OTHER_VALUE_TYPE> &other )
    {
        TreePtr<VALUE_TYPE>::operator=(other); 
        p_tree_ptr = other.p_tree_ptr;
        dep_rep = other.dep_rep;
        return *this;
    }
   
    operator bool()
    {
        return TreePtr<VALUE_TYPE>::operator bool(); 
    }

    // Use Tree style when parent is another AugTreePtr. Should always be
    // a.Descend(&b->c) where b is a dyncast of a.
    template<class OTHER_VALUE_TYPE>
    AugTreePtr<OTHER_VALUE_TYPE> Descend( TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr ) const
    {
        // If we are Tree construct+return Tree style, otherwise reduce to Free style. This
        // is to stop descendents of Free masquerading as Tree.
        if( p_tree_ptr )
            return AugTreePtr<OTHER_VALUE_TYPE>(other_tree_ptr, dep_rep);
        else
            return AugTreePtr<OTHER_VALUE_TYPE>(*other_tree_ptr);
    }


    template<class OTHER_VALUE_TYPE>
    static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
    {
        return AugTreePtr(TreePtr<VALUE_TYPE>::DynamicCast(g), g);
    }

    const TreePtrInterface *p_tree_ptr;
    DependencyReporter *dep_rep;
};


class Transformation : public virtual Graphable
{
public:       
    // Apply this transformation to tree at node, using root for decls etc.
    // This entry point is for non-VN use cases, without dmain extension.
    AugTreePtr<Node> operator()( TreePtr<Node> node, 
    		                     TreePtr<Node> root ) const;
                                         	                          
    // Apply this transformation to tree at node, using kit for decls etc.
    // Vida Nova implementation with a TreeKit for VN integration: see
    // comment by RunTeleportQuery().
    virtual AugTreePtr<Node> ApplyTransformation( const TreeKit &kit, // Handy functions
    		                                      TreePtr<Node> node ) const = 0;    // Root of the subtree we want to modify    		                          
};


class ReferenceNavigationUtils : public NavigationUtils
{
public:	
	ReferenceNavigationUtils( TreePtr<Node> root_ );

    bool IsRequireReports() const override;
	set<LinkInfo> GetParents( TreePtr<Node> node ) const override;
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
	
private:
	TreePtr<Node> root;
};


#endif
