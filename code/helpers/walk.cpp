
#include "tree/tree.hpp"
#include "walk.hpp"

bool Traverse::iterator::IsAtEndOfChildren() const
{
	ASSERT( index <= children.size() );
    return index == children.size();
}

void Traverse::iterator::BypassEndOfContainer()
{
	while( cit == con->end() )
	{
		++mit;
		if( mit == members.end() )
			return;
		if( con = dynamic_cast<ContainerInterface *>(*mit) )
			cit = con->begin();
		else
			break;
	}

}

Traverse::iterator::iterator( TreePtr<Node> r )
{
    members = r->Itemise();
    index = 0;

    mit = members.begin();

	if( mit == members.end() )
		return;
    if( con = dynamic_cast<ContainerInterface *>(*mit) )
    {
    	cit = con->begin();
    	BypassEndOfContainer();
    	if( mit == members.end() )
    		return;
    }


    while(1)
    {
    	if( dynamic_cast<ContainerInterface *>(*mit) )
        {
            children.push_back( cit );
        }
        else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(*mit) )
        {
            children.push_back( PointIterator(ptr) ); // TODO if the itemise was a member or otherwise persistent, I wouldn't need to use PointIterator
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
        }


        if( con = dynamic_cast<ContainerInterface *>(*mit) )
        {

        	++cit;

        	BypassEndOfContainer();
        	if( mit == members.end() )
        		return;
        }
        else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(*mit) )
        {
            ++mit;
            if( mit == members.end() )
            	return;
            if( con = dynamic_cast<ContainerInterface *>(*mit) )
            {
            	cit = con->begin();
            	BypassEndOfContainer();
            	if( mit == members.end() )
            		return;
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
        }
    }
}

Traverse::iterator::iterator()
{
	index = 0; // children.size() will also be 0, so this is at the end
}

Traverse::iterator::iterator( const Traverse::iterator & other ) :
	children( other.children ),
	index( other.index )
{
}

Traverse::iterator::operator string() const
{
    if (IsAtEndOfChildren())
    	return string("end");

	// member/element number
    string s;
	s = SSPrintf("%d:", index); // going backwards so prepend

    // node type
	ContainerInterface::iterator child = children[index];
	if( *child )
		s += TypeInfo(*(child->get())).name();
	else
		s += string("NULL");

    return s;
}

void Traverse::iterator::Advance()
{
	ASSERT( !IsAtEndOfChildren() );
    index++;
}

shared_ptr<ContainerInterface::iterator_interface> Traverse::iterator::Clone() const
{
	shared_ptr<iterator> ni( new iterator(*this) );
	return ni;
}

Traverse::iterator &Traverse::iterator::operator++()
{
	Advance();
	return *this;
}

Traverse::iterator::reference Traverse::iterator::operator*() const
{
    ASSERT( !IsAtEndOfChildren() );
    return *(children[index]);
}

Traverse::iterator::pointer Traverse::iterator::operator->() const
{
	return &operator*();
}

bool Traverse::iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const iterator *pi = dynamic_cast<const iterator *>(&ib);
	ASSERT(pi)("Comparing traversing iterator with something else ")(ib);
	if( pi->IsAtEndOfChildren() || IsAtEndOfChildren() )
		return pi->IsAtEndOfChildren() && IsAtEndOfChildren();
	return **pi == **this; // TODO do not like - too many derefs
}

void Traverse::iterator::Overwrite( Traverse::iterator::pointer v ) const
{
    ASSERT( !IsAtEndOfChildren() );
    children[index].Overwrite( v );
}

const bool Traverse::iterator::IsOrdered() const
{
	return true; // traverse walks tree in order generally
}

Traverse::Traverse( TreePtr<Node> r ) :
	root(r)
{
}

const Traverse::iterator &Traverse::begin()
{
	my_begin = iterator( root );
	return my_begin;
}

const Traverse::iterator &Traverse::end()
{
	my_end = iterator();
	return my_end;
}


















bool Walk::iterator::IsAtEndOfChildren() const
{
	ASSERT( !done );

	if( state.empty() )
		return false;

    const Traverse::iterator &ti = state.top();

    return ti == Traverse::iterator();
}

void Walk::iterator::BypassEndOfChildren()
{
	ASSERT( !done );
	while( IsAtEndOfChildren() )
	{
		state.pop();

	    done = state.empty();
	    if( done )
	    	break;

	    ++(state.top());
	}
}

void Walk::iterator::Push( TreePtr<Node> n )
{ 
    state.push( Traverse(n).begin() );
}        

Walk::iterator::iterator( TreePtr<Node> &r, TreePtr<Node> res ) :
    root( new TreePtr<Node>(r) ),
    restrictor( res ),
    done( false )
{
}

Walk::iterator::iterator() :
    done( true )
{
}        

Walk::iterator::iterator( const Walk::iterator & other ) :
	root( other.root ),
	restrictor( other.restrictor ),
	state( other.state ),
	done( other.done )
{
}

Walk::iterator::operator string() const
{
    string s;
    stack< Traverse::iterator > ps = state; // std::stack doesn't have [] so copy the whole thing and go backwards
    while( !ps.empty() )
    {
        s = string(ps.top()) + string(" ") + s;
        ps.pop();
    }
    return s;
}

void Walk::iterator::AdvanceInto()
{
	ASSERT( !done );
	ASSERT( !IsAtEndOfChildren() );
	TreePtr<Node> element = **this; // look at current node
    if( element &&                                                    // must be non-NULL
    	(!restrictor || restrictor->IsLocalMatch(element.get()) ) ) // and pass the restriction
    {
    	// Step into
        Push( **this );
        BypassEndOfChildren();
    }
    else
    {
    	// If we can't step into then step over
    	AdvanceOver();
    }
}

void Walk::iterator::AdvanceOver()
{
	ASSERT( !done );
	ASSERT( !IsAtEndOfChildren() );

	if( state.empty() )
	{
		// At top level there's only one element, so finish the walk
		done = true;
	}
	else
	{
		// otherwise, propagate
	    ++(state.top());
  	    BypassEndOfChildren();
	}
}

shared_ptr<ContainerInterface::iterator_interface> Walk::iterator::Clone() const
{
	shared_ptr<iterator> ni( new iterator(*this) );
	return ni;
}

Walk::iterator &Walk::iterator::operator++()
{
	AdvanceInto();
	return *this;
}

Walk::iterator::reference Walk::iterator::operator*() const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

	if( state.empty() )
	{
		return *root;
	}
	else
	{
        return *(state.top());
	}
}

Walk::iterator::pointer Walk::iterator::operator->() const
{
	return &operator*();
}

bool Walk::iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const iterator *pi = dynamic_cast<const iterator *>(&ib);
	ASSERT(pi)("Comparing walking iterator with something else ")(ib);
	if( pi->done || done )
		return pi->done && done;
	return **pi == **this;
}

void Walk::iterator::Overwrite( Walk::iterator::pointer v ) const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

    if( state.empty() )
    {
    	*root = *v;
    }
    else
    {
        state.top().Overwrite( v );
    }
}

const bool Walk::iterator::IsOrdered() const
{
	return true; // walk walks tree in order generally
}

Walk::Walk( TreePtr<Node> r,
		    TreePtr<Node> res ) :
	root(r),
	restrictor(res),
	my_begin( iterator( root, restrictor ) ),
	my_end( iterator() )
{
}

const Walk::iterator &Walk::begin()
{
	my_begin = iterator( root, restrictor );
	return my_begin;
}

const Walk::iterator &Walk::end()
{
	my_end = iterator();
	return my_end;
}
