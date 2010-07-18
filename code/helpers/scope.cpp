#include "scope.hpp"
#include "walk.hpp"
#include "misc.hpp"

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
// TODO take id as SpecificIdentifier, not Identifier, so do not need to ASSERT check this
TreePtr<Scope> GetScope( TreePtr<Program> program, TreePtr<Identifier> id )
{
    TRACE("Trying program (global)\n" );

    // Look through the members of all scopes (Program, Records, Procedures, Compounds)
    Walk walkr(program);
	FOREACH( const TreePtr<Node> n, walkr )
	{
    	if( TreePtr<Scope> s = dynamic_pointer_cast<Scope>(n) )
	    FOREACH( TreePtr<Declaration> d, s->members )
	    {            
	        if( id == GetIdentifier( d ) ) 
	            return s;
	    }
	}
	
	// Special additional processing for Compounds - look for statements that are really Instance Declarations
	Walk walkc(program);
	FOREACH( const TreePtr<Node> n, walkc )
	{
    	if( TreePtr<Compound> c = dynamic_pointer_cast<Compound>(n) )
			FOREACH( TreePtr<Statement> s, c->statements )
			{
				if( TreePtr<Instance> d = dynamic_pointer_cast<Instance>(s) )
					if( id == GetIdentifier( d ) )
						return c;
			}
	}
	
	if( TreePtr<SpecificIdentifier> sid = dynamic_pointer_cast<SpecificIdentifier>( id ) )
		ASSERT(0)("cannot get scope of ")( *sid );
	else
		ASSERT(0)("non-specific type ")(*id)(" - should not be doing GetScope() on these" );
	// every identifier should have a scope - if this fails, we've missed out a kind of scope
	// Note: if Flattener is not automated yet, then it may have missed something
	return TreePtr<Scope>();
}
