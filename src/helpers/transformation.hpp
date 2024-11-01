#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include "common/lambda_loops.hpp"
#include <functional>

class Transformation;

// ---------------------- Dependencies ---------------------------

class Dependencies
{
public:	
	// We actually require a pointer to the parent node's TreePtr 
	// to the node in question.
	virtual void AddDep( TreePtr<Node> tree_ptr ) = 0;
};

// ---------------------- AugBEInterface ---------------------------

class AugBEInterface
{
public:
	virtual ~AugBEInterface() = default;
	virtual AugBEInterface *Clone() const = 0;
	
    virtual AugBEInterface *OnGetChild( const TreePtrInterface *other_tree_ptr ) = 0;
    virtual void OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val ) = 0;	
    virtual void OnDepLeak() = 0;
};

// ---------------------- AugTreePtrBase ---------------------------

class AugTreePtrBase
{
public:
	explicit AugTreePtrBase();
	explicit AugTreePtrBase( ValuePtr<AugBEInterface> &&impl_ );

	AugTreePtrBase( const AugTreePtrBase &other ) = default;	
	AugTreePtrBase &operator=(const AugTreePtrBase &other) = default;

	ValuePtr<AugBEInterface> GetImpl() const;
    AugTreePtrBase OnGetChild( const TreePtrInterface *other_tree_ptr ) const;
    void OnSetChild( const TreePtrInterface *other_tree_ptr, AugTreePtrBase new_val ) const;
    void OnDepLeak() const;

protected:
	ValuePtr<AugBEInterface> impl;
};

// ---------------------- AugTreePtr ---------------------------

// The augmented tree pointer is designed to act like a normal TreePtr
// (to an extent) while hepling to meet the requirements of domain extension
template<class VALUE_TYPE>
class AugTreePtr : public AugTreePtrBase,
			       public Traceable			   
			       
{
public:

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
        tree_ptr(tree_ptr_)  // No back-end if derived from a TreePtr (legacy usage)        
    {
    }
   	
    explicit AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr_, ValuePtr<AugBEInterface> &&impl_) : 
        AugTreePtrBase(move(impl_)),
        tree_ptr(tree_ptr_)
    {
    }

    explicit AugTreePtr(TreePtr<VALUE_TYPE> tree_ptr_, AugTreePtrBase base_) : 
        AugTreePtrBase(base_),
        tree_ptr(tree_ptr_)
    {
    }
    
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
        return AugTreePtr<OTHER_VALUE_TYPE>(*other_tree_ptr, OnGetChild(other_tree_ptr)); 
    }

    // Should always be a.SetChild(&a->c, newval)
    template<class OTHER_VALUE_TYPE, class NEW_VALUE_TYPE>
    void SetChild( TreePtr<OTHER_VALUE_TYPE> *other_tree_ptr, AugTreePtr<NEW_VALUE_TYPE> new_val )
    {
		ASSERT( !ON_STACK(other_tree_ptr) );		
		*other_tree_ptr = new_val.GetTreePtr(); // Update the type-safe free tree
        AugTreePtrBase::OnSetChild(other_tree_ptr, new_val); // Let base decide what to do
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
		OnDepLeak();
		return GetTreePtr();
	}

	const VALUE_TYPE &operator*()
	{
		OnDepLeak();
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
		// Not reporting a dep leak because this are only used by 
		// std::set, std::map etc
		// TODO make private and friend them
		return GetTreePtr() < r.GetTreePtr();
	}	
	
    operator bool()
    {
		OnDepLeak();		
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
#define GET_CHILD( ATP, FIELD ) ((ATP).GetChild(&(ATP).GetTreePtr()->FIELD)) // singular
#define SET_CHILD( ATP, FIELD, NEWVAL ) ((ATP).SetChild(&(ATP).GetTreePtr()->FIELD, (NEWVAL))) // singular

#define GET_CHILD_BACK( ATP, FIELD ) ((ATP).GetChild(&(ATP).GetTreePtr()->FIELD.back()))
#define GET_CHILD_FRONT( ATP, FIELD ) ((ATP).GetChild(&(ATP).GetTreePtr()->FIELD.front()))

#define FOR_AUG_CONTAINER( ATP, FIELD, BODY ) ((ATP).ForAugContainer(&(ATP).GetTreePtr()->FIELD, (BODY)))

// ---------------------- TransUtilsInterface ---------------------------

class TransUtilsInterface
{
public:    
    class UnknownNode : public Exception {};

	virtual ~TransUtilsInterface() = default;

	// TODO there is no reason not to implement GetParents() along the same lines
	// here and in the two impl classes.
	virtual set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const = 0;

	// Create Node and AugTreePtr 
	template<typename VALUE_TYPE, typename ... CP>
	AugTreePtr<VALUE_TYPE> MakeAugTreeNode(const CP &...cp) 
	{
		auto tp = MakeTreeNode<VALUE_TYPE>(cp...);
		return AugTreePtr<VALUE_TYPE>( tp, CreateBE(tp) );
	}

protected:	
    virtual ValuePtr<AugBEInterface> CreateBE( TreePtr<Node> tp ) const = 0;
};

// ---------------------- DefaultTransUtils ---------------------------

class DefaultTransUtils : public TransUtilsInterface
{
public:	
	explicit DefaultTransUtils( TreePtr<Node> root_ );	                           
	set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const override;
			
private:	
    ValuePtr<AugBEInterface> CreateBE( TreePtr<Node> tp ) const override;			
	TreePtr<Node> root;
};

// ---------------------- TransKit ---------------------------

struct TransKit
{
    TransUtilsInterface *utils;    
};

// ---------------------- Transformation ---------------------------

class Transformation : public virtual Graphable
{
public:       
    class TransformationFailedMismatch : public Mismatch {};

    // Apply this transformation to tree at node, using root for decls etc.
    // This entry point is for non-VN use cases, without dmain extension.
    AugTreePtr<Node> operator()( AugTreePtr<Node> node, 
    		                     TreePtr<Node> root ) const;
                                         	                          
    // Apply this transformation to tree at node, using kit for decls etc.
    // Vida Nova implementation with a TransKit for VN integration: see
    // comment by RunTeleportQuery().
    virtual AugTreePtr<Node> TryApplyTransformation( const TransKit &kit, // Handy functions
    		                                         AugTreePtr<Node> node ) const = 0;    // Root of the subtree we want to modify    		                          

    AugTreePtr<Node> ApplyTransformation( const TransKit &kit, // Handy functions
                                          AugTreePtr<Node> node ) const;    // Root of the subtree we want to modify    		                          
};

#endif
