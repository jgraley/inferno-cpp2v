#include "split_instance_declarations.hpp"
#include "tree/cpptree.hpp"
#include "sr/agents/all.hpp"
#include "inferno_agents.hpp"

using namespace CPPTree;
using namespace Steps;

// TODO these to go in a container of transformations, and rename better

SplitInstanceDeclarations::SplitInstanceDeclarations()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    auto sc = MakePatternPtr<Compound>();
    auto si = MakePatternPtr<LocalVariable>();
    auto over = MakePatternPtr< Delta<LocalVariable> >();
    si->identifier = MakePatternPtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternPtr<Expression>();  // Only acting on initialised Instances
    auto decls = MakePatternPtr< Star<Declaration> >();
    sc->members = { decls };
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    sc->statements = ( pre, over, post );

    auto rc = MakePatternPtr<Compound>();
    auto ri = MakePatternPtr<LocalVariable>();
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternPtr<Uninitialised>();
    rc->members = ( over, decls );
    auto ra = MakePatternPtr<Assign>();
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( pre, ra, post );

    Configure( SEARCH_REPLACE,sc, rc);
}
	
	
MoveInstanceDeclarations::MoveInstanceDeclarations()
{	
	// Just move the decl to the decls collection
    auto sc = MakePatternPtr<Compound>();
    auto var = MakePatternPtr<LocalVariable>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    sc->members = ( decls );
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    sc->statements = ( pre, var, post );

    auto rc = MakePatternPtr<Compound>();
    rc->members = ( var, decls ); // Instance now in unordered decls part
    rc->statements = ( pre, post );

    Configure( SEARCH_REPLACE,sc, rc);
}


SplitInstanceDeclarations2::SplitInstanceDeclarations2()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    auto sc = MakePatternPtr<Compound>();
    auto si = MakePatternPtr<LocalVariable>();
    auto over = MakePatternPtr< Delta<LocalVariable> >();
    si->identifier = MakePatternPtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternPtr<Expression>();  // Only acting on initialised Instances
    auto decls = MakePatternPtr< Star<Declaration> >();
    sc->members = ( decls, over );
    auto stmts = MakePatternPtr< Star<Statement> >();
    sc->statements = ( stmts );

    auto rc = MakePatternPtr<Compound>();
    auto ri = MakePatternPtr<LocalVariable>();
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternPtr<Uninitialised>();
    rc->members = ( over, decls );
    auto ra = MakePatternPtr<Assign>();
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( ra, stmts );

    Configure( SEARCH_REPLACE,sc, rc);
}
	
	


