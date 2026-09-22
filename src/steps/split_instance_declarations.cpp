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
    // Just move the decls to the top of the body. Use AdvanceDeclaration
    // so we move Instance and TypeDeclaration but not LabelDeclaration, which maybe 
    // shouldn't be a Declaration at all.
    auto sc = MakePatternNode<Compound>();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto pre_decls = MakePatternNode<StarAgent, AdvanceDeclaration>();
    auto first_non_decl = MakePatternNode<NegationAgent, Statement>();
    auto post_any = MakePatternNode<StarAgent, Statement>();
    auto mid_decl = MakePatternNode<AdvanceDeclaration>();
    auto mid_not_decls = MakePatternNode<StarAgent, Statement>();
    auto x_post = MakePatternNode<NegationAgent, Statement>();

	// Carefully preserve the relative order of the decls, as well as the statements.
    sc->members = ( decls );
    sc->statements = ( pre_decls, first_non_decl, mid_not_decls, mid_decl, post_any );
    first_non_decl->negand = MakePatternNode<AdvanceDeclaration>();
    mid_not_decls->restriction = x_post;
    x_post->negand = MakePatternNode<AdvanceDeclaration>();
	
    auto rc = MakePatternNode<Compound>();
    rc->members = ( decls ); // Instance now in unordered decls part
    rc->statements = ( pre_decls, mid_decl, first_non_decl, mid_not_decls, post_any );

    Configure( SEARCH_REPLACE,sc, rc);
}

