#ifndef WALK_HPP
#define WALK_HPP

#include "node/specialise_oostd.hpp"
#include "transformation.hpp"
#include <deque>
#include <list>


class Filter
{
public:
    // Test the subtree at the supplied root, producing true or false
    virtual bool IsMatch( TreePtr<Node> context,       // The whole program, so declarations may be searched for
                          TreePtr<Node> root ) = 0;    // Root of the subtree we want to test
};


/// Iterator for Walk
class Walk_iterator : public ContainerInterface::iterator
{
public:
    // Standard types for stl compliance (note that the iterators are implicitly const)
    typedef forward_iterator_tag iterator_category;
    typedef TreePtrInterface value_type;
    typedef int difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;

    // Copy constructor and standard iterator operations
    Walk_iterator(); // makes "end" iterator
    Walk_iterator( const Walk_iterator & other );
    Walk_iterator& operator=(const Walk_iterator&) = default;
    Walk_iterator( TreePtr<Node> &root,
                   Filter *out_filter = nullptr,
                   Filter *recurse_filter = nullptr );
    virtual unique_ptr<ContainerInterface::iterator_interface> Clone() const;
    virtual Walk_iterator &operator++();
    virtual reference operator*() const;
    virtual pointer operator->() const;
    virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
    virtual bool operator==( const Walk_iterator &ib ) const { return operator==((ContainerInterface::iterator_interface &)ib); } // dismbiguation, see #830
    virtual void Mutate( pointer v ) const;
    virtual bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    string GetName() const;
    virtual void AdvanceOver();
    virtual void AdvanceInto();
    virtual TreePtr<Node> GetParent(int grandness=0) const; // Go back a whole step
    virtual const TreePtrInterface *GetNodePointerInParent(int grandness=0) const;// Go back half a step
    // Get all the true recursions made so far (i.e. excluding root and current position)
    virtual list< pair<TreePtr<Node>, const TreePtrInterface *> > GetCurrentPath() const;  // return current recurse nodes
    // NOTE on recurse nodes. Recuse nodes are the nodes that were "passed through" 
    // (flattened) on the way to the current node (i.e. *iterator). That means the 
    // current node is not a recurse node. The root node is not a recurse node at
    // begin(), when it is the current node. But it *is* a recurse node after iterator++
    // when it has been passed through in order to reach the current node (except
    // when we are at end() of course). Note: returns pairs of (node, parent_ptr)
    // except when node is root node when you get (NULL, NULL)
    
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
        TreePtr<Node> node;
        shared_ptr<ContainerInterface> container;
        ContainerInterface::iterator iterator;
    };
    // deque not stack so we can iterate the whole thing
    list< StateEntry > state; // Mapping: top is back
    bool done;   
};


/*! Inferno's tree-walking class. This is a stated out depth-first tree walker.
    A walk object is constructed on a node (possibly with other params) and it acts
    like an OOStd container whose iterator walks the subtree with sucessive invocations
    of operator++. A walking loop may be created using range-for as with containers. */
typedef ContainerFromIterator< Walk_iterator, TreePtr<Node>, Filter *, Filter * > Walk;


/// Filter that only matches each Node one time, then not again until Reset() is called
struct UniqueFilter : public Filter
{
	virtual ~UniqueFilter() = default;
    virtual bool IsMatch( TreePtr<Node> context,
                          TreePtr<Node> root );
    void Reset() { seen.clear(); }                         
    set< TreePtr<Node> > seen;    
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
    virtual unique_ptr<ContainerInterface::iterator_interface> Clone() const;    
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
    virtual unique_ptr<ContainerInterface::iterator_interface> Clone() const;    
protected:
    UniqueFilter *unique_filter;
};

/*! UniqueWalk presents each element exactly once, and skips nullptr pointers */
typedef ContainerFromIterator< UniqueWalk_iterator, TreePtr<Node> > UniqueWalk;

#endif

