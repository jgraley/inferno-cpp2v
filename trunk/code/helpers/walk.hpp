#ifndef WALK_HPP
#define WALK_HPP

#include "tree/tree.hpp"

class Walk
{
    struct Frame
    {
        vector< GenericContainer::iterator > children;
        int index;
    };
    
    shared_ptr< SharedPtr<Node> > root;
    SharedPtr<Node> restrictor;
    stack< Frame > state;
    
    bool IsAtEndOfCollection() const;
    void BypassInvalid();
    void Push( shared_ptr<Node> n );
    void Pop();
    void PoppingIncrement();

public:
    Walk( shared_ptr<Node> root=shared_ptr<Node>(), // NULL root gets us a walk stuck on Done()
          shared_ptr<Node> restrictor=shared_ptr<Node>() );
    Walk( const Walk & other );
    bool Done() const;
    int Depth() const;
    GenericContainer::iterator GetIterator() const;
    shared_ptr<Node> Get() const;
    operator string() const;
    void AdvanceOver(); 
    void AdvanceInto();
};


/*
    Example usage:
    
    Flattener<Record> w(program);
    FOREACH( shared_ptr<Record> r, w )
        printf("%s\n", typeid(*r).name() );
*/

template< class FIND >
class Flattener : public Sequence< FIND >
{
public:
    Flattener( shared_ptr<Node> root ) 
    {
        TRACE("Flattener\n");
        Walk w( root );
        while(!w.Done())
        {
            shared_ptr<Node> x = w.Get();
            if( shared_ptr<FIND> f = dynamic_pointer_cast< FIND >( x ) )
                Sequence< FIND >::push_back( f );
            w.AdvanceInto(); 
        }    
    }
};


struct WalkingIterator : public STLContainerBase<Itemiser::Element, GenericSharedPtr>::iterator_base,
                         Walk
{
	WalkingIterator( shared_ptr<Node> root = shared_ptr<Node>(), // NULL root gets us an "end" iterator
			         shared_ptr<Node> restrictor = shared_ptr<Node>() ) :
		Walk( root, restrictor )
    {
    }

	virtual shared_ptr<STLContainerBase<Itemiser::Element, GenericSharedPtr>::iterator_base> Clone() const
	{
		shared_ptr<WalkingIterator> ni( new WalkingIterator(*this) );
		return ni;
	}

	virtual WalkingIterator &operator++()
	{
		AdvanceInto();
		return *this;
	}

	virtual const GenericSharedPtr &operator*() const
	{
	    return *GetIterator();
	}

	const virtual GenericSharedPtr *operator->() const
	{
		return &*GetIterator();
	}

	virtual bool operator==( const STLContainerBase<Itemiser::Element, GenericSharedPtr>::iterator_base &ib ) const
	{
		const WalkingIterator *pi = dynamic_cast<const WalkingIterator *>(&ib);
		ASSERT(pi)("Comparing walking iterator with something else %s", typeid(ib).name());
		if( pi->Done() || Done() )
			return pi->Done() && Done();
		//ASSERTFAIL("Comparison must be with end");
		return pi->Get() == Get();
	}

	virtual void Overwrite( const GenericSharedPtr *v ) const
	{
		GetIterator().Overwrite( v );
	}

	virtual const bool IsOrdered() const
	{
		return true; // walk walks tree in order generally
	}
};



#endif
