#include "scope.hpp"
#include "walk.hpp"
#include "misc.hpp"

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
// TODO return Scope node (means adding Procedure as a Scope)
shared_ptr<Node> GetScope( shared_ptr<Program> program, shared_ptr<Identifier> id )
{
    TRACE("Trying program (global)\n" );
    FOREACH( shared_ptr<Declaration> d, program->members )
    {            
        if( id == GetIdentifier( d ) )
            return program;
    }

	Flattener<Record> walkr(program);
	FOREACH( shared_ptr<Record> r, walkr )
	{
	    FOREACH( shared_ptr<Declaration> d, r->members )
	    {            
	        if( id == GetIdentifier( d ) ) 
	            return r;
	    }
	}
	
	Flattener<Compound> walkc(program);
	FOREACH( shared_ptr<Compound> c, walkc )
	{
	    Flattener<Declaration> walks(c); // TODO possible bug - this search should not recurse into sub scopes
	    FOREACH( shared_ptr<Declaration> d, walks )
	    {            
	        if( id == GetIdentifier( d ) )
	            return c;
	    }
	}
	
	Flattener<Procedure> walkp(program);
	FOREACH( shared_ptr<Procedure> p, walkp )
	{
	    FOREACH( shared_ptr<Declaration> d, p->members )
	    {            
	        if( id == GetIdentifier( d ) )
	            return p;
	    }
	}
	
	ASSERTFAIL("Could not find scope for identifier"); 
	// every identifier should have a scope - if this fails, we've missed out a kind of scope
	// Note: if Flattener is not automated yet, then it may have missed something
	return shared_ptr<Node>();
}
