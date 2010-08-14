#ifndef WALK_HPP
#define WALK_HPP

#include "tree/generics.hpp"

class Traverse : public ContainerInterface
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
		virtual void Overwrite( pointer v ) const;
        virtual const bool IsOrdered() const;
        // Some additional operations specific to walk iterators
	    operator string() const;
	    void Advance();
	    iterator(); // makes "end" iterator
	private:
	    iterator( TreePtr<Node> root );
	    bool IsAtEndOfChildren() const;

        vector< ContainerInterface::iterator > children;
        int index;

	    friend class Traverse;
    };

	// Constructor for walk container. The actual container is lightweight - it can be
	// destructed and the iterators remain valid. A new one may be created with the
	// same parameters and it will be equivalent.
	Traverse( TreePtr<Node> root ); // root of the subtree to walk

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
class Walk : public ContainerInterface
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
		virtual void Overwrite( pointer v ) const;
        virtual const bool IsOrdered() const;
        // Some additional operations specific to walk iterators
	    operator string() const;
	    void AdvanceOver();
	    void AdvanceInto();
	    iterator(); // makes "end" iterator
	private:
	    iterator( TreePtr<Node> &root, TreePtr<Node> restrictor );
	    bool IsAtEndOfChildren() const;
	    void BypassEndOfChildren();
	    void Push( TreePtr<Node> n );

	    shared_ptr< TreePtr<Node> > root;
	    TreePtr<Node> restrictor;
	    stack< Traverse::iterator > state;
	    bool done;

	    friend class Walk;
    };

	// Constructor for walk container. The actual container is lightweight - it can be
	// destructed and the iterators remain valid. A new one may be created with the
	// same parameters and it will be equivalent.
	Walk( TreePtr<Node> root, // root of the subtree to walk
		  TreePtr<Node> restrictor = TreePtr<Node>() ); // optional restrictor; walk skips these and does not recurse under them

	// Standard container ops, note that modification is not allowed through container interface
	virtual const iterator &begin();
    virtual const iterator &end();
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
private:
    TreePtr<Node> root;
    TreePtr<Node> restrictor;
    iterator my_begin, my_end;
};

#endif
