#include "split_instance_declarations.hpp"
#include "tree/cpptree.hpp"
#include "pattern_helpers.hpp"

using namespace CPPTree;
using namespace Steps;

// TODO these to go in a container of transformations, and rename better

SplitInstanceDeclarations::SplitInstanceDeclarations()
{
    // Match a compound with an ini9tialised decl in the statements. Replace
    // with an uninitialised decl and an assign. Put the new decl in the 
    // decls section of the compound.
    auto sc = MakePatternNode<Compound>();
    auto si = MakePatternNode<Local>();
    auto delta = MakePatternNode<DeltaAgent, Local>();
    si->permission = MakePatternNode<NonConst>();  // Cannot split const
    si->identifier = MakePatternNode<InstanceIdentifier>();  // Only acting on initialised Instances
    si->initialiser = MakePatternNode<Expression>();  // Only acting on initialised Instances
    auto decls = MakePatternNode<StarAgent, Declaration>();
    sc->members = { decls };
    auto pre = MakePatternNode<StarAgent, Statement>();
    auto post = MakePatternNode<StarAgent, Statement>();
    sc->statements = ( pre, delta, post );

    auto rc = MakePatternNode<Compound>();
    auto ri = MakePatternNode<Local>();
    delta->through = si;
    delta->overlay = ri;
    ri->permission = MakePatternNode<NonConst>();  
    ri->initialiser = MakePatternNode<Uninitialised>();
    rc->members = ( decls );
    auto ra = MakePatternNode<Assign>();
    ra->operands = ( si->identifier, si->initialiser );
    rc->statements = ( pre, delta, ra, post );

    Configure( SEARCH_REPLACE,sc, rc);
}
    
    
MoveDeclarationsToTheTop::MoveDeclarationsToTheTop()
{    
    // Just move the decl to the decls collection
    auto sc = MakePatternNode<Compound>();
    auto var = MakePatternNode<Local>();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto decls2 = MakePatternNode<StarAgent, Declaration>();
    sc->members = ( decls );
    auto pre = MakePatternNode<StarAgent, Statement>();
    auto non_decl = MakePatternNode<NegationAgent, Statement>();
    auto post = MakePatternNode<StarAgent, Statement>();
    sc->statements = ( decls2, non_decl, pre, var, post );

    auto rc = MakePatternNode<Compound>();
    rc->members = ( decls ); // Instance now in unordered decls part
    rc->statements = ( decls2, var, non_decl, pre, post );
    non_decl->negand = MakePatternNode<Declaration>();

    Configure( SEARCH_REPLACE,sc, rc);
}

