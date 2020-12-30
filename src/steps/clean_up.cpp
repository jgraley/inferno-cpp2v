/*
 * clean_up.cpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#include "steps/clean_up.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_agents.hpp"

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
     // in a Compound or CompoundExpression. It also requires that there be no loops.
     //
     // Everything in a CompoundExpression is a Statement, even the final one that
     // gets evaluated (by order of gcc). If this is not an Expression then the type
     // of the CompoundExpression is Void. However, this looks like an "evaluate and
     // discard" kind of statement, and we could act on it accordingly (in the case 
     // of a SE ending in another nested SE). So we have to ensure we do outermost 
     // first.
    MakePatternPtr< Stuff<Node> > root_stuff;
    MakePatternPtr< Overlay<Node> > root_overlay;
    MakePatternPtr< Negation<Node> > root_not;
    MakePatternPtr< CompoundExpression > root_ce;
          
    MakePatternPtr< Conjunction<Statement> > s_all;
    MakePatternPtr< PointerIs<Statement> > sx_pointeris;
    MakePatternPtr< Negation<Statement> > sx_not;
    MakePatternPtr<Expression> sx_expr;
    
    MakePatternPtr< Stuff<Statement> > stuff;
    MakePatternPtr< Negation<Statement> > sr_not;
    MakePatternPtr<SequentialScope> sr_comp;
    MakePatternPtr< Star<Declaration> > sr_cdecls;
    MakePatternPtr< Star<Statement> > sr_cstmts;
    
    MakePatternPtr<CompoundExpression> s_ce;
    MakePatternPtr<Compound> r_comp;
    MakePatternPtr< Star<Statement> > s_pre, s_post, body;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr< TransformOf<Expression> > last( &TypeOf::instance );
    MakePatternPtr<BuildInstanceIdentifierAgent> r_temp_id("result");
    MakePatternPtr<Assign> r_assign;
    MakePatternPtr< Overlay<Expression> > overlay;
    MakePatternPtr<Type> r_type;

    root_stuff->terminus = root_overlay;
    root_stuff->recurse_restriction = root_not;
    root_not->pattern = root_ce;
    root_ce->members = ( MakePatternPtr< Star<Declaration> >() );
    root_ce->statements = ( MakePatternPtr< Star<Statement> >() );
    root_overlay->through = s_all;
    root_overlay->overlay = r_comp;
    
    s_all->patterns = (stuff, sx_pointeris);
    sx_pointeris->pointer = sx_not;
    sx_not->pattern = sx_expr;
    stuff->recurse_restriction = sr_not;
    sr_not->pattern = sr_comp;
    sr_comp->members = sr_cdecls;
    sr_comp->statements = sr_cstmts;
    
    stuff->terminus = overlay;
    overlay->through = s_ce;
    s_ce->members = ( decls );
    s_ce->statements = ( body, last );
    
    r_comp->statements = ( body, r_assign, stuff );
    r_comp->members = ( decls, r_temp );
    r_temp->identifier = r_temp_id;
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_temp->type = r_type;
    r_assign->operands = (r_temp_id, last);
    last->pattern = r_type;
    overlay->overlay = r_temp_id;        
    
    Configure( root_stuff );
}


// Removing superfluous Compund blocks to clean up the code
CleanupCompoundMulti::CleanupCompoundMulti() // LIMITAION: decls in body not allowed
{
     // {x;{a;b;c}y} -> {x;a;b;c;y}
     // Find a compound block as a statement in another compound block. 
     // Merge the decls and insert the statements in the correct sequence..
    MakePatternPtr<Compound> s_inner, s_outer, r_comp;
    MakePatternPtr< Star<Statement> > s_pre, s_post, s_body;
    MakePatternPtr< Star<Declaration> > s_inner_decls, s_outer_decls;

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
    MakePatternPtr< Conjunction<Statement> > all;    
    MakePatternPtr< Negation<Statement> > sx_not;
    MakePatternPtr<Instance> sx_instance;
    MakePatternPtr< AnyNode<Statement> > node;
    MakePatternPtr< Overlay<Statement> > over;   
    MakePatternPtr<Compound> s_comp;
    MakePatternPtr< Statement > body;

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
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr<Nop> s_nop;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;

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
    
    MakePatternPtr<Instance> s_instance, r_instance;
    MakePatternPtr< Stuff<Compound> > stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr<Label> s_label1, s_label2, r_label1; // keep l1 and elide l2
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<LabelIdentifier> s_labelid1, s_labelid2;
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("%s_%s", BYPASS_WHEN_IDENTICAL);
    MakePatternPtr< Disjunction<LabelIdentifier> > l_s_orrule;
    MakePatternPtr<InstanceIdentifier> identifier;
    MakePatternPtr<Callable> type;
    
    l_s_orrule->patterns = (s_labelid1, s_labelid2);
    
    MakePatternPtr< SlaveSearchReplace<Compound> > r_slave( stuff, l_s_orrule, r_labelid );
    
    s_instance->initialiser = stuff;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_comp->members = decls;
    s_comp->statements = ( pre, s_label1, s_label2, post );
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
    
    MakePatternPtr<Instance> s_instance, r_instance;
    MakePatternPtr< Stuff<Compound> > stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr<Label> s_label; // keep l1 and elide l2
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<LabelIdentifier> s_labelid1, s_labelid2;
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("%s_%s", BYPASS_WHEN_IDENTICAL);
    MakePatternPtr< Disjunction<LabelIdentifier> > l_s_orrule;
    MakePatternPtr<InstanceIdentifier> identifier;
    MakePatternPtr<Callable> type;
    MakePatternPtr<Goto> s_goto, r_goto;
    
    l_s_orrule->patterns = (s_labelid1, s_labelid2);
    
    MakePatternPtr< SlaveSearchReplace<Compound> > r_slave( stuff, l_s_orrule, r_labelid );
    
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
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr<Goto> s_goto;
    MakePatternPtr<Label> s_label, r_label;
    MakePatternPtr<LabelIdentifier> labelid;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;

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
    MakePatternPtr<Instance> s_instance, r_instance;
    MakePatternPtr< Stuff<Compound> > stuff;
    MakePatternPtr< Stuff<Compound> > sx_stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr<Label> s_label; // keep l1 and elide l2
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<LabelIdentifier> labelid;
    MakePatternPtr<Goto> sx_goto;
    MakePatternPtr< Conjunction<Compound> > s_andrule;
    MakePatternPtr< Negation<Compound> > sx_notrule;
    MakePatternPtr< Negation<Node> > sxx_notrule;        
    MakePatternPtr< Label > sxx_label;        
    MakePatternPtr<InstanceIdentifier> identifier;
    MakePatternPtr<Callable> type;

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
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< Negation<Statement> > s_dead_not;
    MakePatternPtr< Disjunction<Statement> > s_dead_any, s_exit_any;
    MakePatternPtr<Case> casee;
    MakePatternPtr<Break> breakk;
     
    s_comp->members = decls;
    s_comp->statements = ( pre, s_exit_any, s_dead_not, post );
    s_exit_any->patterns = (MakePatternPtr<Break>(), MakePatternPtr<Continue>(), MakePatternPtr<Return>(), MakePatternPtr<Goto>());
    s_dead_not->pattern = s_dead_any;
    s_dead_any->patterns = (MakePatternPtr<Case>(), MakePatternPtr<Default>(), MakePatternPtr<Label>());
    r_comp->members = decls;
    r_comp->statements = ( pre, s_exit_any, post );
    
    Configure( s_comp, r_comp );            
}


ReduceVoidCompoundExpression::ReduceVoidCompoundExpression()
{
    MakePatternPtr<CompoundExpression> s_ce;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > stmts;
    MakePatternPtr< Negation<Statement> > last;
    MakePatternPtr< TransformOf<Expression> > sx_expr( &TypeOf::instance );
    MakePatternPtr< Negation<Type> > sx_type_not;
    MakePatternPtr<Void> sx_void;
    MakePatternPtr<Compound> r_comp;
    
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
    MakePatternPtr< Conjunction<Scope> > s_all;
    MakePatternPtr<Scope> s_scope, r_scope;
    MakePatternPtr< Overlay<Scope> > over_scope; 
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr<Instance> inst;
    MakePatternPtr<NestedArrayAgent> nested_array;
    MakePatternPtr< Negation<Type> > sx_not;
    MakePatternPtr< Disjunction<Type> > sx_any;
    MakePatternPtr< TransformOf<TypeIdentifier> > getdecl( &GetDeclaration::instance ); // TODO should be modulo typedefs
    MakePatternPtr<InstanceIdentifier> id;
    MakePatternPtr< Stuff<Scope> > stuff1, s_stuff2;
    MakePatternPtr< Conjunction<Node> > s_antip;
    MakePatternPtr< AnyNode<Node> > s_anynode;
    MakePatternPtr< Negation<Node> > s_nm;
    MakePatternPtr<InheritanceRecord> sx_ir;     
    MakePatternPtr< Negation<Scope> > s_nscope;
    
    s_all->patterns = (stuff1, s_nscope);
    stuff1->terminus = over_scope;
    over_scope->through = s_scope;
    over_scope->overlay = r_scope;
    s_scope->members = (inst, decls);
    r_scope->members = (decls);
    inst->type = nested_array;
    inst->identifier = id;
    nested_array->terminus = sx_not;
    sx_not->pattern = sx_any;
    sx_any->patterns = ( MakePatternPtr<Callable>(),
                         getdecl );
    getdecl->pattern = sx_ir;
    sx_ir->members = MakePatternPtr< Star<Declaration> >();
    sx_ir->bases = MakePatternPtr< Star<Base> >();
    s_nscope->pattern = s_stuff2;
    s_stuff2->terminus = s_antip;
    s_antip->patterns = (s_anynode, s_nm);
    s_anynode->terminus = id;
    s_nm->pattern = inst;
                        
    Configure( s_all, stuff1 );
}


CleanupNestedIf::CleanupNestedIf()
{
    MakePatternPtr<If> s_outer_if, s_inner_if, r_if;
    MakePatternPtr<Statement> body;
    MakePatternPtr<Nop> s_inner_nop, s_outer_nop, r_nop;
    MakePatternPtr<Expression> inner_cond, outer_cond;
    MakePatternPtr<LogicalAnd> r_and;
    
    s_outer_if->condition = outer_cond;
    s_outer_if->body = s_inner_if;
    s_outer_if->else_body = s_outer_nop;
    s_inner_if->condition = inner_cond;
    s_inner_if->body = body;
    s_inner_if->else_body = s_inner_nop;
    
    r_if->condition = r_and;
    r_if->body = body; 
    r_if->else_body = r_nop;
    r_and->operands = (outer_cond, inner_cond); // outer first, to be side-effect correct
    
    Configure( s_outer_if, r_if );
}
