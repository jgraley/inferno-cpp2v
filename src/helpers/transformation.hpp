#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>


// The so-called augmented tree pointer can be used as a tree
// pointer but it also holds other info, in this case a pointer
// to a tree ptr (i.e. a double pointer). This is used to 
// disambiguate which parent is meant when there can be more 
// than one (Tree-style). It can also be constructed without the double
// pointer, for when creating new free nodes (Free-style).
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
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr_, const TreePtrInterface *p_tree_ptr_) : 
        TreePtr<VALUE_TYPE>(tree_ptr_), 
        p_tree_ptr(p_tree_ptr_) 
    {
        ASSERTS( *p_tree_ptr );
        // Not a local automatic please, we're going to hang on to it.
        ASSERTS( !ON_STACK(p_tree_ptr_) );
    }
        
    // Tree style constructor: if a pointer was provided we keep the pointer. Usage style
    // is typically AugTreePtr<NodeType>( &parent->child_ptr );
    template<class OTHER_VALUE_TYPE>
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> *p_tree_ptr_) : 
        TreePtr<VALUE_TYPE>(*p_tree_ptr_), 
        p_tree_ptr(p_tree_ptr_) 
    {
        ASSERTS( *p_tree_ptr );
        // Not a local automatic please, we're going to hang on to it.
        ASSERTS( !ON_STACK(p_tree_ptr_) );
    }
        
    // Free style constructor: if a value was provided the pointer is NULL 
    template<class OTHER_VALUE_TYPE>
    explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        p_tree_ptr(nullptr)
    {
    }
    
private:
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr, const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        p_tree_ptr(other.p_tree_ptr)
    {
    }

public:
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        TreePtr<VALUE_TYPE>(other), 
        p_tree_ptr(other.p_tree_ptr)
    {
    }
            
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<VALUE_TYPE> &other ) = default;

    template<class OTHER_VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<OTHER_VALUE_TYPE> &other )
    {
        TreePtr<VALUE_TYPE>::operator=(other); 
        p_tree_ptr = other.p_tree_ptr;
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
            return AugTreePtr<OTHER_VALUE_TYPE>(other_tree_ptr);
        else
            return AugTreePtr<OTHER_VALUE_TYPE>(*other_tree_ptr);
    }

    template<class OTHER_VALUE_TYPE>
    static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
    {
        return AugTreePtr(TreePtr<VALUE_TYPE>::DynamicCast(g), g);
    }

    const TreePtrInterface *p_tree_ptr;
};

class ReportingTreeAccess
{
    // Use Tree style when parent is another AugTreePtr. Should always be
    // tree_access.Descend(b, &b->c) where b is a dyncast of a.
    template<class PARENT_TYPE, class CHILD_TYPE>
    AugTreePtr<CHILD_TYPE> Descend( const AugTreePtr<PARENT_TYPE> &parent_ptr, 
                                    TreePtr<CHILD_TYPE> *child_ptr ) const
    {
        return parent_ptr.Descend(child_ptr);
        // TODO report child_ptr as in-tree if parent_ptr was
    }

    // Tree Style
    template<class PARENT_TYPE, class CHILD_TYPE>
    AugTreePtr<CHILD_TYPE> Descend( const TreePtr<PARENT_TYPE> &parent_ptr, 
                                    TreePtr<CHILD_TYPE> *child_ptr ) const
    {
        return AugTreePtr<CHILD_TYPE>(child_ptr);
        // TODO report child_ptr as in-tree if parent_ptr was
    }

    // Free Style
    template<class PARENT_TYPE, class CHILD_TYPE>
    AugTreePtr<CHILD_TYPE> Descend( const TreePtr<PARENT_TYPE> &parent_ptr, 
                                    TreePtr<CHILD_TYPE> child_ptr ) const
    {
        return AugTreePtr<CHILD_TYPE>(child_ptr);
        // TODO report child_ptr as in-tree if parent_ptr was
    }      
};



class TreeKit
{
public:	
    class UnknownNode : public Exception {};
	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;

    virtual bool IsRequireReports() const = 0;
	virtual set<LinkInfo> GetParents( TreePtr<Node> node ) const = 0;
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;
    virtual ~TreeKit();
};


class ReferenceTreeKit : public TreeKit
{
public:	
	ReferenceTreeKit( TreePtr<Node> context_ );

    bool IsRequireReports() const override;
	set<LinkInfo> GetParents( TreePtr<Node> node ) const override;
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
	
private:
	TreePtr<Node> context;
};


class Transformation : public virtual Graphable
{
public:

    
    // Apply this transformation to tree at root, using context for decls etc.
    virtual AugTreePtr<Node> operator()( const TreeKit &kit, // Handy functions
    		                             TreePtr<Node> node ) = 0;    // Root of the subtree we want to modify    		                          
};

#endif
