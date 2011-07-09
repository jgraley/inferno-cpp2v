
#include "walk.hpp"
#include "transformation.hpp"


////////////////////////// Flatten //////////////////////////

bool Flatten_iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}

void Flatten_iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
}

void Flatten_iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(GetCurrentMember()) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}

Flatten_iterator::Flatten_iterator( TreePtr<Node> r ) :
	root( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
    NormaliseNewMember();
}

Flatten_iterator::Flatten_iterator() :
	empty( true )
{
}

Flatten_iterator::Flatten_iterator( const Flatten_iterator & other ) :
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
    root( other.root ),
	empty( other.empty )
{
}

Flatten_iterator::operator string() const
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

shared_ptr<ContainerInterface::iterator_interface> Flatten_iterator::Clone() const
{
	shared_ptr<Flatten_iterator> ni( new Flatten_iterator(*this) );
	return ni;
}

Flatten_iterator &Flatten_iterator::operator++()
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

Flatten_iterator::reference Flatten_iterator::operator*() const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        return *ptr;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

Flatten_iterator::pointer Flatten_iterator::operator->() const
{
	return &operator*();
}

bool Flatten_iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const Flatten_iterator *pi = dynamic_cast<const Flatten_iterator *>(&ib);
	ASSERT(pi)("Comparing traversing iterator with something else ")(ib);
	if( pi->IsAtEnd() || IsAtEnd() )
		return pi->IsAtEnd() && IsAtEnd();
	return *pi == *this; 
}

void Flatten_iterator::Overwrite( Flatten_iterator::pointer v ) const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        *ptr = *v;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

const bool Flatten_iterator::IsOrdered() const
{
	return true; // traverse walks tree in order generally
}


////////////////////////// Expand //////////////////////////

bool Expand_iterator::IsAtEndOfChildren() const
{
	ASSERT( !done );

	if( state.empty() )
		return false;

    return state.top().iterator == state.top().container->end();
}

void Expand_iterator::BypassEndOfChildren()
{
	ASSERT( !done );
	while( IsAtEndOfChildren() )
	{
		state.pop();

	    done = state.empty();
	    if( done )
	    	break;

	    ++state.top().iterator;
	}
}

shared_ptr<ContainerInterface> Expand_iterator::GetChildContainer( TreePtr<Node> n ) const
{ 
    return shared_ptr<ContainerInterface>( new Flatten(n) );
}        

void Expand_iterator::Push( TreePtr<Node> n )
{ 
    StateEntry ns;
    ns.container = GetChildContainer( n );
    ns.iterator = ns.container->begin();
    state.push( ns );
}        

Expand_iterator::Expand_iterator( TreePtr<Node> &r,
	                    	      Filter *of,
                                  Filter *rf ) :
    root( new TreePtr<Node>(r) ),
    out_filter( of ),
    recurse_filter( rf ),
    done( false )
{
    DoNodeFilter();
}

Expand_iterator::Expand_iterator() :
    out_filter( NULL ),
    recurse_filter( NULL ),
    done( true )
{
}        

Expand_iterator::Expand_iterator( const Expand_iterator & other ) :
	root( other.root ),
    out_filter( other.out_filter ),
    recurse_filter( other.recurse_filter ),
	state( other.state ),
	done( other.done )
{
}

Expand_iterator::operator string() const
{
    string s;
    stack< StateEntry > ps = state; // std::stack doesn't have [] so copy the whole thing and go backwards
    while( !ps.empty() )
    {
        s = string(*(ps.top().iterator)) + string(" ") + s;
        ps.pop();
    }
    return s;
}

void Expand_iterator::AdvanceInto()
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

void Expand_iterator::AdvanceOver()
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
	    ++state.top().iterator;
  	    BypassEndOfChildren();
	}
}

shared_ptr<ContainerInterface::iterator_interface> Expand_iterator::Clone() const
{
	shared_ptr<Expand_iterator> ni( new Expand_iterator(*this) );
	return ni;
}

Expand_iterator &Expand_iterator::operator++()
{
	AdvanceInto();
    DoNodeFilter();
	return *this;
}

Expand_iterator::reference Expand_iterator::operator*() const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

	if( state.empty() )
	{
		return *root;
	}
	else
	{
        return *(state.top().iterator);
	}
}

Expand_iterator::pointer Expand_iterator::operator->() const
{
	return &operator*();
}

bool Expand_iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const Expand_iterator *pi = dynamic_cast<const Expand_iterator *>(&ib);
	ASSERT(pi)("Comparing walking iterator with something else ")(ib);
	if( pi->done || done )
		return pi->done && done;
	return **pi == **this;
}

void Expand_iterator::Overwrite( Expand_iterator::pointer v ) const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

    if( state.empty() )
    {
    	*root = *v;
    }
    else
    {
        state.top().iterator.Overwrite( v );
    }
}

const bool Expand_iterator::IsOrdered() const
{
	return true; // walk walks tree in order generally
}

void Expand_iterator::DoNodeFilter()
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

ParentTraverse_iterator::ParentTraverse_iterator() :
    unique_filter( new UniqueFilter )
{
}        

ParentTraverse_iterator::~ParentTraverse_iterator() 
{
    delete unique_filter;
}        

ParentTraverse_iterator::ParentTraverse_iterator( const ParentTraverse_iterator & other ) :
    Expand_iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    recurse_filter = unique_filter; 
}

ParentTraverse_iterator::ParentTraverse_iterator &ParentTraverse_iterator::operator=( const ParentTraverse_iterator & other )
{
    Expand_iterator::operator=( other ),
    *unique_filter = *(other.unique_filter);
    recurse_filter = unique_filter; 
    return *this;
}

ParentTraverse_iterator::ParentTraverse_iterator( TreePtr<Node> &root ) :
    Expand_iterator( root, NULL, unique_filter = new UniqueFilter )
{
}

////////////////////////// Traverse //////////////////////////

Traverse_iterator::Traverse_iterator() :
    unique_filter( new UniqueFilter )
{
}        

Traverse_iterator::~Traverse_iterator()
{
    delete unique_filter;
}        

Traverse_iterator::Traverse_iterator( const Traverse_iterator & other ) :
    Expand_iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    out_filter = unique_filter; 
}

Traverse_iterator::Traverse_iterator &Traverse_iterator::operator=( const Traverse_iterator &other )
{
    Expand_iterator::operator=( other );
    *unique_filter = *(other.unique_filter);
    out_filter = unique_filter; 
    return *this;
}

Traverse_iterator::Traverse_iterator( TreePtr<Node> &root ) :
    Expand_iterator( root, unique_filter = new UniqueFilter, NULL )
{
}

