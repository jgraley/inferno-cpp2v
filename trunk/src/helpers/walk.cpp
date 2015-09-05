
#include "walk.hpp"
#include "transformation.hpp"

////////////////////////// FlattenNode //////////////////////////

bool FlattenNode_iterator::IsAtEnd() const
{
    return empty || mit == m_end;
}

void FlattenNode_iterator::NormaliseNewMember()
{
	if( !IsAtEnd() )
	{
		Itemiser::Element *ie = GetCurrentMember();
		if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(ie) )
		{
			cit = con->begin();
			c_end = con->end();
			BypassEndOfContainer();
		}
	}
}

void FlattenNode_iterator::BypassEndOfContainer()
{
	ASSERT( !IsAtEnd() && dynamic_cast<ContainerInterface *>(GetCurrentMember()) ); // this fn requires us to be on a container
	if( cit == c_end )
	{
		++mit;
		NormaliseNewMember();
	}
}

FlattenNode_iterator::FlattenNode_iterator( TreePtr<Node> r ) :
	root( r ),
	empty( false )
{
    //members = shared_ptr< vector< int > >( new vector< int >( root->BasicItemise() ) );

    mit = 0;
    m_end = root->ItemiseSize();
	//TRACE("Flattening node ")(*root)(" size %d\n", m_end);
    NormaliseNewMember();
}

FlattenNode_iterator::FlattenNode_iterator() :
	empty( true )
{
}

FlattenNode_iterator::FlattenNode_iterator( const FlattenNode_iterator & other ) :
    mit( other.mit ),
    m_end( other.m_end ),
    cit( other.cit ),
    c_end( other.c_end ),
    root( other.root ),
	empty( other.empty )
{
}

string FlattenNode_iterator::GetName() const
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

shared_ptr<ContainerInterface::iterator_interface> FlattenNode_iterator::Clone() const
{
	shared_ptr<FlattenNode_iterator> ni( new FlattenNode_iterator(*this) );
	return ni;
}

FlattenNode_iterator &FlattenNode_iterator::operator++()
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

FlattenNode_iterator::reference FlattenNode_iterator::operator*() const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        return *cit;
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        return *ptr;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

FlattenNode_iterator::pointer FlattenNode_iterator::operator->() const
{
	return &operator*();
}

bool FlattenNode_iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const FlattenNode_iterator *pi = dynamic_cast<const FlattenNode_iterator *>(&ib);
	ASSERT(pi)("Comparing traversing iterator with something else ")(ib);
	if( pi->IsAtEnd() || IsAtEnd() )
		return pi->IsAtEnd() && IsAtEnd();
	return pi->mit==mit && pi->cit==cit; 
}

void FlattenNode_iterator::Overwrite( FlattenNode_iterator::pointer v ) const
{
    ASSERT( !IsAtEnd() );
	if( dynamic_cast<ContainerInterface *>(GetCurrentMember()) )
        cit.Overwrite( v );
    else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(GetCurrentMember()) )
        *ptr = *v;
    else
        ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
}

const bool FlattenNode_iterator::IsOrdered() const
{
	return true; // traverse walks tree in order generally
}


////////////////////////// Walk //////////////////////////

Walk_iterator::Walk_iterator() :
    out_filter( NULL ),
    recurse_filter( NULL ),
    done( true )
{
}        


Walk_iterator::Walk_iterator( TreePtr<Node> &r,
                              Filter *of,
                              Filter *rf ) :
    root( new TreePtr<Node>(r) ),
    out_filter( of ),
    recurse_filter( rf ),
    done( false )
{
    DoNodeFilter();
}


Walk_iterator::Walk_iterator( const Walk_iterator & other ) :
    root( other.root ),
    out_filter( other.out_filter ),
    recurse_filter( other.recurse_filter ), // See note in header about recurse nodes
    state( other.state ),
    done( other.done )
{
}


Walk_iterator &Walk_iterator::operator++()
{
    AdvanceInto();
    DoNodeFilter();
    return *this;
}


Walk_iterator::reference Walk_iterator::operator*() const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

    if( state.empty() )
    {
        return *root;
    }
    else
    {
        return *(state.back().iterator);
    }
}


Walk_iterator::pointer Walk_iterator::operator->() const
{
    return &operator*();
}


// See note in header about recurse nodes
list< TreePtr<Node> > Walk_iterator::GetPath() const
{
    list< TreePtr<Node> > l;
    FOREACH( const StateEntry &se, state )
        l.push_back( se.node );
    return l;
}


bool Walk_iterator::IsAtEndOfChildren() const
{
	ASSERT( !done ); // Caller should have already checked this (done is our resultant end-state for the whole walk)

	if( state.empty() )
		return false; // At root i.e. initial state - this is not an end state

	// An end state means top iterator is at "end"
    return state.back().iterator == state.back().container->end();
}


void Walk_iterator::BypassEndOfChildren()
{
	ASSERT( !done );
	while( IsAtEndOfChildren() )
	{
		state.pop_back();

	    done = state.empty();
	    if( done )
	    	break;

	    ++state.back().iterator;
	}
}


shared_ptr<ContainerInterface> Walk_iterator::GetChildContainer( TreePtr<Node> n ) const
{ 
    return shared_ptr<ContainerInterface>( new FlattenNode(n) );
}        


void Walk_iterator::Push( TreePtr<Node> n )
{ 
    StateEntry ns;
    ns.node = n;
    ns.container = GetChildContainer( n );
    ns.iterator = ns.container->begin();
    state.push_back( ns );
}        


string Walk_iterator::GetName() const
{
    string s;
    FOREACH( const StateEntry &se, state )
    {
        s += (*se.node).GetTrace() + string(" ");
    }
    return s;
}


void Walk_iterator::AdvanceInto()
{
	ASSERT( !done );
	ASSERT( !IsAtEndOfChildren() );
	TreePtr<Node> element = **this; // look at current node
	bool recurse = false;
	if( element ) // must be non-NULL
	{
		if( recurse_filter ) // is there a filter on recursion?
			recurse = recurse_filter->IsMatch( element, element ); // must pass the restriction
		else
            recurse = true; // no filter so try to recurse into node
	}
	else
        recurse = false; // no node so cannot recurse

    if( recurse )
    {
    	// Step into
        Push( **this );
        BypassEndOfChildren(); // get past an empty child
    }
    else
    {
    	// If we can't step into then step over
    	AdvanceOver(); // this will do its own BypassEndOfChildren()
    }
}


void Walk_iterator::AdvanceOver()
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
	    ++state.back().iterator;
  	    BypassEndOfChildren();
	}
}


shared_ptr<ContainerInterface::iterator_interface> Walk_iterator::Clone() const
{
	shared_ptr<Walk_iterator> ni( new Walk_iterator(*this) );
	return ni;
}


bool Walk_iterator::operator==( const ContainerInterface::iterator_interface &ib ) const
{
	const Walk_iterator *pi = dynamic_cast<const Walk_iterator *>(&ib);
	ASSERT(pi)("Comparing walking iterator with something else ")(ib);
	if( pi->done || done )
		return pi->done && done;
	return **pi == **this;
}


void Walk_iterator::Overwrite( Walk_iterator::pointer v ) const
{
    ASSERT( !done )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfChildren() );

    if( state.empty() )
    {
    	*root = *v;
    }
    else
    {
        state.back().iterator.Overwrite( v );
    }
}


const bool Walk_iterator::IsOrdered() const
{
	return true; // walk walks tree in order generally
}


void Walk_iterator::DoNodeFilter()
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

////////////////////////// ParentWalk //////////////////////////

ParentWalk_iterator::ParentWalk_iterator() :
    unique_filter( new UniqueFilter )
{
}        

ParentWalk_iterator::~ParentWalk_iterator() 
{
    delete unique_filter;
}        

ParentWalk_iterator::ParentWalk_iterator( const ParentWalk_iterator & other ) :
    Walk_iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    recurse_filter = unique_filter; 
}

ParentWalk_iterator &ParentWalk_iterator::operator=( const ParentWalk_iterator & other )
{
    Walk_iterator::operator=( other ),
    *unique_filter = *(other.unique_filter);
    recurse_filter = unique_filter; 
    return *this;
}

ParentWalk_iterator::ParentWalk_iterator( TreePtr<Node> &root ) :
    Walk_iterator( root, NULL, unique_filter = new UniqueFilter )
{
}

shared_ptr<ContainerInterface::iterator_interface> ParentWalk_iterator::Clone() const
{
	shared_ptr<ParentWalk_iterator> ni( new ParentWalk_iterator(*this) );
	return ni;
}

////////////////////////// UniqueWalk //////////////////////////

UniqueWalk_iterator::UniqueWalk_iterator() :
    unique_filter( new UniqueFilter )
{
}        

UniqueWalk_iterator::~UniqueWalk_iterator()
{
    delete unique_filter;
}        

UniqueWalk_iterator::UniqueWalk_iterator( const UniqueWalk_iterator & other ) :
    Walk_iterator( other )
{
    unique_filter = new UniqueFilter( *(other.unique_filter) );
    out_filter = unique_filter; 
}

UniqueWalk_iterator &UniqueWalk_iterator::operator=( const UniqueWalk_iterator &other )
{
    Walk_iterator::operator=( other );
    *unique_filter = *(other.unique_filter);
    out_filter = unique_filter; 
    return *this;
}

UniqueWalk_iterator::UniqueWalk_iterator( TreePtr<Node> &root ) :
    Walk_iterator( root, unique_filter = new UniqueFilter, NULL )
{
}

shared_ptr<ContainerInterface::iterator_interface> UniqueWalk_iterator::Clone() const
{
	shared_ptr<UniqueWalk_iterator> ni( new UniqueWalk_iterator(*this) );
	return ni;
}


