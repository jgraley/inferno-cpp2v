
#include "tree/tree.hpp"
#include "walk.hpp"

bool Walk::IsAtEndOfCollection() const
{
	ASSERT( !Done() );

    const Frame &f = state.top();

    return f.index == f.children.size();
}

void Walk::BypassInvalid()
{
	while( !Done() && IsAtEndOfCollection() )
        PoppingIncrement();
}

void Walk::PoppingIncrement()
{
	while( !Done() && IsAtEndOfCollection() )
 		Pop();

    if( !Done() )
		state.top().index++;
}

void Walk::Push( TreePtr<Node> n )
{ 
    Frame f;

    vector< Itemiser::Element * > members = n->Itemise();
    for( int i=0; i<members.size(); i++ )
    {
        if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(members[i]) )
        {
        	//FOREACH( TreePtrInterface &n, *con )
        	for( ContainerInterface::iterator i=con->begin(); i!=con->end(); ++i )
                f.children.push_back( i );
        }            
        else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
        {
            f.children.push_back( PointIterator(ptr) );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a container or a shared pointer");
        }
    }

    f.index = 0;
    state.push( f );
}        

void Walk::Pop()
{
    state.pop();
}

Walk::Walk( TreePtr<Node> r, TreePtr<Node> res ) :
    root( new TreePtr<Node>(r) ),
    restrictor( res )
{
	if( *root ) // if root is NULL, leave the state empty so Done() always returns true
	{
        Frame f;
        PointIterator gpi(&*root);
        f.children.push_back( gpi );
        f.index = 0;
        state.push( f );
	}
}        

Walk::Walk( const Walk & other ) :
	root( other.root ),
	restrictor( other.restrictor ),
	state( other.state )
{
}

bool Walk::Done() const
{
    return state.empty();
}    

int Walk::Depth() const
{
    return state.size();
}
    
ContainerInterface::iterator Walk::GetIterator() const
{
    ASSERT( !Done() )("Already advanced over everything; reached end of walk");
    ASSERT( !IsAtEndOfCollection() );
        
    Frame f = state.top();
    ASSERT( f.index < f.children.size() );
    return f.children[f.index];
}

TreePtr<Node> Walk::Get() const
{
    return *GetIterator();
}

void Walk::Set( TreePtr<Node> n )
{
    GetIterator().Overwrite( &n );
}

Walk::operator string() const
{
    string s;
    stack< Frame > ps = state; // std::stack doesn't have [] so copy the whole thing and go backwards
    while( !ps.empty() )
    {
        Frame f = ps.top();
        ps.pop();
        
        // node type
        ContainerInterface::iterator child = f.children[f.index];
        if( *child )
            s = TypeInfo(*(child->get())).name() + s;
        else
            s = string("NULL") + s;

        // member/element number
        if( !ps.empty() ) // bottom level index always 0, don't print
            s = SSPrintf(":%d ", f.index) + s; // going backwards so prepend
    }
    return s;
}

void Walk::AdvanceInto()
{
	ASSERT( !Done() );
	ASSERT( !IsAtEndOfCollection() );
	TreePtr<Node> element = Get(); // look at current node
    if( element &&                                                    // must be non-NULL
    	(!restrictor || restrictor->IsLocalMatch(element.get()) ) ) // and pass the restriction
    {
    	// Step into
        Push( Get() );
        BypassInvalid();
    }
    else
    {
    	// If we can't step into then step over
    	AdvanceOver();
    }
}

void Walk::AdvanceOver()
{
	ASSERT( !Done() );
	ASSERT( !IsAtEndOfCollection() );
	PoppingIncrement();
	BypassInvalid();
}

