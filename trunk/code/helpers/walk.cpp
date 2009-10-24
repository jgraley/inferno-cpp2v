
#include "tree/tree.hpp"
#include "walk.hpp"

bool Walk::IsValid()
{
    if( state.empty() )
        return true;

    Frame &f = state.top();

    return f.index < f.children.size();                      
}

void Walk::AdvanceOver()
{
    Frame &f = state.top();
    if( state.empty() )
        return;

    if( f.index == f.children.size() )
    {
        Pop();
        if( state.empty() )
            return;
        AdvanceOver();
        return;
    }

    f.index++;                
}

void Walk::Push( shared_ptr<Node> n )
{ 
    Frame f;

    vector< Itemiser::Element * > members = Itemiser::Itemise(n);
    for( int i=0; i<members.size(); i++ )
    {
        if( GenericContainer *con = dynamic_cast<GenericContainer *>(members[i]) )
        {
        	//FOREACH( GenericSharedPtr &n, *con )
        	for( GenericContainer::iterator i=con->begin(); i!=con->end(); ++i )
                f.children.push_back( i );
        }            
        else if( GenericSharedPtr *ptr = dynamic_cast<GenericSharedPtr *>(members[i]) )         
        {
            f.children.push_back( GenericPointIterator(ptr) );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a container or a shared pointer");
        }
    }

    f.index = 0;
    state.push( f );
}        

void Walk::Pop()
{
    state.pop();
}

Walk::Walk( shared_ptr<Node> r ) :
    root( r ) 
{
    Frame f;
    GenericSharedPtr *gsp = &root;
    GenericPointIterator gpi(gsp);
    f.children.push_back( gpi ); // TODO this makes the point iterator for the top level be invalid
    f.index = 0;
    state.push( f );
}        
    
bool Walk::Done()
{
    return state.empty();
}    

int Walk::Depth()
{
    return state.size();
}
    
GenericContainer::iterator Walk::GetIterator()
{
    ASSERT( !state.empty() );        
    ASSERT( IsValid() );
        
    Frame f = state.top();
    ASSERT( f.index < f.children.size() );
    return f.children[f.index];
}

shared_ptr<Node> Walk::Get()
{
    return *GetIterator();
}

string Walk::GetPathString()
{
    string s;
    stack< Frame > ps = state; // std::stack doesn't have [] so copy the whole thing and go backwards
    while( !ps.empty() )
    {
        Frame f = ps.top();
        ps.pop();
        
        // node type
        GenericContainer::iterator child = f.children[f.index];
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
    if( IsValid() && Get() )
        Push( Get() );
    else
        AdvanceOver();
                
    while( !IsValid() )
        AdvanceOver();        
}

