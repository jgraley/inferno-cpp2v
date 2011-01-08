
#include "tree/tree.hpp"
#include "walk.hpp"
#include "search_replace.hpp"


////////////////////////// Traverse //////////////////////////

bool Traverse::iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}

void Traverse::iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
}

void Traverse::iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(GetCurrentMember()) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}

Traverse::iterator::iterator( TreePtr<Node> r ) :
	root( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
    NormaliseNewMember();
}

Traverse::iterator::iterator() :
	empty( true )
{
}

Traverse::iterator::iterator( const Traverse::iterator & other ) :
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
    root( other.root ),
	empty( other.empty )
{
}

Traverse::iterator::operator string() const
{
    if (IsAtEnd())
    	return string("end");
    else if( dynamic_cast<CollectionInterface *>(GetCurrentMember()) )
        return string("{") + TypeInfo(cit->get()).name() + string("}");
	else if( dynamic_cast<SequenceInterface *>(GetCurrentMember()) )
        return string("[") + TypeInfo(cit->get()).name() + string("]");
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
       	return TypeInfo(*(ptr->get())).name();
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
    if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
    {
    	++cit;
    	BypassEndOfContainer();
    }
    else if( dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
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
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
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
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
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


////////////////////////// Walk //////////////////////////

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

Walk::iterator::iterator( TreePtr<Node> &r,
	                   	  TreePtr<Node> res,
	                   	  const RootedSearchReplace *rc,
	                   	  CouplingKeys *k ) :
    root( new TreePtr<Node>(r) ),
    restrictor( res ),
    restriction_comparison( rc ),
    keys( k ),
    done( false )
{
}

Walk::iterator::iterator() :
	restriction_comparison( NULL ),
    done( true )
{
}        

Walk::iterator::iterator( const Walk::iterator & other ) :
	root( other.root ),
	restrictor( other.restrictor ),
	restriction_comparison( other.restriction_comparison ),
	keys( other.keys ),
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
	bool recurse = false;
	if( element ) // must be non-NULL
	{
		recurse = true;
		if( restrictor ) // is there a restriction?
		{
			ASSERT( restriction_comparison );
			if( !restriction_comparison->Compare( element, restrictor, keys, false ) ) // must pass the restriction
				recurse = false;
		}
	}

    if( recurse )
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
		    TreePtr<Node> res,
		    const RootedSearchReplace *rsr,
		    CouplingKeys *k ) :
	root(r),
	restrictor( res ),
	restriction_comparison( rsr ),
	keys( k ),
	my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
	my_end( iterator() )
{
}

const Walk::iterator &Walk::begin()
{
	my_begin = iterator( root, restrictor, restriction_comparison, keys );
	return my_begin;
}

const Walk::iterator &Walk::end()
{
	my_end = iterator();
	return my_end;
}

////////////////////////// Sweep //////////////////////////

Sweep::iterator::iterator()
{
}        

Sweep::iterator::iterator( const Sweep::iterator & other ) :
    Walk::iterator( other )
{
}

Sweep::iterator::iterator( TreePtr<Node> &root,
                           TreePtr<Node> restrictor,
                           const RootedSearchReplace *rc,
                           CouplingKeys *k ) :
    Walk::iterator( root, restrictor, rc, k )
{
}
 
Sweep::Sweep( TreePtr<Node> r,
              TreePtr<Node> res,
              const RootedSearchReplace *rsr,
              CouplingKeys *k ) :
    root(r),
    restrictor( res ),
    restriction_comparison( rsr ),
    keys( k ),
    my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
    my_end( iterator() )
{
}

const Sweep::iterator &Sweep::begin()
{
    my_begin = iterator( root, restrictor, restriction_comparison, keys );
    return my_begin;
}

const Sweep::iterator &Sweep::end()
{
    my_end = iterator();
    return my_end;
}

void Sweep::iterator::AdvanceInto()
{
    TreePtr<Node> element = **this; // look at current node
    if( element && seen.IsExist( element ) )
        Walk::iterator::AdvanceOver(); // do not recurse
    else
        Walk::iterator::AdvanceInto(); // recurse if walk would
        
    if( element )
        seen.insert( element );    
}    
    
////////////////////////// Stroll //////////////////////////

Stroll::iterator::iterator()
{
}        

Stroll::iterator::iterator( const Stroll::iterator & other ) :
    Walk::iterator( other )
{
    TRACE("Stroll copy\n");
    Filter();
}

Stroll::iterator::iterator( TreePtr<Node> &root,
                           TreePtr<Node> restrictor,
                           const RootedSearchReplace *rc,
                           CouplingKeys *k ) :
    Walk::iterator( root, restrictor, rc, k )
{
    Filter();
}
 
Stroll::Stroll( TreePtr<Node> r,
              TreePtr<Node> res,
              const RootedSearchReplace *rsr,
              CouplingKeys *k ) :
    root(r),
    restrictor( res ),
    restriction_comparison( rsr ),
    keys( k ),
    my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
    my_end( iterator() )
{
}

const Stroll::iterator &Stroll::begin()
{
    my_begin = iterator( root, restrictor, restriction_comparison, keys );
    return my_begin;
}

const Stroll::iterator &Stroll::end()
{
    my_end = iterator();
    return my_end;
}

Stroll::iterator &Stroll::iterator::operator++()
{
    Walk::iterator::operator++(); 
    Filter();
    return *this;
}

void Stroll::iterator::Filter()
{
    while(!done)
    {
        TreePtr<Node> element = **this; // look at current node
        
        bool ok;
        { // this block is the "client filter rule", rest of fn is filtering infrastructure
            ok = element && !seen.IsExist( element );
            if( ok )
            {
                seen.insert( element );    
                TRACE("Stroll inserting node ")(*element)(" at %p size now %d\n", element.get(), seen.size());        
            } 
        }        
        
        if( ok )
        {
            TRACE("Stroll returning node ")(*element)(" at %p size now %d\n", element.get(), seen.size());
            return;
        }
        
        if( element )
            TRACE("Stroll skipping node ")(*element)(" at %p\n", element.get());
        Walk::iterator::operator++(); // increment once
    }    
}
