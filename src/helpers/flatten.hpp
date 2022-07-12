#ifndef FLATTEN_HPP
#define FLATTEN_HPP

#include "node/node.hpp"
#include "transformation.hpp"
#include <deque>
#include <list>

/// Iterator for FlattenNode
class FlattenNode_iterator : public ContainerInterface::iterator
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
	virtual bool operator==( const ContainerInterface::iterator_interface &ciii_o ) const;
	virtual bool operator!=( const ContainerInterface::iterator_interface &ciii_o ) const { return !operator==(ciii_o); }
	//virtual bool operator<( const FlattenNode_iterator &o ) const;
	virtual void Overwrite( pointer v ) const;
    virtual const bool IsOrdered() const;
    // Some additional operations specific to walk iterators
    string GetName() const;
    FlattenNode_iterator(); // makes "end" iterator
    FlattenNode_iterator( TreePtr<Node> root );
    FlattenNode_iterator( const Node *root );
    
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
    const Node *root;
    TreePtr<Node> ref_holder;
    bool empty; // TODO use nullptr root for empty
};

/*! Stated out traversal across a node's children. UniqueWalks the members and elements of containers
    but does not follow any TreePtr. Basically an itemise that expands containers. */
typedef ContainerFromIterator< FlattenNode_iterator, TreePtr<Node> > FlattenNode;

#endif

