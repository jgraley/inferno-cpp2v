#include "scope.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"

using namespace CPPTree;

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
// TODO take id as SpecificIdentifier, not Identifier, so do not need to ASSERT check this
TreePtr<Scope> GetScope( TreePtr<Program> program, TreePtr<Identifier> id )
{
    TRACE("Trying program (global)\n" );

    // Look through the members of all scopes (Program, Records, CallableParams, Compounds)
    Walk walkr(program, nullptr, nullptr);
	for( const TreePtrInterface &n : walkr )
	{
    	if( TreePtr<Scope> s = DynamicTreePtrCast<Scope>((TreePtr<Node>)n) )
#if 1
        {
            auto && __range = s->members ;
            for (Collection<Declaration>::iterator __begin = __range.begin(), __end = __range.end(); __begin != __end; ++__begin)
            {
                TreePtr<Declaration> d = *__begin;
                if( id == GetIdentifier( d ) ) 
                    return s;
            }
        }
#else	    
        for( TreePtr<Declaration> d : s->members )
	    {            
	        if( id == GetIdentifier( d ) ) 
	            return s;
	    }
#endif
	}
	
	// Special additional processing for Compounds - look for statements that are really Instance Declarations
	Walk walkc(program, nullptr, nullptr);
	for( const TreePtrInterface &n : walkc )
	{
    	if( TreePtr<Compound> c = DynamicTreePtrCast<Compound>((TreePtr<Node>)n) )
			for( TreePtr<Statement> s : c->statements )
			{
				if( TreePtr<Instance> d = DynamicTreePtrCast<Instance>(s) )
					if( id == GetIdentifier( d ) )
						return c;
			}
	}
	
	if( TreePtr<SpecificIdentifier> sid = DynamicTreePtrCast<SpecificIdentifier>( id ) )
		throw ScopeNotFoundMismatch();
	else
		throw ScopeOnNonSpecificMismatch();
	// Every identifier should have a scope - if this fails, we've missed out a kind of scope
	return TreePtr<Scope>();
}
