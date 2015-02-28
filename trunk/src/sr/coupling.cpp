#include "coupling.hpp"
#include "helpers/walk.hpp" 
#include "search_replace.hpp" 

using namespace SR;


CouplingKeys::CouplingKeys() : 
    master(NULL) 
{
}

void CouplingKeys::DoKey( TreePtr<Node> x, 
	                      Agent *agent )
{
    ASSERT(this);
    ASSERT(x);
	shared_ptr<Key> key( new Key );
	if( x )
    {
        ASSERT( x->IsFinal() )("Intermediate node ")(*x)(" set as key by/for ")(*agent);
        key->root = x;
    }
    else
        key = shared_ptr<Key>();
	return DoKey( key, agent );
}


void CouplingKeys::DoKey( shared_ptr<Key> key, 
	                      Agent *agent )
{
	//INDENT;
	ASSERT( this );
    ASSERT( agent );
        
    if( key )
    {
        ASSERT( key->root );
        ASSERT( key->root->IsFinal() )("Intermediate node ")(*(key->root))(" set as key by/for ")(*agent);
    }
    
    // If we're keying and we haven't keyed this node so far, key it now
	if( key && !GetKey( agent ) )
	{        
	    key->agent = agent;
		keys_map[agent] = key;	
	}
	
	if( master )
        master->DoKey( key, agent );
}


TreePtr<Node> CouplingKeys::GetCoupled( Agent *agent )  
{
    ASSERT(this);
    shared_ptr<Key> k = GetKey( agent );
	if( k )
	    return k->root;
	else
	    return TreePtr<Node>(); 
}


shared_ptr<Key> CouplingKeys::GetKey( Agent *agent )  
{
    ASSERT(this);
	if( keys_map.IsExist(agent) )
	{
	    return keys_map[agent];
	}
	else if( master )
	{
	    shared_ptr<Key> k = master->GetKey(agent);
	    return k;
	}
	else
	    return shared_ptr<Key>(); 
}


void CouplingKeys::SetMaster( CouplingKeys *m ) 
{ 
    master = m; 
}

