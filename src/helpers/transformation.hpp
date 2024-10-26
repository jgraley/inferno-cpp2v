#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include "common/lambda_loops.hpp"
#include <functional>

#define NEWCODE

class Transformation;

// ---------------------- DependencyReporter ---------------------------

class DependencyReporter
{
public:	
	// We actually require a pointer to the parent node's TreePtr 
	// to the node in question.
	virtual void ReportTreeNode( TreePtr<Node> tree_ptr ) = 0;
};

// ---------------------- AugTreePtrImpl ---------------------------

class AugTreePtrImpl
{
public:
	explicit AugTreePtrImpl();
	explicit AugTreePtrImpl( TreePtr<Node> generic_tree_ptr_ );
    explicit AugTreePtrImpl( const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_ );
	AugTreePtrImpl( const AugTreePtrImpl &other ) = default;	
	AugTreePtrImpl &operator=(const AugTreePtrImpl &other) = default;
	virtual AugTreePtrImpl *Clone() const;

	TreePtr<Node> GetGenericTreePtr() const;
    AugTreePtrImpl *GetChild( const TreePtrInterface *other_tree_ptr ) const;
    void SetChildChecks( const TreePtrInterface *other_tree_ptr, const AugTreePtrImpl *new_val ) const;

private:
    friend class TreeUtils;

    TreePtr<Node> generic_tree_ptr;
    const TreePtrInterface *p_tree_ptr;
    DependencyReporter *dep_rep;	    
};

// ---------------------- AugTreePtrBase ---------------------------

class AugTreePtrBase
{
public:
	explicit AugTreePtrBase();
	explicit AugTreePtrBase( TreePtr<Node> generic_tree_ptr_ );
    explicit AugTreePtrBase( const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_ );

	// Temporary - TODO just AugTreePtrBase( ValuePtr<AugTreePtrImpl> &&impl_)
	explicit AugTreePtrBase( ValuePtr<AugTreePtrImpl> &&impl_, TreePtr<Node> generic_tree_ptr_ );
    explicit AugTreePtrBase( ValuePtr<AugTreePtrImpl> &&impl_, const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_ );

	AugTreePtrBase( const AugTreePtrBase &other ) = default;	
	AugTreePtrBase &operator=(const AugTreePtrBase &other) = default;

	TreePtr<Node> GetGenericTreePtr() const;
    AugTreePtrBase GetChild( const TreePtrInterface *other_tree_ptr ) const;
    void SetChildChecks( const TreePtrInterface *other_tree_ptr, AugTreePtrBase new_val ) const;
    
protected:
    friend class TreeUtils;

	ValuePtr<AugTreePtrImpl> impl;

    TreePtr<Node> generic_tree_ptr;
    const TreePtrInterface *p_tree_ptr;
    DependencyReporter *dep_rep;	    
};

// ---------------------- AugTreePtr ---------------------------

// The augmented tree pointer is designed to act like a normal TreePtr
// (to an extent) while hepling to meet the requirements of domain extension
template<class VALUE_TYPE>
class AugTreePtr : public AugTreePtrBase,
			       public Traceable			   
			       
{
public:
    friend class TreeUtils;

	template<class OTHER_VALUE_TYPE>
	friend class AugTreePtr;

	// ------------ Constructors etc --------------
    AugTreePtr() :
        AugTreePtrBase()
    {
    }
         
    // Free style constructor: if a value was provided the pointer is NULL 
    // Not explicit so we can come in from parse and render
	// Construct from other type, safe casts only
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr_) : 
        AugTreePtrBase(tree_ptr_), // No back-end if derived from a TreePtr (legacy usage)
        tree_ptr(tree_ptr_) 
    {
    }
   
private:	
    explicit AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr_, const AugTreePtrBase &ob) : 
        AugTreePtrBase(ob),
        tree_ptr(tree_ptr_)
    {
    }

public:
	// Copy-construct from other type, safe casts only
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        AugTreePtrBase(other),
        tree_ptr(other.GetTreePtr())
    {
    }
            
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<VALUE_TYPE> &other ) = default;

    template<class OTHER_VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<OTHER_VALUE_TYPE> &other )
    {
        AugTreePtrBase::operator=(other);      
        tree_ptr = other.GetTreePtr(); 
        return *this;
    }     
   	
	// ------------ Access the wrapped, typed TreePtr() -------------
    TreePtr<VALUE_TYPE> &GetTreePtr()
	{
		return tree_ptr;
	}
   
    const TreePtr<VALUE_TYPE> &GetTreePtr() const 
	{
		return tree_ptr;
	}
      
    // ----------- Implementations for API -------------
    template<class OTHER_VALUE_TYPE>
    AugTreePtr<OTHER_VALUE_TYPE> GetChild( const TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr ) const
    {
		ASSERT( !ON_STACK(other_tree_ptr) );		
        return AugTreePtr<OTHER_VALUE_TYPE>(*other_tree_ptr, AugTreePtrBase::GetChild(other_tree_ptr)); 
    }

    // Should always be a.SetChild(&a->c, newval)
    template<class OTHER_VALUE_TYPE, class NEW_VALUE_TYPE>
    void SetChild( TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr, AugTreePtr<NEW_VALUE_TYPE> new_val )
    {
		ASSERT( !ON_STACK(other_tree_ptr) );		
		*other_tree_ptr = new_val.GetTreePtr(); // Update the type-safe free tree
        AugTreePtrBase::SetChildChecks(other_tree_ptr, new_val); // Let base decide what to do
    }

    template<class OTHER_VALUE_TYPE>
    static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
    {
		auto new_tree_ptr = TreePtr<VALUE_TYPE>::DynamicCast(g.GetTreePtr());
		if( new_tree_ptr ) // Dynamic cast can fail
			return AugTreePtr(new_tree_ptr, g); // success, keep the same Base
		else
			return AugTreePtr(); // failed, all to NULL
    }
    
	template<typename C>
	void ForAugContainer( C *container, 
						  function<void(const AugTreePtr<typename C::value_type::value_type> &x_atp)> func) const try
	{
		for( const auto &x : *container )
		{
			AugTreePtr<typename C::value_type::value_type> x_atp = GetChild(&x);
			func( x_atp );
		}
	}
	catch( BreakException )
	{
	}    
	
	// -------------- Smart pointer methods (will report a dep leak) -----------------
	TreePtr<VALUE_TYPE> operator->()
	{
		// Dep leak!!
		return GetTreePtr();
	}

	const VALUE_TYPE &operator*()
	{
		// Dep leak!!
		return *GetTreePtr();
	}
	
	// Not allowed!!
	template<typename C>
	void operator[](C c) = delete;

	template<typename C>
	void operator->*(C c) = delete;
		
	// -------------- Other methods to be a useful type ------------------
		
    template<class OTHER_VALUE_TYPE>
	bool operator <( const AugTreePtr<OTHER_VALUE_TYPE> &r ) const
	{
		return GetTreePtr() < r.GetTreePtr();
	}	
	
    operator bool()
    {
		return !!tree_ptr;
	}
		
	string GetTrace() const override
	{
		return GetTreePtr().GetTrace()+"(Aug)";
	}		
	
	// -------------- data members -----------------	
	TreePtr<VALUE_TYPE> tree_ptr;
};


// Note that, in the case of soft nodes, this macro could stringize the FIELD, which
// would be the recommended style (as long as the field names are legal
// symbol values for the C++ preprocessor)
#define GET_CHILD( ATP, FIELD ) ((ATP).GetChild(&(ATP)->FIELD)) // singular
#define SET_CHILD( ATP, FIELD, NEWVAL ) ((ATP).SetChild(&(ATP)->FIELD, (NEWVAL))) // singular

#define GET_CHILD_BACK( ATP, FIELD ) ((ATP).GetChild(&(ATP)->FIELD.back()))
#define GET_CHILD_FRONT( ATP, FIELD ) ((ATP).GetChild(&(ATP)->FIELD.front()))

#define FOR_AUG_CONTAINER( ATP, FIELD, BODY ) ((ATP).ForAugContainer(&(ATP)->FIELD, (BODY)))


// TODO:
// Implement ATP::operator-> as a dep leak and do not use in these macros.
// Cast to bool is also a dep leak
// With -> we're a smart pointer. No need to derive from TreePtr but
// still contain one and still be a template. 

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

// ---------------------- TreeUtils ---------------------------

class TreeUtils
{
public:	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;

	explicit TreeUtils( const NavigationInterface *nav_, 
	                    DependencyReporter *dep_rep_ = nullptr );

	// Create AugTreePtr from a link
    AugTreePtr<Node> CreateAugTreePtr(const TreePtrInterface *p_tree_ptr) const;
	
	// Create Node and AugTreePtr 
	template<typename VALUE_TYPE, typename ... CP>
	AugTreePtr<VALUE_TYPE> MakeAugTreeNode(const CP &...cp) 
	{
		return AugTreePtr<VALUE_TYPE>( MakeTreeNode<VALUE_TYPE>(cp...) );
	}
	
	// Getters for AugTreePtr - back end only
    const TreePtrInterface *GetPTreePtr( const AugTreePtrBase &atp ) const;	
    TreePtr<Node> GetGenericTreePtr( const AugTreePtrBase &atp ) const;
	
	// Forwarding methods from NavigationInterface
	bool IsRequireReports() const;
	set<LinkInfo> GetParents( TreePtr<Node> node ) const; 
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const;
	
	set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const;
	
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

// For when you don't have a database - searches for things from the
// supplied root.
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
