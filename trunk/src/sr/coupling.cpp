#include "coupling.hpp"
#include "helpers/walk.hpp" 
#include "search_replace.hpp" 

CouplingKeys::CouplingKeys() : 
    master(NULL) 
{
}

void CouplingKeys::DoKey( TreePtr<Node> x, 
	                      TreePtr<Node> pattern, 
	                      Conjecture::Choice *gc,
	                      int go )
{
//	INDENT;
	shared_ptr<Key> key( new Key );
	if( x )
        key->root = x;
    else
        key = shared_ptr<Key>();
	return DoKey( key, pattern, gc, go );
}


void CouplingKeys::DoKey( shared_ptr<Key> key, 
	                      TreePtr<Node> pattern, 
	                      Conjecture::Choice *gc,
	                      int go )
{
	//INDENT;
	ASSERT( this );
    ASSERT( pattern );
    
   // TRACE("CouplingKeys@%p: ", this)(master?"has ":"does not have ")("master\n");
    
    if( key )
    {
        ASSERT( key->root );
    }
    
#if 0    
	TRACE("coupling={");
	bool first=true;
	FOREACH( TreePtr<Node> n, coupling )
	{
		if( !first )
			TRACE(", ");
		if( pattern == n )
			TRACE("-->");
		TRACE(*n);
		first=false;
	}
   TRACE("} key ptr=%p\n", keys_map[coupling].get()); // TODO put this in as a common utility somewhere
#endif

	// If we're keying and we haven't keyed this node so far, key it now
	if( key && !GetKey( pattern ) )
	{
	    key->replace_pattern = pattern;
		key->governing_choice = gc;	
		key->governing_offset = go;	
		keys_map[pattern] = key;	
      //  TRACE("Keyed root=")(*key->root)(" pattern=")(*key->replace_pattern)(" with governing_choice=%p\n", gc);
	}
	
    // TRACE("@%p Keyed ", this)(*(key->root))(" size %d\n", keys_map.size());
}


TreePtr<Node> CouplingKeys::GetCoupled( TreePtr<Node> pattern )  
{
  //  INDENT;
    shared_ptr<Key> k = GetKey( pattern );
	if( k )
	    return k->root;
	else
	    return TreePtr<Node>(); // TODO may never happen now, try assert
}


shared_ptr<Key> CouplingKeys::GetKey( TreePtr<Node> pattern )  
{
 //   INDENT;
    //TRACE("@%p Getting key for ", this)(*pattern)(" master is %p size %d\n", master, keys_map.size());
	if( keys_map.IsExist(pattern) )
	{
	    return keys_map[pattern];
	}
	else if( master )
	{
	   // TRACE("Going to master to get key for ")(*pattern)("\n");
	    shared_ptr<Key> k = master->GetKey(pattern);
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
            Expand e(p.second->root, &uf); 
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

