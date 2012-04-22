/*
 * clean_up.cpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#include "steps/clean_up.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_patterns.hpp"

using namespace CPPTree;
using namespace Steps;

// Removing superfluous CompundExpression blocks to clean up the code
CleanupCompoundExpression::CleanupCompoundExpression() // LIMITAION: decls in body not allowed
{
     // Lowering compound expressions
     //
     // exp( ({a; b; c; )) ) -> a; b; t=c; exp(t)
     //
     // Temp is used to preserve sequence point after c. This step assumes that
     // all sequence points that need preserving co-incide with the semicolons
     // in a Compound or CompundExpression. It also requires that there be no loops.
    MakeTreePtr< MatchAll<Statement> > s_all;
    MakeTreePtr< PointerIs<Statement> > sx_pointeris;
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr<Expression> sx_expr;
    
    MakeTreePtr< Stuff<Statement> > stuff;
    MakeTreePtr< NotMatch<Statement> > sr_not;
    MakeTreePtr<SequentialScope> sr_comp;
    MakeTreePtr< Star<Declaration> > sr_cdecls;
    MakeTreePtr< Star<Statement> > sr_cstmts;
    
    MakeTreePtr<CompoundExpression> s_ce;
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr< Star<Statement> > s_pre, s_post, body;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr< TransformOf<Expression> > last( &TypeOf::instance );
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("result");
    MakeTreePtr<Assign> r_assign;
    MakeTreePtr< Overlay<Expression> > overlay;
    MakeTreePtr<Type> r_type;

    s_all->patterns = (stuff, sx_pointeris);
    sx_pointeris->pointer = sx_not;
    sx_not->pattern = sx_expr;
    stuff->recurse_restriction = sr_not;
    sr_not->pattern = sr_comp;
    sr_comp->members = sr_cdecls;
    sr_comp->statements = sr_cstmts;
    
    stuff->terminus = overlay;
    overlay->through = s_ce;
    s_ce->statements = ( body, last );
    s_ce->members = ( decls );
    
    r_comp->statements = ( body, r_assign, stuff );
    r_comp->members = ( decls, r_temp );
    r_temp->identifier = r_temp_id;
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_temp->type = r_type;
    r_assign->operands = (r_temp_id, last);
    last->pattern = r_type;
    overlay->overlay = r_temp_id;        
    
    Configure( s_all, r_comp );
}


// Removing superfluous Compund blocks to clean up the code
CleanupCompoundMulti::CleanupCompoundMulti() // LIMITAION: decls in body not allowed
{
     // {x;{a;b;c}y} -> {x;a;b;c;y}
     // Find a compound block as a statement in another compound block. 
     // Merge the decls and insert the statements in the correct sequence..
    MakeTreePtr<Compound> s_inner, s_outer, r_comp;
    MakeTreePtr< Star<Statement> > s_pre, s_post, s_body;
    MakeTreePtr< Star<Declaration> > s_inner_decls, s_outer_decls;

    s_inner->statements = ( s_body );
    s_inner->members = ( s_inner_decls );
    s_outer->statements = ( s_pre, s_inner, s_post );
    s_outer->members = ( s_outer_decls );
    r_comp->statements = ( s_pre, s_body, s_post );
    r_comp->members = ( s_inner_decls, s_outer_decls );

    Configure( s_outer, r_comp );
}


CleanupCompoundSingle::CleanupCompoundSingle() 
{
    // Find a compound block with no decls and one statemewnt. Replace
    // with just the statement
    //
    //{a} -> a TODO need to restrict parent node to Statement: For, If etc OK; Instance is NOT OK
    //         TODO OR maybe just fix renderer for that case
    // Note: this hits eg If(x){a;} which the "Multi" version misses 
    MakeTreePtr< MatchAll<Statement> > all;    
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr<Instance> sx_instance;
    MakeTreePtr< AnyNode<Statement> > node;
    MakeTreePtr< Overlay<Statement> > over;   
    MakeTreePtr<Compound> s_comp;
    MakeTreePtr< Statement > body;

    all->patterns = (node, sx_not);
    node->terminus = over;
    sx_not->pattern = sx_instance;
    sx_instance->initialiser = s_comp;
    over->through = s_comp;
    over->overlay = body;

    s_comp->statements = body;
    // Note: leaving s_comp empty meaning no decls allowed

    Configure( all );
}

CleanupNop::CleanupNop() 
{
    // Find compound block with Nop in it, replace has the Nop removed.
    // Note: Nop is a no-effect statement, sort-of like ; on its own.
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr<Nop> s_nop;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;

    s_comp->members = decls;
    s_comp->statements = (pre, s_nop, post);
    
    r_comp->members = decls;
    r_comp->statements = (pre, post);

    Configure( s_comp, r_comp );
}

CleanupDuplicateLabels::CleanupDuplicateLabels()
{
    // Search for a function that contains a compound block that has 
    // two labels in succession. Replace the pair of labels with a single
    // one.
    //
    // Using a slave, find references to either one of the original labels 
    // and replace by a reference to the new label.
    //
    // Notes:
    // - The slave must operate over the entire function, not just the 
    // compound that containes the labels, because labels have function 
    // scope and the gotos can be anywhere.
    // - Do not assume the usages of the labels will be gotos. We support
    // GCCs goto-a-variable extension in which case a label could be 
    // on the right of an assignment.
    
    MakeTreePtr<Instance> s_instance, r_instance;
    MakeTreePtr< Stuff<Compound> > stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr<Label> s_label1, s_label2, r_label1; // keep l1 and elide l2
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<LabelIdentifier> s_labelid1, s_labelid2;
    MakeTreePtr<BuildLabelIdentifier> r_labelid("%s_%s", BYPASS_WHEN_IDENTICAL);
    MakeTreePtr< MatchAny<LabelIdentifier> > l_s_orrule;
    MakeTreePtr<InstanceIdentifier> identifier;
    MakeTreePtr<Callable> type;
    
    l_s_orrule->patterns = (s_labelid1, s_labelid2);
    
    MakeTreePtr< SlaveSearchReplace<Compound> > r_slave( stuff, l_s_orrule, r_labelid );
    
    s_instance->initialiser = stuff;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_comp->members = decls;
    s_comp->statements = (pre, s_label1, s_label2, post);
    s_label1->identifier = s_labelid1;
    s_label2->identifier = s_labelid2;
    
    r_instance->initialiser = r_slave;
    r_instance->identifier = identifier;
    r_instance->type = type;
    stuff->terminus = overlay;           
    overlay->through = s_comp;
    overlay->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, r_label1, post);
    r_label1->identifier = r_labelid;
    r_labelid->sources = (s_labelid1, s_labelid2);
    
    Configure( s_instance, r_instance );
}

CleanupIneffectualLabels::CleanupIneffectualLabels()
{
    // Search for a function that contains a compound block that has 
    // a label followed by a goto. Remove the label, leaving the goto.
    //
    // Using a slave, find references to either one of the original labels 
    // and replace by a reference to a new merged one.
    //
    // Notes:
    // - The slave must operate over the entire function, not just the 
    // compound that containes the labels, because labels have function 
    // scope and the gotos can be anywhere.
    // - Do not assume the usages of the labels will be gotos. We support
    // GCCs goto-a-variable extension in which case a label could be 
    // on the right of an assignment.
    
    MakeTreePtr<Instance> s_instance, r_instance;
    MakeTreePtr< Stuff<Compound> > stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr<Label> s_label; // keep l1 and elide l2
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<LabelIdentifier> s_labelid1, s_labelid2;
    MakeTreePtr<BuildLabelIdentifier> r_labelid("%s_%s", BYPASS_WHEN_IDENTICAL);
    MakeTreePtr< MatchAny<LabelIdentifier> > l_s_orrule;
    MakeTreePtr<InstanceIdentifier> identifier;
    MakeTreePtr<Callable> type;
    MakeTreePtr<Goto> s_goto, r_goto;
    
    l_s_orrule->patterns = (s_labelid1, s_labelid2);
    
    MakeTreePtr< SlaveSearchReplace<Compound> > r_slave( stuff, l_s_orrule, r_labelid );
    
    s_instance->initialiser = stuff;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_comp->members = decls;
    s_comp->statements = (pre, s_label, s_goto, post);
    s_label->identifier = s_labelid1;
    s_goto->destination = s_labelid2;
    
    r_instance->initialiser = r_slave;
    r_instance->identifier = identifier;
    r_instance->type = type;
    stuff->terminus = overlay;           
    overlay->through = s_comp;
    overlay->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, r_goto, post);
    r_goto->destination = r_labelid;
    r_labelid->sources = (s_labelid1, s_labelid2);
    
    Configure( s_instance, r_instance );
}

CleanupIneffectualGoto::CleanupIneffectualGoto()
{
    // Find a compound containing a Goto and a Label where the 
    // goto goes to the label. Remove the Goto (but not the Label
    // since there may be other Gotos to it).
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr<Goto> s_goto;
    MakeTreePtr<Label> s_label, r_label;
    MakeTreePtr<LabelIdentifier> labelid;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;

    s_comp->members = decls;
    s_comp->statements = (pre, s_goto, s_label, post);
    s_goto->destination = labelid;
    s_label->identifier = labelid;
    
    r_comp->members = decls;
    r_comp->statements = (pre, r_label, post);
    r_label->identifier = labelid;

    Configure( s_comp, r_comp );
}

CleanupUnusedLabels::CleanupUnusedLabels()
{
    // Find a function that contains a compound that contains
    // a label. Use an and-not rule to exclude any usages of the 
    // label. Replace the compound with one that does not have the
    // label. 
    //
    // Usages are detected by searching for the label's identifier
    // using a recurse restriction that prevents recusing through
    // the Label node, thus excluding the declaration which we want
    // to ignore.
    MakeTreePtr<Instance> s_instance, r_instance;
    MakeTreePtr< Stuff<Compound> > stuff;
    MakeTreePtr< Stuff<Compound> > sx_stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr<Label> s_label; // keep l1 and elide l2
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<LabelIdentifier> labelid;
    MakeTreePtr<Goto> sx_goto;
    MakeTreePtr< MatchAll<Compound> > s_andrule;
    MakeTreePtr< NotMatch<Compound> > sx_notrule;
    MakeTreePtr< NotMatch<Node> > sxx_notrule;        
    MakeTreePtr< Label > sxx_label;        
    MakeTreePtr<InstanceIdentifier> identifier;
    MakeTreePtr<Callable> type;

    s_instance->initialiser = s_andrule;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_andrule->patterns = (stuff, sx_notrule);
    s_comp->members = decls;
    s_comp->statements = (pre, s_label, post);
    s_label->identifier = labelid;
                    
    sx_notrule->pattern = sx_stuff;
    sx_stuff->recurse_restriction = sxx_notrule;
    sxx_notrule->pattern = sxx_label;
    sx_stuff->terminus = labelid;
    
    r_instance->initialiser = stuff;
    r_instance->identifier = identifier;
    r_instance->type = type;
    stuff->terminus = overlay;
    overlay->through = s_comp;
    overlay->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, post);
    
    Configure( s_instance, r_instance );
}


CleanUpDeadCode::CleanUpDeadCode()
{
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< NotMatch<Statement> > s_dead_not;
    MakeTreePtr< MatchAny<Statement> > s_dead_any, s_exit_any;
    MakeTreePtr<Case> casee;
    MakeTreePtr<Break> breakk;
     
    s_comp->members = decls;
    s_comp->statements = ( pre, s_exit_any, s_dead_not, post );
    s_exit_any->patterns = (MakeTreePtr<Break>(), MakeTreePtr<Continue>(), MakeTreePtr<Return>(), MakeTreePtr<Goto>());
    s_dead_not->pattern = s_dead_any;
    s_dead_any->patterns = (MakeTreePtr<Case>(), MakeTreePtr<Default>(), MakeTreePtr<Label>());
    r_comp->members = decls;
    r_comp->statements = ( pre, s_exit_any, post );
    
    Configure( s_comp, r_comp );            
}


ReduceVoidCompoundExpression::ReduceVoidCompoundExpression()
{
    MakeTreePtr<CompoundExpression> s_ce;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > stmts;
    MakeTreePtr< NotMatch<Statement> > last;
    MakeTreePtr< TransformOf<Expression> > sx_expr( &TypeOf::instance );
    MakeTreePtr< NotMatch<Type> > sx_type_not;
    MakeTreePtr<Void> sx_void;
    MakeTreePtr<Compound> r_comp;
    
    s_ce->members = (decls);
    s_ce->statements = (stmts, last);
    last->pattern = sx_expr;
    sx_expr->pattern = sx_type_not;
    sx_type_not->pattern = sx_void;
    r_comp->members = (decls);
    r_comp->statements = (stmts, last);
    
    Configure( s_ce, r_comp );      
}


CleanupUnusedVariables::CleanupUnusedVariables()
{
    MakeTreePtr< MatchAll<Scope> > s_all;
    MakeTreePtr<Scope> scope;
    MakeTreePtr< Star<Declaration> > decls;    
    MakeTreePtr<Instance> inst;
    MakeTreePtr<NestedArray> nested_array;
    MakeTreePtr< NotMatch<Type> > sx_not;
    MakeTreePtr< MatchAny<Type> > sx_any;
    MakeTreePtr< TransformOf<TypeIdentifier> > getdecl( &GetDeclaration::instance );
    MakeTreePtr<InstanceIdentifier> id;
    MakeTreePtr< Stuff<Scope> > stuff1, s_stuff2;
    MakeTreePtr< MatchAll<Node> > s_antip;
    MakeTreePtr< AnyNode<Node> > s_anynode;
    MakeTreePtr< NotMatch<Node> > s_nm;
    MakeTreePtr< Erase<Instance> > erase;
    MakeTreePtr<InheritanceRecord> sx_ir;     
    MakeTreePtr< NotMatch<Scope> > s_nscope;
    
    s_all->patterns = (stuff1, s_nscope);
    stuff1->terminus = scope;
    scope->members = (erase, decls);
    erase->erase = inst;
    inst->type = nested_array;
    inst->identifier = id;
    nested_array->terminus = sx_not;
    sx_not->pattern = sx_any;
    sx_any->patterns = ( MakeTreePtr<Callable>(),
                         getdecl );
    getdecl->pattern = sx_ir;
    sx_ir->members = MakeTreePtr< Star<Declaration> >();
    sx_ir->bases = MakeTreePtr< Star<Base> >();
    s_nscope->pattern = s_stuff2;
    s_stuff2->terminus = s_antip;
    s_antip->patterns = (s_anynode, s_nm);
    s_anynode->terminus = id;
    s_nm->pattern = inst;
                        
    Configure( s_all, stuff1 );
}