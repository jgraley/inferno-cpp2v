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
CleanupStatementExpression::CleanupStatementExpression() // LIMITAION: decls in body not allowed
{
     // Lowering compound expressions
     //
     // exp( ({a; b; c; )) ) -> a; b; t=c; exp(t)
     //
     // Temp is used to preserve sequence point after c. This step assumes that
     // all sequence points that need preserving co-incide with the semicolons
     // in a Compound or StatementExpression. It also requires that there be no loops.
     //
     // Everything in a StatementExpression is a Statement, even the final one that
     // gets evaluated (by order of gcc). If this is not an Expression then the type
     // of the StatementExpression is Void. However, this looks like an "evaluate and
     // discard" kind of statement, and we could act on it accordingly (in the case 
     // of a SE ending in another nested SE). So we have to ensure we do outermost 
     // first.
    auto root_stuff = MakePatternPtr< Stuff<Node> >();
    auto root_overlay = MakePatternPtr< Delta<Node> >();
    auto root_not = MakePatternPtr< Negation<Node> >();
    auto root_ce = MakePatternPtr< StatementExpression >();
          
    auto s_all = MakePatternPtr< Conjunction<Statement> >();
    auto sx_pointeris = MakePatternPtr< PointerIs<Statement> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto sx_expr = MakePatternPtr<Expression>();
    
    auto stuff = MakePatternPtr< Stuff<Statement> >();
    auto sr_not = MakePatternPtr< Negation<Statement> >();
    auto sr_comp = MakePatternPtr<SequentialScope>();
    auto sr_cdecls = MakePatternPtr< Star<Declaration> >();
    auto sr_cstmts = MakePatternPtr< Star<Statement> >();
    
    auto s_ce = MakePatternPtr<StatementExpression>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_pre = MakePatternPtr< Star<Statement> >();
    auto s_post = MakePatternPtr< Star<Statement> >();
    auto body = MakePatternPtr< Star<Statement> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto r_temp = MakePatternPtr<Temporary>();
    auto last = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("result");
    auto r_assign = MakePatternPtr<Assign>();
    auto overlay = MakePatternPtr< Delta<Expression> >();
    auto r_type = MakePatternPtr<Type>();

    root_stuff->terminus = root_overlay;
    root_stuff->recurse_restriction = root_not;
    root_not->negand = root_ce;
    root_ce->members = ( MakePatternPtr< Star<Declaration> >() );
    root_ce->statements = ( MakePatternPtr< Star<Statement> >() );
    root_overlay->through = s_all;
    root_overlay->overlay = r_comp;
    
    s_all->conjuncts = (stuff, sx_pointeris);
    sx_pointeris->pointer = sx_not;
    sx_not->negand = sx_expr;
    stuff->recurse_restriction = sr_not;
    sr_not->negand = sr_comp;
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
    
    Configure( COMPARE_REPLACE, root_stuff );
}


// Removing superfluous Compund blocks to clean up the code
CleanupCompoundMulti::CleanupCompoundMulti() // LIMITAION: decls in body not allowed
{
     // {x;{a;b;c}y} -> {x;a;b;c;y}
     // Find a compound block as a statement in another compound block. 
     // Merge the decls and insert the statements in the correct sequence..
    auto s_inner = MakePatternPtr<Compound>();
    auto s_outer = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_pre = MakePatternPtr< Star<Statement> >();
    auto s_post = MakePatternPtr< Star<Statement> >();
    auto s_body = MakePatternPtr< Star<Statement> >();
    auto s_inner_decls = MakePatternPtr< Star<Declaration> >();
    auto s_outer_decls = MakePatternPtr< Star<Declaration> >();

    s_inner->statements = ( s_body );
    s_inner->members = ( s_inner_decls );
    s_outer->statements = ( s_pre, s_inner, s_post );
    s_outer->members = ( s_outer_decls );
    r_comp->statements = ( s_pre, s_body, s_post );
    r_comp->members = ( s_inner_decls, s_outer_decls );

    Configure( SEARCH_REPLACE, s_outer, r_comp );
}


CleanupCompoundSingle::CleanupCompoundSingle() 
{
    // Find a compound block with no decls and one statemewnt. Replace
    // with just the statement
    //
    //{a} -> a TODO need to restrict parent node to Statement: For, If etc OK; Instance is NOT OK
    //         TODO OR maybe just fix renderer for that case
    // Note: this hits eg If(x){a;} which the "Multi" version misses 
    auto all = MakePatternPtr< Conjunction<Statement> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto sx_instance = MakePatternPtr<Instance>();
    auto node = MakePatternPtr< AnyNode<Statement> >();
    auto over = MakePatternPtr< Delta<Statement> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto body = MakePatternPtr< Statement >();

    all->conjuncts = (node, sx_not);
    node->terminus = over;
    sx_not->negand = sx_instance;
    sx_instance->initialiser = s_comp;
    over->through = s_comp;
    over->overlay = body;

    s_comp->statements = body;
    // Note: leaving s_comp empty meaning no decls allowed

    Configure( SEARCH_REPLACE, all );
}

CleanupNop::CleanupNop() 
{
    // Find compound block with Nop in it, replace has the Nop removed.
    // Note: Nop is a no-effect statement, sort-of like ; on its own.
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_nop = MakePatternPtr<Nop>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();

    s_comp->members = decls;
    s_comp->statements = (pre, s_nop, post);
    
    r_comp->members = decls;
    r_comp->statements = (pre, post);

    Configure( SEARCH_REPLACE, s_comp, r_comp );
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
    
    auto s_instance = MakePatternPtr<Instance>();
    auto r_instance = MakePatternPtr<Instance>();
    auto stuff = MakePatternPtr< Stuff<Compound> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_label1 = MakePatternPtr<Label>(); // keep l1 and elide l2
    auto s_label2 = MakePatternPtr<Label>();
    auto r_label1 = MakePatternPtr<Label>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto s_labelid1 = MakePatternPtr<LabelIdentifier>();
    auto s_labelid2 = MakePatternPtr<LabelIdentifier>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("%s_%s", BYPASS_WHEN_IDENTICAL);
    auto l_s_orrule = MakePatternPtr< Disjunction<LabelIdentifier> >();
    auto identifier = MakePatternPtr<InstanceIdentifier>();
    auto type = MakePatternPtr<Callable>();
    
    l_s_orrule->disjuncts = (s_labelid1, s_labelid2);
    
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Compound> >( stuff, l_s_orrule, r_labelid );
    
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
    
    Configure( SEARCH_REPLACE, s_instance, r_instance );
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
    
    auto s_instance = MakePatternPtr<Instance>();
    auto r_instance = MakePatternPtr<Instance>();
    auto stuff = MakePatternPtr< Stuff<Compound> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_label = MakePatternPtr<Label>(); // keep l1 and elide l2
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto s_labelid1 = MakePatternPtr<LabelIdentifier>();
    auto s_labelid2 = MakePatternPtr<LabelIdentifier>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("%s_%s", BYPASS_WHEN_IDENTICAL);
    auto l_s_orrule = MakePatternPtr< Disjunction<LabelIdentifier> >();
    auto identifier = MakePatternPtr<InstanceIdentifier>();
    auto type = MakePatternPtr<Callable>();
    auto s_goto = MakePatternPtr<Goto>();
    auto r_goto = MakePatternPtr<Goto>();
    
    l_s_orrule->disjuncts = (s_labelid1, s_labelid2);
    
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Compound> >( stuff, l_s_orrule, r_labelid );
    
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
    
    Configure( SEARCH_REPLACE, s_instance, r_instance );
}

CleanupIneffectualGoto::CleanupIneffectualGoto()
{
    // Find a compound containing a Goto and a Label where the 
    // goto goes to the label. Remove the Goto (but not the Label
    // since there may be other Gotos to it).
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_goto = MakePatternPtr<Goto>();
    auto s_label = MakePatternPtr<Label>();
    auto r_label = MakePatternPtr<Label>();
    auto labelid = MakePatternPtr<LabelIdentifier>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();

    s_comp->members = decls;
    s_comp->statements = (pre, s_goto, s_label, post);
    s_goto->destination = labelid;
    s_label->identifier = labelid;
    
    r_comp->members = decls;
    r_comp->statements = (pre, r_label, post);
    r_label->identifier = labelid;

    Configure( SEARCH_REPLACE, s_comp, r_comp );
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
    auto s_instance = MakePatternPtr<Instance>();
    auto r_instance = MakePatternPtr<Instance>();
    auto stuff = MakePatternPtr< Stuff<Compound> >();
    auto sx_stuff = MakePatternPtr< Stuff<Compound> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto s_label = MakePatternPtr<Label>(); // keep l1 and elide l2
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto labelid = MakePatternPtr<LabelIdentifier>();
    auto sx_goto = MakePatternPtr<Goto>();
    auto s_andrule = MakePatternPtr< Conjunction<Compound> >();
    auto sx_notrule = MakePatternPtr< Negation<Compound> >();
    auto sxx_notrule = MakePatternPtr< Negation<Node> >();
    auto sxx_label = MakePatternPtr< Label >();
    auto identifier = MakePatternPtr<InstanceIdentifier>();
    auto type = MakePatternPtr<Callable>();

    s_instance->initialiser = s_andrule;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_andrule->conjuncts = (stuff, sx_notrule);
    s_comp->members = decls;
    s_comp->statements = (pre, s_label, post);
    s_label->identifier = labelid;
                    
    sx_notrule->negand = sx_stuff;
    sx_stuff->recurse_restriction = sxx_notrule;
    sxx_notrule->negand = sxx_label;
    sx_stuff->terminus = labelid;
    
    r_instance->initialiser = stuff;
    r_instance->identifier = identifier;
    r_instance->type = type;
    stuff->terminus = overlay;
    overlay->through = s_comp;
    overlay->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, post);
    
    Configure( SEARCH_REPLACE, s_instance, r_instance );
}


CleanUpDeadCode::CleanUpDeadCode()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto s_dead_not = MakePatternPtr< Negation<Statement> >();
    auto s_dead_any = MakePatternPtr< Disjunction<Statement> >();
    auto s_exit_any = MakePatternPtr< Disjunction<Statement> >();
    auto casee = MakePatternPtr<Case>();
    auto breakk = MakePatternPtr<Break>();
     
    s_comp->members = decls;
    s_comp->statements = ( pre, s_exit_any, s_dead_not, post );
    s_exit_any->disjuncts = (MakePatternPtr<Break>(), MakePatternPtr<Continue>(), MakePatternPtr<Return>(), MakePatternPtr<Goto>());
    s_dead_not->negand = s_dead_any;
    s_dead_any->disjuncts = (MakePatternPtr<Case>(), MakePatternPtr<Default>(), MakePatternPtr<Label>());
    r_comp->members = decls;
    r_comp->statements = ( pre, s_exit_any, post );
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );            
}


ReduceVoidStatementExpression::ReduceVoidStatementExpression()
{
    auto s_ce = MakePatternPtr<StatementExpression>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto last = MakePatternPtr< Negation<Statement> >();
    auto sx_expr = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto sx_type_not = MakePatternPtr< Negation<Type> >();
    auto sx_void = MakePatternPtr<Void>();
    auto r_comp = MakePatternPtr<Compound>();
    
    s_ce->members = (decls);
    s_ce->statements = (stmts, last);
    last->negand = sx_expr;
    sx_expr->pattern = sx_type_not;
    sx_type_not->negand = sx_void;
    r_comp->members = (decls);
    r_comp->statements = (stmts, last);
    
    Configure( SEARCH_REPLACE, s_ce, r_comp );      
}


CleanupUnusedVariables::CleanupUnusedVariables()
{
    auto s_all = MakePatternPtr< Conjunction<Scope> >();
    auto s_scope = MakePatternPtr<Scope>();
    auto r_scope = MakePatternPtr<Scope>();
    auto over_scope = MakePatternPtr< Delta<Scope> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto inst = MakePatternPtr<Instance>();
    auto nested_array = MakePatternPtr<NestedArrayAgent>();
    auto sx_not = MakePatternPtr< Negation<Type> >();
    auto sx_any = MakePatternPtr< Disjunction<Type> >();
    auto getdecl = MakePatternPtr< TransformOf<TypeIdentifier> >( &GetDeclaration::instance ); // TODO should be modulo typedefs
    auto id = MakePatternPtr<InstanceIdentifier>();
    auto stuff1 = MakePatternPtr< Stuff<Scope> >();
    auto s_stuff2 = MakePatternPtr< Stuff<Scope> >();
    auto s_antip = MakePatternPtr< Conjunction<Node> >();
    auto s_anynode = MakePatternPtr< AnyNode<Node> >();
    auto s_nm = MakePatternPtr< Negation<Node> >();
    auto sx_ir = MakePatternPtr<InheritanceRecord>();
    auto s_nscope = MakePatternPtr< Negation<Scope> >();
    
    s_all->conjuncts = (stuff1, s_nscope);
    stuff1->terminus = over_scope;
    over_scope->through = s_scope;
    over_scope->overlay = r_scope;
    s_scope->members = (inst, decls);
    r_scope->members = (decls);
    inst->type = nested_array;
    inst->identifier = id;
    nested_array->terminus = sx_not;
    sx_not->negand = sx_any;
    sx_any->disjuncts = ( MakePatternPtr<Callable>(),
                         getdecl );
    getdecl->pattern = sx_ir;
    sx_ir->members = MakePatternPtr< Star<Declaration> >();
    sx_ir->bases = MakePatternPtr< Star<Base> >();
    s_nscope->negand = s_stuff2;
    s_stuff2->terminus = s_antip;
    s_antip->conjuncts = (s_anynode, s_nm);
    s_anynode->terminus = id;
    s_nm->negand = inst;
                        
    Configure( COMPARE_REPLACE, s_all, stuff1 );
}


CleanupNestedIf::CleanupNestedIf()
{
    auto s_outer_if = MakePatternPtr<If>();
    auto s_inner_if = MakePatternPtr<If>();
    auto r_if = MakePatternPtr<If>();
    auto body = MakePatternPtr<Statement>();
    auto s_inner_nop = MakePatternPtr<Nop>();
    auto s_outer_nop = MakePatternPtr<Nop>();
    auto r_nop = MakePatternPtr<Nop>();
    auto inner_cond = MakePatternPtr<Expression>();
    auto outer_cond = MakePatternPtr<Expression>();
    auto r_and = MakePatternPtr<LogicalAnd>();
    
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
    
    Configure( SEARCH_REPLACE, s_outer_if, r_if );
}
