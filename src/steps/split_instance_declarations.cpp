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
    MakePatternPtr<Compound> sc;
    MakePatternPtr<LocalVariable> si;
    MakePatternPtr< Overlay<LocalVariable> > over;
    si->identifier = MakePatternPtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternPtr<Expression>();  // Only acting on initialised Instances
    MakePatternPtr< Star<Declaration> > decls;
    sc->members = ( decls );
    MakePatternPtr< Star<Statement> > pre, post;
    sc->statements = ( pre, over, post );

    MakePatternPtr<Compound> rc;
    MakePatternPtr<LocalVariable> ri;
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternPtr<Uninitialised>();
    rc->members = ( over, decls );
    MakePatternPtr<Assign> ra;
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( pre, ra, post );

    Configure(sc, rc);
}
	
	
MoveInstanceDeclarations::MoveInstanceDeclarations()
{	
	// Just move the decl to the decls collection
    MakePatternPtr<Compound> sc;
    MakePatternPtr<LocalVariable> var;
    MakePatternPtr< Star<Declaration> > decls;
    sc->members = ( decls );
    MakePatternPtr< Star<Statement> > pre, post;
    sc->statements = ( pre, var, post );

    MakePatternPtr<Compound> rc;
    rc->members = ( var, decls ); // Instance now in unordered decls part
    rc->statements = ( pre, post );

    Configure(sc, rc);
}


SplitInstanceDeclarations2::SplitInstanceDeclarations2()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    MakePatternPtr<Compound> sc;
    MakePatternPtr<LocalVariable> si;
    MakePatternPtr< Overlay<LocalVariable> > over;
    si->identifier = MakePatternPtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternPtr<Expression>();  // Only acting on initialised Instances
    MakePatternPtr< Star<Declaration> > decls;
    sc->members = ( decls, over );
    MakePatternPtr< Star<Statement> > stmts;
    sc->statements = ( stmts );

    MakePatternPtr<Compound> rc;
    MakePatternPtr<LocalVariable> ri;
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakePatternPtr<Uninitialised>();
    rc->members = ( over, decls );
    MakePatternPtr<Assign> ra;
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( ra, stmts );

    Configure(sc, rc);
}
	
	


