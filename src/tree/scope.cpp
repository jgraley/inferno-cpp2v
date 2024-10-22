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
        {
            for( TreePtr<Declaration> d : s->members )
            {
                if( id == (TreePtr<Identifier>)GetIdentifierOfDeclaration( d ) ) 
                    return s;
            }
        }
	}
	
	// Special additional processing for Compounds - look for statements that are really Instance Declarations
	Walk walkc(program, nullptr, nullptr);
	for( const TreePtrInterface &n : walkc )
	{
    	if( TreePtr<Compound> c = DynamicTreePtrCast<Compound>((TreePtr<Node>)n) )
			for( TreePtr<Statement> s : c->statements )
			{
				if( TreePtr<Instance> d = DynamicTreePtrCast<Instance>(s) )
					if( id == GetIdentifierOfDeclaration( d ) )
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
