
#include "tree/tree.hpp"
#include "walk.hpp"
#include "search_replace.hpp"


////////////////////////// Flatten //////////////////////////

bool Flatten::iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}

void Flatten::iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
}

void Flatten::iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(GetCurrentMember()) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}

Flatten::iterator::iterator( TreePtr<Node> r ) :
	root( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
    NormaliseNewMember();
}

Flatten::iterator::iterator() :
	empty( true )
{
}

Flatten::iterator::iterator( const Flatten::iterator & other ) :
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
    root( other.root ),
	empty( other.empty )
{
}

Flatten::iterator::operator string() const
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

shared_ptr<ContainerInterface::iterator_interface> Flatten::iterator::Clone() const
{
	shared_ptr<iterator> ni( new iterator(*this) );
	return ni;
}

Flatten::iterator &Flatten::iterator::operator++()
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

Flatten::iterator::reference Flatten::iterator::operator*() const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        return *ptr;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

Flatten::iterator::pointer Flatten::iterator::operator->() const
{
	return &operator*();
}

bool Flatten::iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const iterator *pi = dynamic_cast<const iterator *>(&ib);
	ASSERT(pi)("Comparing traversing iterator with something else ")(ib);
	if( pi->IsAtEnd() || IsAtEnd() )
		return pi->IsAtEnd() && IsAtEnd();
	return **pi == **this; // TODO do not like - too many derefs
}

void Flatten::iterator::Overwrite( Flatten::iterator::pointer v ) const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        *ptr = *v;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

const bool Flatten::iterator::IsOrdered() const
{
	return true; // traverse walks tree in order generally
}

Flatten::Flatten( TreePtr<Node> r ) :
	root(r)
{
}

const Flatten::iterator &Flatten::begin()
{
	my_begin = iterator( root );
	return my_begin;
}

const Flatten::iterator &Flatten::end()
{
	my_end = iterator();
	return my_end;
}


////////////////////////// Expand //////////////////////////

bool Expand::iterator::IsAtEndOfChildren() const
{
	ASSERT( !done );

	if( state.empty() )
		return false;

    const Flatten::iterator &ti = state.top();

    return ti == Flatten::iterator();
}

void Expand::iterator::BypassEndOfChildren()
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

void Expand::iterator::Push( TreePtr<Node> n )
{ 
    state.push( Flatten(n).begin() );
}        

Expand::iterator::iterator( TreePtr<Node> &r,
	                   	  TreePtr<Node> res,
	                   	  const CompareReplace *rc,
	                   	  CouplingKeys *k ) :
    root( new TreePtr<Node>(r) ),
    restrictor( res ),
    restriction_comparison( rc ),
    keys( k ),
    done( false )
{
}

Expand::iterator::iterator() :
	restriction_comparison( NULL ),
    done( true )
{
}        

Expand::iterator::iterator( const Expand::iterator & other ) :
	root( other.root ),
	restrictor( other.restrictor ),
	restriction_comparison( other.restriction_comparison ),
	keys( other.keys ),
	state( other.state ),
	done( other.done )
{
}

Expand::iterator::operator string() const
{
    string s;
    stack< Flatten::iterator > ps = state; // std::stack doesn't have [] so copy the whole thing and go backwards
    while( !ps.empty() )
    {
        s = string(ps.top()) + string(" ") + s;
        ps.pop();
    }
    return s;
}

void Expand::iterator::AdvanceInto()
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

void Expand::iterator::AdvanceOver()
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

shared_ptr<ContainerInterface::iterator_interface> Expand::iterator::Clone() const
{
	shared_ptr<iterator> ni( new iterator(*this) );
	return ni;
}

Expand::iterator &Expand::iterator::operator++()
{
	AdvanceInto();
	return *this;
}

Expand::iterator::reference Expand::iterator::operator*() const
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

Expand::iterator::pointer Expand::iterator::operator->() const
{
	return &operator*();
}

bool Expand::iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const iterator *pi = dynamic_cast<const iterator *>(&ib);
	ASSERT(pi)("Comparing walking iterator with something else ")(ib);
	if( pi->done || done )
		return pi->done && done;
	return **pi == **this;
}

void Expand::iterator::Overwrite( Expand::iterator::pointer v ) const
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

const bool Expand::iterator::IsOrdered() const
{
	return true; // walk walks tree in order generally
}

Expand::Expand( TreePtr<Node> r,
		    TreePtr<Node> res,
		    const CompareReplace *rsr,
		    CouplingKeys *k ) :
	root(r),
	restrictor( res ),
	restriction_comparison( rsr ),
	keys( k ),
	my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
	my_end( iterator() )
{
}

const Expand::iterator &Expand::begin()
{
	my_begin = iterator( root, restrictor, restriction_comparison, keys );
	return my_begin;
}

const Expand::iterator &Expand::end()
{
	my_end = iterator();
	return my_end;
}

////////////////////////// ParentTraverse //////////////////////////

ParentTraverse::iterator::iterator()
{
}        

ParentTraverse::iterator::iterator( const ParentTraverse::iterator & other ) :
    Expand::iterator( other )
{
}

ParentTraverse::iterator::iterator( TreePtr<Node> &root,
                           TreePtr<Node> restrictor,
                           const CompareReplace *rc,
                           CouplingKeys *k ) :
    Expand::iterator( root, restrictor, rc, k )
{
}
 
ParentTraverse::ParentTraverse( TreePtr<Node> r,
              TreePtr<Node> res,
              const CompareReplace *rsr,
              CouplingKeys *k ) :
    root(r),
    restrictor( res ),
    restriction_comparison( rsr ),
    keys( k ),
    my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
    my_end( iterator() )
{
}

const ParentTraverse::iterator &ParentTraverse::begin()
{
    my_begin = iterator( root, restrictor, restriction_comparison, keys );
    return my_begin;
}

const ParentTraverse::iterator &ParentTraverse::end()
{
    my_end = iterator();
    return my_end;
}

void ParentTraverse::iterator::AdvanceInto()
{
    TreePtr<Node> element = **this; // look at current node
    if( element && seen.IsExist( element ) )
        Expand::iterator::AdvanceOver(); // do not recurse
    else
        Expand::iterator::AdvanceInto(); // recurse if walk would
        
    if( element )
        seen.insert( element );    
}    
    
////////////////////////// Traverse //////////////////////////

Traverse::iterator::iterator()
{
}        

Traverse::iterator::iterator( const Traverse::iterator & other ) :
    Expand::iterator( other )
{
    TRACE("Traverse copy\n");
    Filter();
}

Traverse::iterator::iterator( TreePtr<Node> &root,
                           TreePtr<Node> restrictor,
                           const CompareReplace *rc,
                           CouplingKeys *k ) :
    Expand::iterator( root, restrictor, rc, k )
{
    Filter();
}
 
Traverse::Traverse( TreePtr<Node> r,
              TreePtr<Node> res,
              const CompareReplace *rsr,
              CouplingKeys *k ) :
    root(r),
    restrictor( res ),
    restriction_comparison( rsr ),
    keys( k ),
    my_begin( iterator( root, restrictor, restriction_comparison, k ) ),
    my_end( iterator() )
{
}

const Traverse::iterator &Traverse::begin()
{
    my_begin = iterator( root, restrictor, restriction_comparison, keys );
    return my_begin;
}

const Traverse::iterator &Traverse::end()
{
    my_end = iterator();
    return my_end;
}

Traverse::iterator &Traverse::iterator::operator++()
{
    Expand::iterator::operator++(); 
    Filter();
    return *this;
}

void Traverse::iterator::Filter()
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
                TRACE("Traverse inserting node ")(*element)(" at %p size now %d\n", element.get(), seen.size());        
            } 
        }        
        
        if( ok )
        {
            TRACE("Traverse returning node ")(*element)(" at %p size now %d\n", element.get(), seen.size());
            return;
        }
        
        if( element )
            TRACE("Traverse skipping node ")(*element)(" at %p\n", element.get());
        Expand::iterator::operator++(); // increment once
    }    
}
