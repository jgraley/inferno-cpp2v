#ifndef WALK_HPP
#define WALK_HPP

#include "tree/generics.hpp"


class Walk : public ContainerInterface
{
public:

	class iterator : public ContainerInterface::iterator_interface
	{
	    struct Frame
	    {
	        vector< ContainerInterface::iterator > children;
	        int index;
	    };

	    shared_ptr< TreePtr<Node> > root;
	    TreePtr<Node> restrictor;
	    stack< Frame > state;

	    bool IsAtEndOfCollection() const;
	    void BypassInvalid();
	    void Push( TreePtr<Node> n );
	    void Pop();
	    void PoppingIncrement();
	    bool Done() const;
	    ContainerInterface::iterator GetIterator() const;
	    TreePtr<Node> Get() const;
	    void Set( TreePtr<Node> n );

	public:
	    iterator( TreePtr<Node> root=TreePtr<Node>(), // NULL root gets us a walk stuck on Done()
	              TreePtr<Node> restrictor=TreePtr<Node>() );
	    iterator( const iterator & other );
	    int Depth() const;
	    operator string() const;
	    void AdvanceOver();
	    void AdvanceInto();

	    typedef forward_iterator_tag iterator_category;
		typedef TreePtrInterface value_type;
		typedef int difference_type;
		typedef const value_type *pointer;
		typedef const value_type &reference;

		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const
		{
			shared_ptr<iterator> ni( new iterator(*this) );
			return ni;
		}

		virtual iterator &operator++()
		{
			AdvanceInto();
			return *this;
		}

		virtual reference operator*() const
		{
			return *GetIterator();
		}

		virtual pointer operator->() const
		{
			return &*GetIterator();
		}

		virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const
		{
			const iterator *pi = dynamic_cast<const iterator *>(&ib);
			ASSERT(pi)("Comparing walking iterator with something else ")(ib);
			if( pi->Done() || Done() )
				return pi->Done() && Done();
			return pi->Get() == Get();
		}

		virtual void Overwrite( pointer v ) const
		{
			GetIterator().Overwrite( v );
		}

		virtual const bool IsOrdered() const
		{
			return true; // walk walks tree in order generally
		}
	};

	Walk( TreePtr<Node> r,
			       TreePtr<Node> res = TreePtr<Node>() ) :
		root(r),
		restrictor(res),
		my_begin( iterator( root, restrictor ) ),
		my_end( iterator( TreePtr<Node>(), restrictor ) )
    {
    }
	TreePtr<Node> root;
    TreePtr<Node> restrictor;
    iterator my_begin, my_end;
    virtual const iterator &begin()
    {
    	my_begin = iterator( root, restrictor );
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end = iterator( TreePtr<Node>(), restrictor );
    	return my_end;
    }
    virtual void erase( ContainerInterface::iterator it ) { ASSERTFAIL("Cannot modify through walking container"); }
    virtual void clear() { ASSERTFAIL("Cannot modify through walking container"); }
};



#endif
