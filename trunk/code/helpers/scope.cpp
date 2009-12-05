#include "scope.hpp"
#include "walk.hpp"
#include "misc.hpp"

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
// TODO take id as SpecificIdentifier, not Identifier, so do not need to ASSERT check this
shared_ptr<Scope> GetScope( shared_ptr<Program> program, shared_ptr<Identifier> id )
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
	                                     // TODO also should ensure we are checking members AND statements
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

	
	if( shared_ptr<SpecificIdentifier> sid = dynamic_pointer_cast<SpecificIdentifier>( id ) )
		ASSERT(0)("cannot get scope of ")( *sid );
	else
		ASSERT(0)("non-specific type ")(*id)(" - should not be doing GetScope() on these" );
	// every identifier should have a scope - if this fails, we've missed out a kind of scope
	// Note: if Flattener is not automated yet, then it may have missed something
	return shared_ptr<Scope>();
}
