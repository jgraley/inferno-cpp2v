#include "coupling.hpp"
#include "helpers/walk.hpp" 
#include "search_replace.hpp" 

using namespace SR;


CouplingKeys::CouplingKeys() : 
    master(NULL) 
{
}

void CouplingKeys::DoKey( TreePtr<Node> x, 
	                      Agent *agent, 
	                      Conjecture::Choice *gc,
	                      int go )
{
//	INDENT;
    ASSERT(this);
	shared_ptr<Key> key( new Key );
	if( x )
    {
        ASSERT( x->IsFinal() )("Intermediate node ")(*x)(" set as key by/for ")(*agent);
        key->root = x;
    }
    else
        key = shared_ptr<Key>();
	return DoKey( key, agent, gc, go );
}


void CouplingKeys::DoKey( shared_ptr<Key> key, 
	                      Agent *agent, 
	                      Conjecture::Choice *gc,
	                      int go )
{
	//INDENT;
	ASSERT( this );
    ASSERT( agent );
    
   // TRACE("CouplingKeys@%p: ", this)(master?"has ":"does not have ")("master\n");
    
    if( key )
    {
        ASSERT( key->root );
        ASSERT( key->root->IsFinal() )("Intermediate node ")(*(key->root))(" set as key by/for ")(*agent);
    }
    
#if 0    
	TRACE("coupling={");
	bool first=true;
	FOREACH( TreePtr<Node> n, coupling )
	{
		if( !first )
			TRACE(", ");
		if( agent == Agent::AsAgent(n) )
			TRACE("-->");
		TRACE(*n);
		first=false;
	}
   TRACE("}\n"); // TODO put this in as a common utility somewhere
#endif

	// If we're keying and we haven't keyed this node so far, key it now
	if( key && !GetKey( agent ) )
	{        
	    key->agent = agent;
		key->governing_choice = gc;	
		key->governing_offset = go;	
		keys_map[agent] = key;	
      //  TRACE("Keyed root=")(*key->root)(" agent=")(*key->agent)(" with governing_choice=%p\n", gc);
	}
	
    // TRACE("@%p Keyed ", this)(*(key->root))(" size %d\n", keys_map.size());
}


TreePtr<Node> CouplingKeys::GetCoupled( Agent *agent )  
{
  //  INDENT;
    ASSERT(this);
    shared_ptr<Key> k = GetKey( agent );
	if( k )
	    return k->root;
	else
	    return TreePtr<Node>(); // TODO may never happen now, try assert
}


shared_ptr<Key> CouplingKeys::GetKey( Agent *agent )  
{
 //   INDENT;
    //TRACE("@%p Getting key for ", this)(*pattern)(" master is %p size %d\n", master, keys_map.size());
    ASSERT(this);
	if( keys_map.IsExist(agent) )
	{
	    return keys_map[agent];
	}
	else if( master )
	{
	   // TRACE("Going to master to get key for ")(*agent)("\n");
	    shared_ptr<Key> k = master->GetKey(agent);
	  //  if( k )
	  //      TRACE("Got root ")(*(k->root))("\n");
	  //  else
	  //      TRACE("Didn't get key\n");
	    return k;
	}
	else
	    return shared_ptr<Key>(); // TODO may never happen now, try assert
}


Set< TreePtr<Node> > CouplingKeys::GetAllKeys() 
{
 //   INDENT;
    ASSERT(this);
    Set< TreePtr<Node> > s;
    UniqueFilter uf;
 //   TRACE("Key nodes:\n");
    typedef pair< TreePtr<Node>, shared_ptr<Key> > Pc;
    // iterate over out couplings
    FOREACH( Pc p, keys_map )
    {
        ASSERT( p.first );
        if( p.second ) // TODO make this always be non-NULL
        {
   //         TRACE("Coupling of ")(*(p.first))(": ");
            Walk e(p.second->root, &uf); 
            // Iterate over every node in the subtree under the key
            FOREACH( TreePtr<Node> n, e )
            {
                s.insert( n );    
                TRACE(*n)(" ");
            }   
            TRACE("\n");
        }
    }
    
    if( master )
    {
        // Fold in the results of calling the master
        Set< TreePtr<Node> > ms = master->GetAllKeys();
        FOREACH( TreePtr<Node> n, ms )
        {
            s.insert( n );    
            TRACE(*n)(" ");
        }   
    }
        
    return s;
}


void CouplingKeys::SetMaster( CouplingKeys *m ) 
{ 
//    INDENT;
  //  TRACE("@%p Setting master to %p\n", this, m);
    master = m; 
}


void CouplingKeys::Clear() 
{ 
  //  INDENT;
  //  TRACE("@%p Clearing keys\n", this);
    keys_map.clear(); 
}
