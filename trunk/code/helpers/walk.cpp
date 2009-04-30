
#include "tree/tree.hpp"
#include "walk.hpp"

bool Walk::IsValid()
{
    if( state.empty() )
        return true;

    Frame &f = state.top();

    return f.index < f.children.size();                      
}

void Walk::Iterate()
{
    Frame &f = state.top();
    if( state.empty() )
        return;

    if( f.index == f.children.size() )
    {
        Pop();
        if( state.empty() )
            return;
        Iterate();
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
        if( GenericSequence *seq = dynamic_cast<GenericSequence *>(members[i]) )                
        {
            for( int j=0; j<seq->size(); j++ )
                f.children.push_back( &(seq->Member(j)) );
        }            
        else if( GenericPointer *ptr = dynamic_cast<GenericPointer *>(members[i]) )         
        {
            f.children.push_back( ptr );
        }
        else
        {
            ASSERT(!"got something from itemise that isnt a sequence or a shared pointer");               
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
    f.children.push_back( &root );
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
    
GenericPointer *Walk::GetGeneric()
{
    ASSERT( !state.empty() );        
    ASSERT( IsValid() );
        
    Frame f = state.top();
    ASSERT( f.index < f.children.size() );
    return f.children[f.index];
}

shared_ptr<Node> Walk::Get()
{
    GenericPointer *gp = GetGeneric();
    ASSERT( gp );
    return gp->Get();
}

void Walk::Advance()
{
    if( IsValid() && Get() )
        Push( Get() );
    else
        Iterate();
                
    while( !IsValid() )
        Iterate();        
}

