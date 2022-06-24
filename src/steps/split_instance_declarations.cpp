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
    auto sc = MakePatternNode<Compound>();
    auto si = MakePatternNode<LocalVariable>();
    auto over = MakePatternNode< Delta<LocalVariable> >();
    si->identifier = MakePatternNode<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternNode<Expression>();  // Only acting on initialised Instances
    auto decls = MakePatternNode< Star<Declaration> >();
    sc->members = { decls };
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    sc->statements = ( pre, over, post );

    auto rc = MakePatternNode<Compound>();
    auto ri = MakePatternNode<LocalVariable>();
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternNode<Uninitialised>();
    rc->members = ( over, decls );
    auto ra = MakePatternNode<Assign>();
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( pre, ra, post );

    Configure( SEARCH_REPLACE,sc, rc);
}
	
	
MoveInstanceDeclarations::MoveInstanceDeclarations()
{	
	// Just move the decl to the decls collection
    auto sc = MakePatternNode<Compound>();
    auto var = MakePatternNode<LocalVariable>();
    auto decls = MakePatternNode< Star<Declaration> >();
    sc->members = ( decls );
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    sc->statements = ( pre, var, post );

    auto rc = MakePatternNode<Compound>();
    rc->members = ( var, decls ); // Instance now in unordered decls part
    rc->statements = ( pre, post );

    Configure( SEARCH_REPLACE,sc, rc);
}


SplitInstanceDeclarations2::SplitInstanceDeclarations2()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    auto sc = MakePatternNode<Compound>();
    auto si = MakePatternNode<LocalVariable>();
    auto over = MakePatternNode< Delta<LocalVariable> >();
    si->identifier = MakePatternNode<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternNode<Expression>();  // Only acting on initialised Instances
    auto decls = MakePatternNode< Star<Declaration> >();
    sc->members = ( decls, over );
    auto stmts = MakePatternNode< Star<Statement> >();
    sc->statements = ( stmts );

    auto rc = MakePatternNode<Compound>();
    auto ri = MakePatternNode<LocalVariable>();
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternNode<Uninitialised>();
    rc->members = ( over, decls );
    auto ra = MakePatternNode<Assign>();
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( ra, stmts );

    Configure( SEARCH_REPLACE,sc, rc);
}
	
	


