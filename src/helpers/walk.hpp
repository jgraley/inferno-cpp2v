#ifndef WALK_HPP
#define WALK_HPP

#include "tree/generics.hpp"
#include "transformation.hpp"

//
// Stated out traversal across a node's children. Traverses the members and elements of containers
// but does not follow any shared_ptrs. Basically an itemise that expands containers.
//
class Flatten : public ContainerInterface
{
public:
	// The walking iterator does all the real work.
	class iterator : public ContainerInterface::iterator_interface
	{
	public:
		// Standard types for stl compliance (note that the iterators are implicitly const)
	    typedef forward_iterator_tag iterator_category;
		typedef TreePtrInterface value_type;
		typedef int difference_type;
		typedef const value_type *pointer;
		typedef const value_type &reference;

		// Copy constructor and standard iterator operations
		iterator( const iterator & other );
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
		virtual iterator &operator++();
		virtual reference operator*() const;
		virtual pointer operator->() const;
		virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
		virtual bool operator!=( const ContainerInterface::iterator_interface &ib ) const { return !operator==(ib); }
		virtual void Overwrite( pointer v ) const;
        virtual const bool IsOrdered() const;
        // Some additional operations specific to walk iterators
	    operator string() const;
	    iterator(); // makes "end" iterator
	private:
	    iterator( TreePtr<Node> root );
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

	    friend class Flatten;
    };

	// Constructor for walk container. The actual container is lightweight - it can be
	// destructed and the iterators remain valid. A new one may be created with the
	// same parameters and it will be equivalent.
	Flatten( TreePtr<Node> root ); // root of the subtree to walk

	// Standard container ops, note that modification is not allowed through container interface
	virtual const iterator &begin();
    virtual const iterator &end();
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
private:
    TreePtr<Node> root;
    iterator my_begin, my_end;
};


//
// Inferno's tree-walking class. This is a stated out depth-first tree walker.
// A walk object is constructed on a node (possibly with other params) and it acts
// like an OOStd container whose iterator walks the subtree with sucessive invocations
// of operator++. A welking loop may be created using FOREACH as with containers.
//
class Expand : public ContainerInterface
{
public:
	// The walking iterator does all the real work.
	class iterator : public ContainerInterface::iterator_interface
	{
	public:
		// Standard types for stl compliance (note that the iterators are implicitly const)
	    typedef forward_iterator_tag iterator_category;
		typedef TreePtrInterface value_type;
		typedef int difference_type;
		typedef const value_type *pointer;
		typedef const value_type &reference;

		// Copy constructor and standard iterator operations
		iterator( const iterator & other );
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
		virtual iterator &operator++();
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
	    iterator(); // makes "end" iterator
	protected:
        virtual void DoNodeFilter();
	    iterator( TreePtr<Node> &root,
                  Filter *out_filter,
	    		  Filter *recurse_filter );
	    bool IsAtEndOfChildren() const;
	    void BypassEndOfChildren();
	    void Push( TreePtr<Node> n );

	    shared_ptr< TreePtr<Node> > root;
        Filter *out_filter;
        Filter *recurse_filter;
	    stack< Flatten::iterator > state;
        bool done;

	    friend class Expand;
    };

	// Constructor for walk container. The actual container is lightweight - it can be
	// destructed and the iterators remain valid. A new one may be created with the
	// same parameters and it will be equivalent.
	Expand( TreePtr<Node> root, // root of the subtree to walk
            Filter *out_filter = NULL,
		    Filter *recurse_filter = NULL ); 

	// Standard container ops, note that modification is not allowed through container interface
	virtual const iterator &begin();
    virtual const iterator &end();
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
protected:
    TreePtr<Node> root;
    Filter *out_filter;
    Filter *recurse_filter;
    iterator my_begin, my_end;
};

struct UniqueFilter : public Filter
{
    virtual bool IsMatch( TreePtr<Node> context,
                          TreePtr<Node> root );
    Set< TreePtr<Node> > seen;    
};


// Version of walk that only sees a node once for each parent i.e. 
// a\
// b-c-d sees c twice but d only once (Expand would see d twice too)
//
// TODO do this as a recursion restriction on Expand - that means cleaning
// up the recurse restriction interface to be more like an observber pattern
// with support for multiple observers. The only tricky part is the "keys" 
// user data item, which should be a member of the client, but isn't.
class ParentTraverse : public ContainerInterface
{
public:
    // The walking iterator does all the real work.
    class iterator : public Expand::iterator
    {
    public:
        iterator(); // makes "end" iterator
        ~iterator(); 
        iterator( const iterator &other );
        iterator &operator=( const iterator &other );
    protected:
        iterator( TreePtr<Node> &root );
        UniqueFilter *unique_filter;
        friend class ParentTraverse;
    };
    ParentTraverse( TreePtr<Node> root );
    virtual const iterator &begin();
    virtual const iterator &end();
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
protected:
    TreePtr<Node> root;
    iterator my_begin, my_end;
};


class Traverse : public ContainerInterface
{
public:
    // The walking iterator does all the real work.
    class iterator : public Expand::iterator
    {
    public:
        iterator(); // makes "end" iterator
        ~iterator(); // makes "end" iterator
        iterator( const iterator &other );        
        iterator &operator=( const iterator &other );
    protected:
        iterator( TreePtr<Node> &root );
        Set< TreePtr<Node> > seen;
        UniqueFilter *unique_filter;
        friend class Traverse;
    };
    Traverse( TreePtr<Node> root );
    virtual const iterator &begin();
    virtual const iterator &end();
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
protected:
    TreePtr<Node> root;
    iterator my_begin, my_end;
};

#endif
