
#include "tree/tree.hpp"
#include "walk.hpp"
#include "transformation.hpp"


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
	return *pi == *this; 
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
	                   	    Filter *of,
                            Filter *rf ) :
    root( new TreePtr<Node>(r) ),
    out_filter( of ),
    recurse_filter( rf ),
    done( false )
{
    DoNodeFilter();
}

Expand::iterator::iterator() :
    out_filter( NULL ),
    recurse_filter( NULL ),
    done( true )
{
}        

Expand::iterator::iterator( const Expand::iterator & other ) :
	root( other.root ),
    out_filter( other.out_filter ),
    recurse_filter( other.recurse_filter ),
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
		if( recurse_filter ) // is there a filter on recursion?
		{
            //TRACE("Recurse filter @%p, this@%p, entering...\n", recurse_filter, this);
			bool ok = recurse_filter->IsMatch( element, element ); // must pass the restriction
			//TRACE("Recurse filter @%p, leaving...\n", recurse_filter);

			if( !ok )
				recurse = false;
		}
		//else
            //TRACE("No recurse filter\n", recurse_filter);
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
    DoNodeFilter();
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

void Expand::iterator::DoNodeFilter()
{
    while(!done)
    {
        TreePtr<Node> element = **this; // look at current node
        
        // TODO pass NULLs to filter, let it decide what to do
        bool ok = true;
        if( out_filter )
            if( !element || !out_filter->IsMatch( element) ) 
                ok = false;
         
        if( ok )
        {
            return;
        }
        
        AdvanceInto();
    }    
}
    
Expand::Expand( TreePtr<Node> r,
		        Filter *of,
                Filter *rf ) :
	root(r),
    out_filter( of ),
    recurse_filter( rf ),
	my_begin( iterator( root, out_filter, recurse_filter ) ),
	my_end( iterator() )
{
}

const Expand::iterator &Expand::begin()
{
	return my_begin;
}

const Expand::iterator &Expand::end()
{
	return my_end;
}

////////////////////////// UniqueFilter //////////////////////////

bool UniqueFilter::IsMatch( TreePtr<Node> context,
                            TreePtr<Node> root )
{
    ASSERT( root );
    (void)context;
    
    //TRACE("Got ")(*root)("\n");
    
    if( seen.IsExist( root ) )
        return false;
    
    seen.insert( root );
    return true;        
}

////////////////////////// ParentTraverse //////////////////////////

ParentTraverse::iterator::iterator() :
    unique_filter( new UniqueFilter )
{
}        

ParentTraverse::iterator::~iterator() 
{
    delete unique_filter;
}        

ParentTraverse::iterator::iterator( const ParentTraverse::iterator & other ) :
    Expand::iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    recurse_filter = unique_filter; 
}

ParentTraverse::iterator::iterator &ParentTraverse::iterator::operator=( const ParentTraverse::iterator & other )
{
    Expand::iterator::operator=( other ),
    *unique_filter = *(other.unique_filter);
    recurse_filter = unique_filter; 
    return *this;
}

ParentTraverse::iterator::iterator( TreePtr<Node> &root ) :
    Expand::iterator( root, NULL, unique_filter = new UniqueFilter )
{
}
 
ParentTraverse::ParentTraverse( TreePtr<Node> r ) :
    root(r),
    my_begin( iterator( root ) ),
    my_end( iterator() )
{
}

const ParentTraverse::iterator &ParentTraverse::begin()
{
    return my_begin;
}

const ParentTraverse::iterator &ParentTraverse::end()
{
    return my_end;
}
    
////////////////////////// Traverse //////////////////////////

Traverse::iterator::iterator() :
    unique_filter( new UniqueFilter )
{
}        

Traverse::iterator::~iterator()
{
    delete unique_filter;
}        

Traverse::iterator::iterator( const Traverse::iterator & other ) :
    Expand::iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    out_filter = unique_filter; 
}

Traverse::iterator::iterator &Traverse::iterator::operator=( const Traverse::iterator &other )
{
    Expand::iterator::operator=( other );
    *unique_filter = *(other.unique_filter);
    out_filter = unique_filter; 
    return *this;
}

Traverse::iterator::iterator( TreePtr<Node> &root ) :
    Expand::iterator( root, unique_filter = new UniqueFilter, NULL )
{
}
 
Traverse::Traverse( TreePtr<Node> r ) :
    root(r),
    my_begin( iterator( root ) ),
    my_end( iterator() )
{
}

const Traverse::iterator &Traverse::begin()
{
    return my_begin;
}

const Traverse::iterator &Traverse::end()
{
    return my_end;
}
