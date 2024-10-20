#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class Transformation;

// ---------------------- DependencyReporter ---------------------------

class DependencyReporter
{
public:	
	// We actually require a pointer to the parent node's TreePtr 
	// to the node in question.
	virtual void ReportTreeNode( TreePtr<Node> tree_ptr ) = 0;
};

// ---------------------- NavigationInterface ---------------------------

class NavigationInterface
{
public:	
    class UnknownNode : public Exception {};
	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;

    virtual bool IsRequireReports() const = 0;
	virtual set<LinkInfo> GetParents( TreePtr<Node> node ) const = 0;
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;    
};

// ---------------------- AugTreePtrBase ---------------------------

class AugTreePtrBase
{
public:
	explicit AugTreePtrBase();
	explicit AugTreePtrBase( TreePtr<Node> tree_ptr_ );
    explicit AugTreePtrBase( const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_ );
	AugTreePtrBase( const AugTreePtrBase &other ) = default;
	
	AugTreePtrBase &operator=(const AugTreePtrBase &other) = default;

	TreePtr<Node> GetTreePtr() const;
	void SetTreePtr(TreePtr<Node> tree_ptr_);

    operator bool();

    template<class OTHER_VALUE_TYPE>
    AugTreePtrBase GetChild( const TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr ) const
	{
		// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
		// is to stop descendents of Free masquerading as Tree.	
		if( p_tree_ptr )
		{
			ASSERT( !ON_STACK(other_tree_ptr) );
			return AugTreePtrBase(other_tree_ptr, dep_rep); // tree
		}
		else
		{
			return AugTreePtrBase(*other_tree_ptr); // free
		}
	}

protected:
    friend class TreeUtils;

    TreePtr<Node> tree_ptr;
    const TreePtrInterface *p_tree_ptr;
    DependencyReporter *dep_rep;	
};

// ---------------------- AugTreePtr ---------------------------

// The augmented tree pointer is designed to act like a normal TreePtr
// (to an extent) while hepling to meet the requirements of domain extension
template<class VALUE_TYPE>
class AugTreePtr : public TreePtr<VALUE_TYPE>,
			       public AugTreePtrBase
{
public:
    friend class TreeUtils;

    AugTreePtr() :
        AugTreePtrBase()
    {
    }
         
    // Free style constructor: if a value was provided the pointer is NULL 
    // Not explicit so we can come in from parse and render
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        AugTreePtrBase(tree_ptr)
    {
    }
   
    AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr, const AugTreePtrBase &ob) : 
        TreePtr<VALUE_TYPE>(tree_ptr), 
        AugTreePtrBase(ob)
    {
    }

    template<class OTHER_VALUE_TYPE>
    AugTreePtr(const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        TreePtr<VALUE_TYPE>(other), 
        AugTreePtrBase(other)
    {
    }
            
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<VALUE_TYPE> &other ) = default;

    template<class OTHER_VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<OTHER_VALUE_TYPE> &other )
    {
        TreePtr<VALUE_TYPE>::operator=(other); 
        AugTreePtrBase::operator=(other);      
        return *this;
    }     
   
    // Use Tree style when parent is another AugTreePtr. Should always be
    // a.GetChild(&a->c)
    template<class OTHER_VALUE_TYPE>
    AugTreePtr<OTHER_VALUE_TYPE> GetChild( const TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr ) const
    {
		ASSERT( !ON_STACK(other_tree_ptr) );		
        return AugTreePtr<OTHER_VALUE_TYPE>(*other_tree_ptr, AugTreePtrBase::GetChild(other_tree_ptr)); 
    }

    template<class OTHER_VALUE_TYPE>
    static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
    {
		auto new_tree_ptr = TreePtr<VALUE_TYPE>::DynamicCast(g);
        auto new_atp = AugTreePtr(new_tree_ptr, g);
		new_atp.SetTreePtr(new_tree_ptr); // could be NULL if dyn cast fails
		return new_atp;
    }
};

template<typename VALUE_TYPE, typename ... CP>
TreePtr<VALUE_TYPE> AugMakeTreeNode(const CP &...cp) 
{
    return AugTreePtr<VALUE_TYPE>( MakeTreeNode<VALUE_TYPE>(cp...) );
}

// Note that, in the case of soft nodes, this macro could stringize the FIELD, which
// would be the recommended style (as long as the field names are legal
// symbol values for the C++ preprocessor)
#define CHILD_OF( ATP, FIELD ) ((ATP).GetChild(&(ATP)->FIELD))


// ---------------------- TreeUtils ---------------------------

class TreeUtils
{
public:	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;

	explicit TreeUtils( const NavigationInterface *nav_, 
	                    DependencyReporter *dep_rep_ = nullptr );

	// Create AugTreePtr
    template<class VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> CreateAugTree(const TreePtrInterface *p_tree_ptr)
    {
		return AugTreePtr<VALUE_TYPE>((TreePtr<VALUE_TYPE>)*p_tree_ptr, AugTreePtrBase(p_tree_ptr, dep_rep));
	}	

    template<class VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> CreateAugTree(const TreePtr<VALUE_TYPE> *p_tree_ptr)
    {
		return AugTreePtr<VALUE_TYPE>((TreePtr<VALUE_TYPE>)*p_tree_ptr, AugTreePtrBase(p_tree_ptr, dep_rep));
	}	

	// Getters for AugTreePtr - back end only
    const TreePtrInterface *GetPTreePtr( const AugTreePtrBase &atp ) const;	
    TreePtr<Node> GetTreePtr( const AugTreePtrBase &atp ) const;
	
	// Forwarding methods from NavigationInterface
	bool IsRequireReports() const;
	set<LinkInfo> GetParents( TreePtr<Node> node ) const; 
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const;
	
	//set<AugTreePtr<Node>> GetParents( AugTreePtr<Node> node ) const; 
	//set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const; 
	
	DependencyReporter *GetDepRep() const;
		
private:	
	const NavigationInterface * const nav;
	DependencyReporter *dep_rep;	
};

// ---------------------- TreeKit ---------------------------

struct TreeKit
{
    TreeUtils *utils;    
};

// ---------------------- Transformation ---------------------------

class Transformation : public virtual Graphable
{
public:       
    // Apply this transformation to tree at node, using root for decls etc.
    // This entry point is for non-VN use cases, without dmain extension.
    AugTreePtr<Node> operator()( AugTreePtr<Node> node, 
    		                     TreePtr<Node> root ) const;
                                         	                          
    // Apply this transformation to tree at node, using kit for decls etc.
    // Vida Nova implementation with a TreeKit for VN integration: see
    // comment by RunTeleportQuery().
    virtual AugTreePtr<Node> ApplyTransformation( const TreeKit &kit, // Handy functions
    		                                      AugTreePtr<Node> node ) const = 0;    // Root of the subtree we want to modify    		                          
};

// ---------------------- SimpleNavigation ---------------------------

class SimpleNavigation : public NavigationInterface
{
public:	
	SimpleNavigation( TreePtr<Node> root_ );

    bool IsRequireReports() const override;
	set<LinkInfo> GetParents( TreePtr<Node> node ) const override;
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
	
private:
	TreePtr<Node> root;
};


#endif
