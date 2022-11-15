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

    template<class VALUE_TYPE>
    class AugTreePtr : public TreePtr<VALUE_TYPE>
    {
    public:
        AugTreePtr() :
            p_tree_ptr(nullptr)
        {
        }
        
        // If a pointer was provided we keep the pointer.
        explicit AugTreePtr(TreePtr<VALUE_TYPE> *p_tree_ptr_) : 
            TreePtr<VALUE_TYPE>(*p_tree_ptr_), 
            p_tree_ptr(p_tree_ptr_) 
        {
        }
            
        // If a value was provided the pointer is NULL 
        explicit AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr) : 
            TreePtr<VALUE_TYPE>(tree_ptr), 
            p_tree_ptr(nullptr)
        {
        }
        
        template<class OTHER_VALUE_TYPE>
        AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr, const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
            TreePtr<VALUE_TYPE>(tree_ptr), 
            p_tree_ptr(other.p_tree_ptr)
        {
        }

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
       
        const TreePtrInterface *p_tree_ptr;
    };
    
// Handy macros for a node and a child pointer or just a node
#define PARENT_AND_CHILD( N, C ) AugTreePtr<decltype(N->C)::value_type>(&(N->C))
#define NODE_ONLY( N ) AugTreePtr<decltype(N)::value_type>(N)
#define CHANGE_NODE( N, NI ) AugTreePtr<decltype(N)::value_type>(N, NI)
#define GET_NODE( NI ) (NI)

    // Apply this transformation to tree at root, using context for decls etc.
    virtual AugTreePtr<Node> operator()( const TreeKit &kit, // Handy functions
    		                             TreePtr<Node> node ) = 0;    // Root of the subtree we want to modify    		                          
};

#endif
