#ifndef WALK_HPP
#define WALK_HPP

#include "tree/generics.hpp"

class Walk
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

public:
    Walk( TreePtr<Node> root=TreePtr<Node>(), // NULL root gets us a walk stuck on Done()
          TreePtr<Node> restrictor=TreePtr<Node>() );
    Walk( const Walk & other );
    bool Done() const;
    int Depth() const;
    ContainerInterface::iterator GetIterator() const;
    TreePtr<Node> Get() const;
    void Set( TreePtr<Node> n );
    operator string() const;
    void AdvanceOver(); 
    void AdvanceInto();
};


class WalkContainer : public ContainerInterface
{
public:

	struct iterator : public ContainerInterface::iterator_interface,
							 Walk
	{
		iterator( TreePtr<Node> root,
				  TreePtr<Node> restrictor ) :
			Walk( root, restrictor )
		{
		}

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

		virtual const TreePtrInterface &operator*() const
		{
			return *GetIterator();
		}

		const virtual TreePtrInterface *operator->() const
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

		virtual void Overwrite( const TreePtrInterface *v ) const
		{
			GetIterator().Overwrite( v );
		}

		virtual const bool IsOrdered() const
		{
			return true; // walk walks tree in order generally
		}
	};

	WalkContainer( TreePtr<Node> r,
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
