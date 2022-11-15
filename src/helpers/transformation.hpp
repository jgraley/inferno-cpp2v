#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class TreeKit
{
public:	
    class UnknownNode : public Exception {};
	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;
	
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;
    virtual ~TreeKit();
};


class ReferenceTreeKit : public TreeKit
{
public:	
	ReferenceTreeKit( TreePtr<Node> context_ );
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
	
private:
	TreePtr<Node> context;
};


class Transformation : public virtual Graphable
{
public:

    // The so-called augmented tree pointer can be uysed as a tree
    // pointer but it also holds other info, in this case a pointer
    // to a tree ptr (i.e. a double pointer). This is used to 
    // disambiguate which parent is meant when there can be more 
    // than one. It can also be constructed without the double
    // pointer, for when creating new free nodes.
    template<class VALUE_TYPE>
    class AugTreePtr : public TreePtr<VALUE_TYPE>
    {
    public:
        AugTreePtr() :
            p_tree_ptr(nullptr)
        {
        }
        
        // If a pointer was provided we keep the pointer. Usage style
        // is typically AugTreePtr<NodeType>( &parent->child_ptr );
        template<class OTHER_VALUE_TYPE>
        explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> *p_tree_ptr_) : 
            TreePtr<VALUE_TYPE>(*p_tree_ptr_), 
            p_tree_ptr(p_tree_ptr_) 
        {
            ASSERTS( p_tree_ptr );
            ASSERTS( *p_tree_ptr );
            // Not a local automatic please, we're going to hang on to it.
            ASSERTS( !ON_STACK(p_tree_ptr_) );
        }
            
        // If a value was provided the pointer is NULL 
        template<class OTHER_VALUE_TYPE>
        explicit AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr) : 
            TreePtr<VALUE_TYPE>(tree_ptr), 
            p_tree_ptr(nullptr)
        {
            ASSERTS( tree_ptr );
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
       
        template<class OTHER_VALUE_TYPE>
        static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
        {
            return AugTreePtr(TreePtr<VALUE_TYPE>::DynamicCast(g), g);
        }

        const TreePtrInterface *p_tree_ptr;
    };
    
    // Apply this transformation to tree at root, using context for decls etc.
    virtual AugTreePtr<Node> operator()( const TreeKit &kit, // Handy functions
    		                             TreePtr<Node> node ) = 0;    // Root of the subtree we want to modify    		                          
};

#endif
