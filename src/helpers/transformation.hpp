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

    template<class OTHER_VALUE_TYPE>
    void SetChild( const TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr, AugTreePtrBase new_val ) const
	{
		// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
		// is to stop descendents of Free masquerading as Tree.	
		if( p_tree_ptr )
		{
			ASSERT(new_val.p_tree_ptr); // can't have tree style -> free style: would modify tree
			ASSERT( !ON_STACK(other_tree_ptr) );
		}
		else if( new_val.p_tree_ptr )
		{
			// TODO add a terminus to free zone
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
class AugTreePtr : 
			       public Traceable,				   
			       public AugTreePtrBase
{
public:
    friend class TreeUtils;

	// ------------ Constructors etc --------------
    AugTreePtr() :
        AugTreePtrBase()
    {
    }
         
    // Free style constructor: if a value was provided the pointer is NULL 
    // Not explicit so we can come in from parse and render
    template<class OTHER_VALUE_TYPE>
    AugTreePtr(TreePtr<OTHER_VALUE_TYPE> tree_ptr_) : 
        typed_tree_ptr(tree_ptr_), 
        AugTreePtrBase(tree_ptr_)
    {
    }
   
    AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr_, const AugTreePtrBase &ob) : 
        typed_tree_ptr(tree_ptr_), 
        AugTreePtrBase(ob)
    {
    }

    template<class OTHER_VALUE_TYPE>
    AugTreePtr(const AugTreePtr<OTHER_VALUE_TYPE> &other) : 
        typed_tree_ptr(other.GetTypedTreePtr()), 
        AugTreePtrBase(other)
    {
    }
            
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<VALUE_TYPE> &other ) = default;

    template<class OTHER_VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> &operator=( const AugTreePtr<OTHER_VALUE_TYPE> &other )
    {
        SetTypedTreePtr( other.GetTypedTreePtr() ); 
        AugTreePtrBase::operator=(other);      
        return *this;
    }     
   	
	// ------------ Access the wrapped, typed TreePtr() -------------
    TreePtr<VALUE_TYPE> &GetTypedTreePtr()
	{
		return typed_tree_ptr;
	}
   
    const TreePtr<VALUE_TYPE> &GetTypedTreePtr() const 
	{
		return typed_tree_ptr;
	}
   
    template<class OTHER_VALUE_TYPE>
    void SetTypedTreePtr(const TreePtr<OTHER_VALUE_TYPE> &new_val)
	{
		typed_tree_ptr = new_val;
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
		*other_tree_ptr = new_val.GetTypedTreePtr(); // Update the type-safe free tree
        AugTreePtrBase::SetChild(other_tree_ptr, new_val); // Let base decide what to do
    }

    template<class OTHER_VALUE_TYPE>
    static AugTreePtr<VALUE_TYPE> DynamicCast( const AugTreePtr<OTHER_VALUE_TYPE> &g )
    {
		auto new_tree_ptr = TreePtr<VALUE_TYPE>::DynamicCast(g.GetTypedTreePtr());
        auto new_atp = AugTreePtr(new_tree_ptr, g);
		new_atp.SetTreePtr(new_tree_ptr); // could be NULL if dyn cast fails
		return new_atp;
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
		return GetTypedTreePtr();
	}

	const VALUE_TYPE &operator*()
	{
		// Dep leak!!
		return *GetTypedTreePtr();
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
		return GetTypedTreePtr() < r.GetTypedTreePtr();
	}	
		
	operator TreePtr<VALUE_TYPE>() const
	{
		// Dep leak!!
		return GetTypedTreePtr();
	}

	string GetTrace() const override
	{
		return GetTypedTreePtr().GetTrace();
	}
	
	// -------------- data members -----------------	
	TreePtr<VALUE_TYPE> typed_tree_ptr;
};

template<typename VALUE_TYPE, typename ... CP>
AugTreePtr<VALUE_TYPE> AugMakeTreeNode(const CP &...cp) 
{
    return AugTreePtr<VALUE_TYPE>( MakeTreeNode<VALUE_TYPE>(cp...) );
}

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

	// Create AugTreePtr
    template<class VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> CreateAugTree(const TreePtrInterface *p_tree_ptr) const
    {
		return AugTreePtr<VALUE_TYPE>((TreePtr<VALUE_TYPE>)*p_tree_ptr, AugTreePtrBase(p_tree_ptr, dep_rep));
	}	

    template<class VALUE_TYPE>
    AugTreePtr<VALUE_TYPE> CreateAugTree(const TreePtr<VALUE_TYPE> *p_tree_ptr) const
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
