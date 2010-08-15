
#include "tree/tree.hpp"
#include "walk.hpp"

bool Traverse::iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}

void Traverse::iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(*mit) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
}

void Traverse::iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(*mit) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}

Traverse::iterator::iterator( TreePtr<Node> r ) :
	empty( false )
{
    members = shared_ptr< vector< Itemiser::Element * > >( new vector< Itemiser::Element * >( r->Itemise() ) );

    mit = members->begin();
    m_end = members->end();
    NormaliseNewMember();
}

Traverse::iterator::iterator() :
	empty( true )
{
}

Traverse::iterator::iterator( const Traverse::iterator & other ) :
    members( other.members ),
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
	empty( other.empty )
{
}

Traverse::iterator::operator string() const
{
    if (IsAtEnd())
    	return string("end");
    else if( dynamic_cast<CollectionInterface *>(*mit) )
        return string("{") + TypeInfo(*(cit->get())).name() + "}";
	else if( dynamic_cast<SequenceInterface *>(*mit) )
        return string("[") + TypeInfo(*(cit->get())).name() + "]";
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(*mit) )
    {
        if( *ptr )
        	return TypeInfo(*(ptr->get())).name();
        else
        	return string("NULL");
    }
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

shared_ptr<ContainerInterface::iterator_interface> Traverse::iterator::Clone() const
{
	shared_ptr<iterator> ni( new iterator(*this) );
	return ni;
}

Traverse::iterator &Traverse::iterator::operator++()
{
	ASSERT( !IsAtEnd() );
    if( dynamic_cast<ContainerInterface *>(*mit) )
    {
    	++cit;
    	BypassEndOfContainer();
    }
    else if( dynamic_cast<TreePtrInterface *>(*mit) )
    {
        ++mit;
        NormaliseNewMember();
    }
    else
    {
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
    }
	return *this;
}

Traverse::iterator::reference Traverse::iterator::operator*() const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(*mit) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(*mit) )
        return *ptr;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

Traverse::iterator::pointer Traverse::iterator::operator->() const
{
	return &operator*();
}

bool Traverse::iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const iterator *pi = dynamic_cast<const iterator *>(&ib);
	ASSERT(pi)("Comparing traversing iterator with something else ")(ib);
	if( pi->IsAtEnd() || IsAtEnd() )
		return pi->IsAtEnd() && IsAtEnd();
	return **pi == **this; // TODO do not like - too many derefs
}

void Traverse::iterator::Overwrite( Traverse::iterator::pointer v ) const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(*mit) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(*mit) )
        *ptr = *v;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
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
