
#include "sort_decls.hpp"
#include "helpers/misc.hpp"


bool IsDependOn( SharedPtr<Declaration> a, SharedPtr<Declaration> b, bool ignore_indirection_to_record )
{
	if( a == b )
	    return false;
	
    const SharedPtr<Record> recb = dynamic_pointer_cast<Record>(b); // 2 unrelated uses
	
	// Only ignore pointers and refs if we're checking dependency on a record; typedefs
	// still apply (since you can't forward declare a typedef).
    bool ignore_indirection = ignore_indirection_to_record && recb;

    // Actually, we really want to see whether declaration a depends on the identifier of b
    // since the rest of b is irrelevent (apart from the above).
    SharedPtr<Identifier> ib = GetIdentifier( b );
    ASSERT(ib);
          
    Walk wa( a );
    while(!wa.Done())
    {
    	if( ignore_indirection ) // are we to ignore pointers/refs?
    	{
    		if( SharedPtr<Indirection> inda = dynamic_pointer_cast<Indirection>(wa.Get()) ) // is a a pointer or ref?
    		{
    			if( dynamic_pointer_cast<Identifier>(inda->destination) == ib ) // does it depend on b?
    	        {
    	        	wa.AdvanceOver(); // Then skip it
    	        	continue;
    	        }
    		}
    	}
    	
        if( wa.Get() == SharedPtr<Node>(ib) ) // If we see b in *any* other cotext under a's type, there's dep.
            return true;                
                        
        wa.AdvanceInto();
    }
    
    // Recurse though members of records since Inferno doesn't require scope to be remembered - so
    // the dependency might be on something buried in the record.
    if( recb )
    {
    	FOREACH( SharedPtr<Declaration> memberb, recb->members )
    	    if( IsDependOn( a, memberb, ignore_indirection_to_record ) )
    	        return true;
    }

    return false; 
}


Sequence<Declaration> SortDecls( ContainerInterface &c, bool ignore_indirection_to_record )
{
	Sequence<Declaration> s;
    int ocs = c.size();
    
    // Our algorithm will modify the source container, so make a copy of it
    Collection<Declaration> cc;
    ContainerInterface::iterator ai;
    for( ai = c.begin(); ai != c.end(); ++ai )
    	cc.insert( *ai );

	// Go on 'till all the decls in the collection are used up
	while( !cc.empty() )
	{
		Collection<Declaration>::iterator ai;
		for( ai = cc.begin(); ai != cc.end(); ++ai )
		{
			bool a_has_deps=false;
			FOREACH( const SharedPtr<Declaration> &b, cc )
		    {
		        SharedPtr<Declaration> aid = dynamic_cast< const SharedPtr<Declaration> & >(*ai); 
		    	a_has_deps |= IsDependOn( aid, b, ignore_indirection_to_record );
		    }
		    if( !a_has_deps )
		        break;
		}
		ASSERT( ai != cc.end() );//("failed to find a decl to add without dependencies");
		s.push_back(*ai);
		cc.erase(*ai);
		//TRACE("%d %d\n", s.size(), c.size() );
	}

	ASSERT( s.size() == ocs );
	return s;
}


Sequence<Declaration> JumbleDecls( Sequence<Declaration> c )
{
	srand(99);
	
	Sequence<Declaration> s;
	FOREACH( SharedPtr<Declaration> to_insert, c ) // we will insert each element from the collection
	{
		// Idea is to insert each new element just before the first exiting element that
		// depends on it. This is the latest position we can insert the new element.
		Sequence<Declaration>::iterator i;
		int n = rand() % (s.size()+1);
		for( i = s.begin(); i != s.end(); ++i )
   	    {
   	    	if( n-- == 0 )
   	    	{
   	    		// Found element that depends on the one we want to insert. So insert just 
   	    		// before the found element.
   	    		break;    		
   	    	}
   	    }
   	    
   	    // Insert the element. If we didn't find a dependency, we'll be off the end of
   	    // the sequence and hopefully insert() will actually push_back()
  		s.insert( i, to_insert ); 
	}
	
	ASSERT( s.size() == c.size() );
	return s;
}

Sequence<Declaration> ReverseDecls( Sequence<Declaration> c )
{
	Sequence<Declaration> s;
	FOREACH( SharedPtr<Declaration> to_insert, c ) // we will insert each element from the collection
	{
   	    // Insert the element. If we didn't find a dependency, we'll be off the end of
   	    // the sequence and hopefully insert() will actually push_back()
  		s.insert( s.begin(), to_insert ); 
	}
	
	ASSERT( s.size() == c.size() );
	return s;
}

