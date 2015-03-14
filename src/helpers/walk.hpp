#ifndef WALK_HPP
#define WALK_HPP

#include "node/node.hpp"
#include "transformation.hpp"

/// Iterator for FlattenNode
class FlattenNode_iterator : public ContainerInterface::iterator_interface
{
public:
	// Standard types for stl compliance (note that the iterators are implicitly const)
    typedef forward_iterator_tag iterator_category;
    typedef TreePtrInterface value_type;
    typedef int difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;

	// Copy constructor and standard iterator operations
	FlattenNode_iterator( const FlattenNode_iterator & other );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	virtual FlattenNode_iterator &operator++();
	virtual reference operator*() const;
	virtual pointer operator->() const;
	virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
	virtual bool operator!=( const ContainerInterface::iterator_interface &ib ) const { return !operator==(ib); }
	virtual void Overwrite( pointer v ) const;
    virtual const bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    string GetName() const;
    FlattenNode_iterator(); // makes "end" iterator
    FlattenNode_iterator( TreePtr<Node> root );
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

/*! Stated out traversal across a node's children. UniqueWalks the members and elements of containers
    but does not follow any TreePtr. Basically an itemise that expands containers. */
typedef ContainerFromIterator< FlattenNode_iterator, TreePtr<Node> > FlattenNode;


/// Iterator for Walk
class Walk_iterator : public ContainerInterface::iterator_interface
{
public:
	// Standard types for stl compliance (note that the iterators are implicitly const)
    typedef forward_iterator_tag iterator_category;
	typedef TreePtrInterface value_type;
	typedef int difference_type;
	typedef const value_type *pointer;
	typedef const value_type &reference;

	// Copy constructor and standard iterator operations
	Walk_iterator( const Walk_iterator & other );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	virtual Walk_iterator &operator++();
	virtual reference operator*() const;
	virtual pointer operator->() const;
	virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
	virtual bool operator!=( const ContainerInterface::iterator_interface &ib ) const { return !operator==(ib); }
	virtual void Overwrite( pointer v ) const;
    virtual const bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    string GetName() const;
    virtual void AdvanceOver();
    virtual void AdvanceInto();
    Walk_iterator(); // makes "end" iterator
    Walk_iterator( TreePtr<Node> &root,
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


/*! Inferno's tree-walking class. This is a stated out depth-first tree walker.
    A walk object is constructed on a node (possibly with other params) and it acts
    like an OOStd container whose iterator walks the subtree with sucessive invocations
    of operator++. A walking loop may be created using FOREACH as with containers. */
typedef ContainerFromIterator< Walk_iterator, TreePtr<Node>, Filter *, Filter * > Walk;


/// Filter that only matches each Node one time, then not again until Reset() is called
struct UniqueFilter : public Filter
{
    virtual bool IsMatch( TreePtr<Node> context,
                          TreePtr<Node> root );
    void Reset() { seen.clear(); }                         
    Set< TreePtr<Node> > seen;    
};


/// Iterator for ParentWalk
class ParentWalk_iterator : public Walk::iterator
{
public:
    ParentWalk_iterator(); // makes "end" iterator
    ~ParentWalk_iterator(); 
    ParentWalk_iterator( const ParentWalk_iterator &other );
    ParentWalk_iterator &operator=( const ParentWalk_iterator &other );
    ParentWalk_iterator( TreePtr<Node> &root );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;    
protected:
    UniqueFilter *unique_filter;
};

/*! Version of Walk that only sees a node once for each parent i.e. 
    with a->c, b->c, c->d
    we get c twice but d only once (Walk would get d twice too) */
typedef ContainerFromIterator< ParentWalk_iterator, TreePtr<Node> > ParentWalk;


/// Iterator for UniqueWalk
class UniqueWalk_iterator : public Walk::iterator
{
public:
    UniqueWalk_iterator(); // makes "end" iterator
    ~UniqueWalk_iterator(); 
    UniqueWalk_iterator( const UniqueWalk_iterator &other );        
    UniqueWalk_iterator &operator=( const UniqueWalk_iterator &other );
    UniqueWalk_iterator( TreePtr<Node> &root );
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;    
protected:
    UniqueFilter *unique_filter;
};

/*! UniqueWalk presents each element exactly once, and skips NULL pointers */
typedef ContainerFromIterator< UniqueWalk_iterator, TreePtr<Node> > UniqueWalk;

#endif

