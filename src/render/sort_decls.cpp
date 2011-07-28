
#include "sort_decls.hpp"
#include "tree/misc.hpp"

using namespace CPPTree;

/** Walks the tree, avoiding recursing into the body (initialiser) of Subroutines. */
class TraverseNoBody_iterator : public Traverse::iterator
{
public:
    TraverseNoBody_iterator( TreePtr<Node> &root ) : Traverse::iterator(root) {}        
    TraverseNoBody_iterator() : Traverse::iterator() {}
protected:
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
    {
        // We need to create a container of elements of the child.
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( n ) ) // an instance...
            if( dynamic_pointer_cast<Subroutine>( i->type ) ) // ...of a function
            {
                // it's an instance, so set up a container containing type and identifier only, 
                // not initialiser (others don't matter for deps purposes). We need 
                // the type for params etc
                shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
                seq->push_back( i->type );
                seq->push_back( i->identifier );
                return seq;
            }
        // it's not a slave, so proceed as for Traverse
        return Traverse::iterator::GetChildContainer(n);
    }
};

typedef ContainerFromIterator< TraverseNoBody_iterator, TreePtr<Node> > TraverseNoBody;

/** Walks the tree, avoiding recursing into the body (initialiser) of Subroutines. */
class TraverseNoBodyOrIndirection_iterator : public TraverseNoBody::iterator
{
public:
    TraverseNoBodyOrIndirection_iterator( TreePtr<Node> &root ) : TraverseNoBody::iterator(root) {}        
    TraverseNoBodyOrIndirection_iterator() : TraverseNoBody::iterator() {}
private:
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
    {
        // We need to create a container of elements of the child.
        if( dynamic_pointer_cast<Indirection>( n ) ) // an instance...
        {
            shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
            return seq;
        }
        // it's not a slave, so proceed as for Traverse
        return TraverseNoBody::iterator::GetChildContainer(n);
    }
};


typedef ContainerFromIterator< TraverseNoBodyOrIndirection_iterator, TreePtr<Node> > TraverseNoBodyOrIndirection;

// Does a depend on b?
bool IsDependOn( TreePtr<Declaration> a, TreePtr<Declaration> b, bool ignore_indirection_to_record )
{
	if( a == b )
	    return false;
	
    const TreePtr<Record> recb = dynamic_pointer_cast<Record>(b); // 2 unrelated uses
	
	// Only ignore pointers and refs if we're checking dependency on a record; typedefs
	// still apply (since you can't forward declare a typedef).
    bool ignore_indirection = ignore_indirection_to_record && recb;

    // Actually, we really want to see whether declaration a depends on the identifier of b
    // since the rest of b is irrelevent (apart from the above).
    TreePtr<Identifier> ib = GetIdentifier( b );
    ASSERT(ib);
          
    TraverseNoBody wnb( a );
    TraverseNoBodyOrIndirection wnbi( a );
    ContainerInterface *w = ignore_indirection ? (ContainerInterface *)&wnbi : (ContainerInterface *)&wnb;
        
    ContainerInterface::iterator wa=w->begin();
    while(!(wa == w->end()))
    {
    /*	if( ignore_indirection ) // are we to ignore pointers/refs?
    	{
    		if( TreePtr<Indirection> inda = TreePtr<Indirection>::DynamicCast(*wa) ) // is a a pointer or ref?
    		{
    			if( dynamic_pointer_cast<Identifier>(inda->destination) == ib ) // does it depend on b?
    	        {
    	        	wa.AdvanceOver(); // Then skip it
    	        	continue;
    	        }
    		}
    	}
    	*/
        if( TreePtr<Node>(*wa) == TreePtr<Node>(ib) ) // If we see b in *any* other context under a's type, there's dep.
            return true;                

        ++wa;
    }
    
    // Recurse though members of records since Inferno doesn't require scope to be remembered - so
    // the dependency might be on something buried in the record.
    if( recb )
    {
    	FOREACH( TreePtr<Declaration> memberb, recb->members )
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
    FOREACH( const TreePtrInterface &a, c )
    	cc.insert( a );

	// Keep searching our local container of decls (cc) for decls that do not depend
    // on anything else in the container. Such a decl may be safely rendered before the
    // rest of the decls, so place it at the end of the sequence we are building up
    // (s) and remove from the container cc since cc only holds the ones we have still to
    // place in s. Repeat until we've done all the decls at which point cc is empty.
    // If no non-dependent decl may be found in cc then it's irredemably circular and
    // we fail. This looks like O(N^3). Well, that's just a damn shame.
	TRACE("Adding decls in dep order: ");
	while( !cc.empty() )
	{
		bool found = false; // just for ASSERT check
		FOREACH( const TreePtr<Declaration> &a, cc )
		{
			bool a_has_deps=false;
			FOREACH( const TreePtr<Declaration> &b, cc )
		    {
		        TreePtr<Declaration> aid = dynamic_cast< const TreePtr<Declaration> & >(a);
		    	a_has_deps |= IsDependOn( aid, b, ignore_indirection_to_record );
		    }
		    if( !a_has_deps )
		    {
		        TRACE(*a)(" ");
				s.push_back(a);
				cc.erase(a);
				found = true;
		        break;
		    }
		}

		if( !found )
		{   
		    TRACE("\nRemaining unsequenceable decls: ");
		    FOREACH( const TreePtr<Declaration> &a, cc )
   		        TRACE(*a)(" ");
		}
		ASSERT( found )("\nfailed to find a decl to add without dependencies, maybe circular\n");
		//TRACE("%d %d\n", s.size(), c.size() );
	}
    TRACE("\n");
	ASSERT( s.size() == ocs );
	return s;
}


Sequence<Declaration> JumbleDecls( Sequence<Declaration> c )
{
	srand(99);
	
	Sequence<Declaration> s;
	FOREACH( TreePtr<Declaration> to_insert, c ) // we will insert each element from the collection
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
	FOREACH( TreePtr<Declaration> to_insert, c ) // we will insert each element from the collection
	{
   	    // Insert the element. If we didn't find a dependency, we'll be off the end of
   	    // the sequence and hopefully insert() will actually push_back()
  		s.insert( s.begin(), to_insert ); 
	}
	
	ASSERT( s.size() == c.size() );
	return s;
}

