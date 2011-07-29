#ifndef WALK_HPP
#define WALK_HPP

#include "node/node.hpp"
#include "transformation.hpp"

/// Iterator for Flatten
class Flatten_iterator : public ContainerInterface::iterator_interface
{
public:
	// Standard types for stl compliance (note that the iterators are implicitly const)
    typedef forward_iterator_tag iterator_category;
    typedef TreePtrInterface value_type;
    typedef int difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;

	// Copy constructor and standard iterator operations
	Flatten_iterator( const Flatten_iterator & other );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	virtual Flatten_iterator &operator++();
	virtual reference operator*() const;
	virtual pointer operator->() const;
	virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
	virtual bool operator!=( const ContainerInterface::iterator_interface &ib ) const { return !operator==(ib); }
	virtual void Overwrite( pointer v ) const;
    virtual const bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    operator string() const;
    Flatten_iterator(); // makes "end" iterator
    Flatten_iterator( TreePtr<Node> root );
private:
    bool IsAtEnd() const;
    void BypassEndOfContainer();
    void NormaliseNewMember();
    inline Itemiser::Element *GetCurrentMember() const
    {
    	ASSERT( mit != m_end );
    	return root->ItemiseIndex( mit );
    }

    int mit;
    int m_end;
    ContainerInterface::iterator cit;
    ContainerInterface::iterator c_end;
    TreePtr<Node> root;
    bool empty; // TODO use NULL root for empty
};

/** Stated out traversal across a node's children. Traverses the members and elements of containers
    but does not follow any TreePtr. Basically an itemise that expands containers. */
typedef ContainerFromIterator< Flatten_iterator, TreePtr<Node> > Flatten;


/// Iterator for Expand
class Expand_iterator : public ContainerInterface::iterator_interface
{
public:
	// Standard types for stl compliance (note that the iterators are implicitly const)
    typedef forward_iterator_tag iterator_category;
	typedef TreePtrInterface value_type;
	typedef int difference_type;
	typedef const value_type *pointer;
	typedef const value_type &reference;

	// Copy constructor and standard iterator operations
	Expand_iterator( const Expand_iterator & other );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	virtual Expand_iterator &operator++();
	virtual reference operator*() const;
	virtual pointer operator->() const;
	virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
	virtual bool operator!=( const ContainerInterface::iterator_interface &ib ) const { return !operator==(ib); }
	virtual void Overwrite( pointer v ) const;
    virtual const bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    operator string() const;
    virtual void AdvanceOver();
    virtual void AdvanceInto();
    Expand_iterator(); // makes "end" iterator
    Expand_iterator( TreePtr<Node> &root,
                     Filter *out_filter = NULL,
    		         Filter *recurse_filter = NULL );
protected:
    virtual void DoNodeFilter();
    bool IsAtEndOfChildren() const;
    void BypassEndOfChildren();
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const;
    void Push( TreePtr<Node> n );

    shared_ptr< TreePtr<Node> > root;
    Filter *out_filter;
    Filter *recurse_filter;
    struct StateEntry
    {
        shared_ptr<ContainerInterface> container;
        ContainerInterface::iterator iterator;
    };
    stack< StateEntry > state;
    bool done;   
};


/** Inferno's tree-walking class. This is a stated out depth-first tree walker.
    A walk object is constructed on a node (possibly with other params) and it acts
    like an OOStd container whose iterator walks the subtree with sucessive invocations
    of operator++. A walking loop may be created using FOREACH as with containers. 
    
    TODO create a seperate container/iterator filter template for the out_filter.
    Make it not execute the filter from the constructor. Simplify Traverse etc. */
typedef ContainerFromIterator< Expand_iterator, TreePtr<Node>, Filter *, Filter * > Expand;


/// Filter that only matches each Node one time, then not again until Reset() is called
struct UniqueFilter : public Filter
{
    virtual bool IsMatch( TreePtr<Node> context,
                          TreePtr<Node> root );
    void Reset() { seen.clear(); }                         
    Set< TreePtr<Node> > seen;    
};


/// Iterator for ParentTraverse
class ParentTraverse_iterator : public Expand::iterator
{
public:
    ParentTraverse_iterator(); // makes "end" iterator
    ~ParentTraverse_iterator(); 
    ParentTraverse_iterator( const ParentTraverse_iterator &other );
    ParentTraverse_iterator &operator=( const ParentTraverse_iterator &other );
    ParentTraverse_iterator( TreePtr<Node> &root );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;    
protected:
    UniqueFilter *unique_filter;
};

/** Version of Expand that only sees a node once for each parent i.e. 
    with a->c, b->c, c->d
    we get c twice but d only once (Expand would get d twice too) */
typedef ContainerFromIterator< ParentTraverse_iterator, TreePtr<Node> > ParentTraverse;


/// Iterator for Traverse
class Traverse_iterator : public Expand::iterator
{
public:
    Traverse_iterator(); // makes "end" iterator
    ~Traverse_iterator(); 
    Traverse_iterator( const Traverse_iterator &other );        
    Traverse_iterator &operator=( const Traverse_iterator &other );
    Traverse_iterator( TreePtr<Node> &root );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;    
protected:
    UniqueFilter *unique_filter;
};

/// Traverse presents each element exactly once, and skips NULL pointers
typedef ContainerFromIterator< Traverse_iterator, TreePtr<Node> > Traverse;

#endif

