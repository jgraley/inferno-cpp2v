#ifndef WALK_HPP
#define WALK_HPP

#include "tree/generics.hpp"


class Walk : public ContainerInterface
{
public:
	class iterator : public ContainerInterface::iterator_interface
	{
	public:
	    typedef forward_iterator_tag iterator_category;
		typedef TreePtrInterface value_type;
		typedef int difference_type;
		typedef const value_type *pointer;
		typedef const value_type &reference;

		iterator( const iterator & other );
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
		virtual iterator &operator++();
		virtual reference operator*() const;
		virtual pointer operator->() const;
		virtual bool operator==( const ContainerInterface::iterator_interface &ib ) const;
		virtual void Overwrite( pointer v ) const;
        virtual const bool IsOrdered() const;
	    int Depth() const;
	    operator string() const;
	    void AdvanceOver();
	    void AdvanceInto();
	private:
        struct Frame
	    {
	        vector< ContainerInterface::iterator > children;
	        int index;
	    };

	    iterator( TreePtr<Node> root, TreePtr<Node> restrictor );
	    bool IsAtEndOfCollection() const;
	    void BypassInvalid();
	    void Push( TreePtr<Node> n );
	    void Pop();
	    void PoppingIncrement();
	    bool Done() const;
	    ContainerInterface::iterator GetIterator() const;
	    TreePtr<Node> Get() const;
	    void Set( TreePtr<Node> n );

	    shared_ptr< TreePtr<Node> > root;
	    TreePtr<Node> restrictor;
	    stack< Frame > state;

	    friend class Walk;
    };

	Walk( TreePtr<Node> r,
		  TreePtr<Node> res = TreePtr<Node>() );
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
