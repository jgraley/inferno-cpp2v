#include "split_instance_declarations.hpp"
#include "tree/cpptree.hpp"
#include "inferno_patterns.hpp"

using namespace CPPTree;
using namespace Steps;

// TODO these to go in a container of transformations, and rename better

SplitInstanceDeclarations::SplitInstanceDeclarations()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    MakeTreePtr<Compound> sc;
    MakeTreePtr<LocalVariable> si;
    MakeTreePtr< Overlay<LocalVariable> > over;
    si->identifier = MakeTreePtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakeTreePtr<Expression>();  // Only acting on initialised Instances
    MakeTreePtr< Star<Declaration> > decls;
    sc->members = ( decls );
    MakeTreePtr< Star<Statement> > pre, post;
    sc->statements = ( pre, over, post );

    MakeTreePtr<Compound> rc;
    MakeTreePtr<LocalVariable> ri;
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakeTreePtr<Uninitialised>();
    rc->members = ( over, decls );
    MakeTreePtr<Assign> ra;
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( pre, ra, post );

    Configure(sc, rc);
}
	
	
MoveInstanceDeclarations::MoveInstanceDeclarations()
{	
	// Just move the decl to the decls collection
    MakeTreePtr<Compound> sc;
    MakeTreePtr<LocalVariable> var;
    MakeTreePtr< Star<Declaration> > decls;
    sc->members = ( decls );
    MakeTreePtr< Star<Statement> > pre, post;
    sc->statements = ( pre, var, post );

    MakeTreePtr<Compound> rc;
    rc->members = ( var, decls ); // Instance now in unordered decls part
    rc->statements = ( pre, post );

    Configure(sc, rc);
}


SplitInstanceDeclarations2::SplitInstanceDeclarations2()
{
	// Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    MakeTreePtr<Compound> sc;
    MakeTreePtr<LocalVariable> si;
    MakeTreePtr< Overlay<LocalVariable> > over;
    si->identifier = MakeTreePtr<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakeTreePtr<Expression>();  // Only acting on initialised Instances
    MakeTreePtr< Star<Declaration> > decls;
    sc->members = ( decls, over );
    MakeTreePtr< Star<Statement> > stmts;
    sc->statements = ( stmts );

    MakeTreePtr<Compound> rc;
    MakeTreePtr<LocalVariable> ri;
    over->through = si;
    over->overlay = ri;
    ri->initialiser = MakeTreePtr<Uninitialised>();
    rc->members = ( over, decls );
    MakeTreePtr<Assign> ra;
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( ra, stmts );

    Configure(sc, rc);
}
	
	


