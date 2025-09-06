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
    auto root_stuff = MakePatternNode< Stuff<Node> >();
    auto root_overlay = MakePatternNode< Delta<Node> >();
    auto root_not = MakePatternNode< Negation<Node> >();
    auto root_ce = MakePatternNode< StatementExpression >();
          
    auto s_all = MakePatternNode< Conjunction<Statement> >();
    auto sx_pointeris = MakePatternNode< PointerIs<Statement> >();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    auto sx_expr = MakePatternNode<Expression>();
    
    auto stuff = MakePatternNode< Stuff<Statement> >();
    auto sr_not = MakePatternNode< Negation<Statement> >();
    auto sr_comp = MakePatternNode<SequentialScope>();
    auto sr_cdecls = MakePatternNode< Star<Declaration> >();
    auto sr_cstmts = MakePatternNode< Star<Statement> >();
    
    auto s_ce = MakePatternNode<StatementExpression>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_pre = MakePatternNode< Star<Statement> >();
    auto s_post = MakePatternNode< Star<Statement> >();
    auto body = MakePatternNode< Star<Statement> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto r_temp = MakePatternNode<Temporary>();
    auto last = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("result");
    auto r_assign = MakePatternNode<Assign>();
    auto overlay = MakePatternNode< Delta<Expression> >();
    auto r_type = MakePatternNode<Type>();

    root_stuff->terminus = root_overlay;
    root_stuff->recurse_restriction = root_not;
    root_not->negand = root_ce;
    root_ce->members = ( MakePatternNode< Star<Declaration> >() );
    root_ce->statements = ( MakePatternNode< Star<Statement> >() );
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
    r_temp->initialiser = MakePatternNode<Uninitialised>();
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
    auto s_inner = MakePatternNode<Compound>();
    auto s_outer = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_pre = MakePatternNode< Star<Statement> >();
    auto s_post = MakePatternNode< Star<Statement> >();
    auto s_body = MakePatternNode< Star<Statement> >();
    auto s_inner_decls = MakePatternNode< Star<Declaration> >();
    auto s_outer_decls = MakePatternNode< Star<Declaration> >();

    s_inner->statements = ( s_body );
    s_inner->members = ( s_inner_decls );
    s_outer->statements = ( s_pre, s_inner, s_post );
    s_outer->members = ( s_outer_decls );
    r_comp->statements = ( s_pre, s_body, s_post );
    r_comp->members = ( s_inner_decls, s_outer_decls );

    Configure( SEARCH_REPLACE, s_outer, r_comp );
}

// Act on all Compound with single Statement. Won't work because
// Compound is an Initialiser while Statement is not.
//#define CLEANUP_COMP_SINGLE_STRONG

CleanupCompoundSingle::CleanupCompoundSingle() 
{
    // Find a compound block with no decls and one statemewnt. Replace
    // with just the statement
    //
    //{a} -> a TODO need to restrict parent node to Statement: For, If etc OK; Instance is NOT OK
    //         TODO OR maybe just fix renderer for that case
    // Note: this hits eg If(x){a;} which the "Multi" version misses 
    auto all = MakePatternNode< Conjunction<Node> >();
    auto sx_not = MakePatternNode< Negation<Node> >();
    auto sx_instance = MakePatternNode<Instance>();
    auto node = MakePatternNode< Child<Node> >();
    auto over = MakePatternNode< Delta<Node> >();
    auto s_comp = MakePatternNode<Compound>();
    auto body = MakePatternNode< Statement >();

    all->conjuncts = (node, sx_not);
    node->terminus = over;
    sx_not->negand = sx_instance;
    sx_instance->initialiser = s_comp;
    over->through = s_comp;
    over->overlay = body;

    s_comp->statements = body;
    // Note: leaving s_comp empty meaning no decls allowed

#ifdef CLEANUP_COMP_SINGLE_STRONG
    Configure( SEARCH_REPLACE, s_comp, body );
#else    
    Configure( SEARCH_REPLACE, all );
#endif    
}

CleanupNop::CleanupNop() 
{
    // Find compound block with Nop in it, replace has the Nop removed.
    // Note: Nop is a no-effect statement, sort-of like ; on its own.
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_nop = MakePatternNode<Nop>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();

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
    // Using an embedded pattern, find references to either one of the original labels 
    // and replace by a reference to the new label.
    //
    // Notes:
    // - The embedded pattern must operate over the entire function, not just the 
    // compound that containes the labels, because labels have function 
    // scope and the gotos can be anywhere.
    // - Do not assume the usages of the labels will be gotos. We support
    // GCCs goto-a-variable extension in which case a label could be 
    // on the right of an assignment.
    
    auto s_instance = MakePatternNode<Instance>();
    auto r_instance = MakePatternNode<Instance>();
    auto stuff = MakePatternNode< Stuff<Compound> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_label1 = MakePatternNode<Label>(); // keep l1 and elide l2
    auto s_label2 = MakePatternNode<Label>();
    auto r_label1 = MakePatternNode<Label>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto s_labelid1 = MakePatternNode<LabelIdentifier>();
    auto s_labelid2 = MakePatternNode<LabelIdentifier>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("%s_%s", BYPASS_WHEN_IDENTICAL);
    auto l_s_orrule = MakePatternNode< Disjunction<LabelIdentifier> >();
    auto identifier = MakePatternNode<InstanceIdentifier>();
    auto type = MakePatternNode<Callable>();
    
    l_s_orrule->disjuncts = (s_labelid1, s_labelid2);
    
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Compound> >( stuff, l_s_orrule, r_labelid );
    
    s_instance->initialiser = stuff;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_comp->members = decls;
    s_comp->statements = ( pre, s_label1, s_label2, post );
    s_label1->identifier = s_labelid1;
    s_label2->identifier = s_labelid2;
    
    r_instance->initialiser = r_embedded;
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
    // Using an embedded pattern, find references to either one of the original labels 
    // and replace by a reference to a new merged one.
    //
    // Notes:
    // - The embedded pattern must operate over the entire function, not just the 
    // compound that containes the labels, because labels have function 
    // scope and the gotos can be anywhere.
    // - Do not assume the usages of the labels will be gotos. We support
    // GCCs goto-a-variable extension in which case a label could be 
    // on the right of an assignment.
    
    auto s_instance = MakePatternNode<Instance>();
    auto r_instance = MakePatternNode<Instance>();
    auto stuff = MakePatternNode< Stuff<Compound> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_label = MakePatternNode<Label>(); // keep l1 and elide l2
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto s_labelid1 = MakePatternNode<LabelIdentifier>();
    auto s_labelid2 = MakePatternNode<LabelIdentifier>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("%s_%s", BYPASS_WHEN_IDENTICAL);
    auto l_s_orrule = MakePatternNode< Disjunction<LabelIdentifier> >();
    auto identifier = MakePatternNode<InstanceIdentifier>();
    auto type = MakePatternNode<Callable>();
    auto s_goto = MakePatternNode<Goto>();
    auto r_goto = MakePatternNode<Goto>();
    
    l_s_orrule->disjuncts = (s_labelid1, s_labelid2);
    
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Compound> >( stuff, l_s_orrule, r_labelid );
    
    s_instance->initialiser = stuff;
    s_instance->identifier = identifier;
    s_instance->type = type;
    s_comp->members = decls;
    s_comp->statements = (pre, s_label, s_goto, post);
    s_label->identifier = s_labelid1;
    s_goto->destination = s_labelid2;
    
    r_instance->initialiser = r_embedded;
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
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_goto = MakePatternNode<Goto>();
    auto s_label = MakePatternNode<Label>();
    auto r_label = MakePatternNode<Label>();
    auto labelid = MakePatternNode<LabelIdentifier>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();

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
    auto s_instance = MakePatternNode<Instance>();
    auto r_instance = MakePatternNode<Instance>();
    auto stuff = MakePatternNode< Stuff<Compound> >();
    auto sx_stuff = MakePatternNode< Stuff<Compound> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto s_label = MakePatternNode<Label>(); // keep l1 and elide l2
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto labelid = MakePatternNode<LabelIdentifier>();
    auto sx_goto = MakePatternNode<Goto>();
    auto s_andrule = MakePatternNode< Conjunction<Compound> >();
    auto sx_notrule = MakePatternNode< Negation<Compound> >();
    auto sxx_notrule = MakePatternNode< Negation<Node> >();
    auto sxx_label = MakePatternNode< Label >();
    auto identifier = MakePatternNode<InstanceIdentifier>();
    auto type = MakePatternNode<Callable>();

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
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto s_dead_not = MakePatternNode< Negation<Statement> >();
    auto s_dead_any = MakePatternNode< Disjunction<Statement> >();
    auto s_exit_any = MakePatternNode< Disjunction<Statement> >();
    auto casee = MakePatternNode<Case>();
    auto breakk = MakePatternNode<Break>();
     
    s_comp->members = decls;
    s_comp->statements = ( pre, s_exit_any, s_dead_not, post );
    s_exit_any->disjuncts = (MakePatternNode<Break>(), MakePatternNode<Continue>(), MakePatternNode<Return>(), MakePatternNode<Goto>());
    s_dead_not->negand = s_dead_any;
    s_dead_any->disjuncts = (MakePatternNode<Case>(), MakePatternNode<Default>(), MakePatternNode<Label>());
    r_comp->members = decls;
    r_comp->statements = ( pre, s_exit_any, post );
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );            
}


CleanupVoidStatementExpression::CleanupVoidStatementExpression()
{
    auto s_ce = MakePatternNode<StatementExpression>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto last = MakePatternNode< Negation<Statement> >();
    auto sx_expr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto sx_type_not = MakePatternNode< Negation<Type> >();
    auto sx_void = MakePatternNode<Void>();
    auto r_comp = MakePatternNode<Compound>();
    
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
    auto s_all = MakePatternNode< Conjunction<Scope> >();
    auto s_scope = MakePatternNode<Scope>();
    auto r_scope = MakePatternNode<Scope>();
    auto over_scope = MakePatternNode< Delta<Scope> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto inst = MakePatternNode<Instance>();
    auto nested_array = MakePatternNode<NestedArrayAgent>();
    auto sx_not = MakePatternNode< Negation<Type> >();
    auto sx_any = MakePatternNode< Disjunction<Type> >();
    auto getdecl = MakePatternNode< TransformOf<TypeIdentifier> >( &DeclarationOf::instance ); // TODO should be modulo typedefs
    auto id = MakePatternNode<InstanceIdentifier>();
    auto stuff1 = MakePatternNode< Stuff<Scope> >();
    auto s_stuff2 = MakePatternNode< Stuff<Scope> >();
    auto s_antip = MakePatternNode< Conjunction<Node> >();
    auto s_anynode = MakePatternNode< Child<Node> >();
    auto s_nm = MakePatternNode< Negation<Node> >();
    auto sx_ir = MakePatternNode<InheritanceRecord>();
    auto s_nscope = MakePatternNode< Negation<Scope> >();
    
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
    sx_any->disjuncts = ( MakePatternNode<Callable>(),
                         getdecl );
    getdecl->pattern = sx_ir;
    sx_ir->members = MakePatternNode< Star<Declaration> >();
    sx_ir->bases = MakePatternNode< Star<Base> >();
    s_nscope->negand = s_stuff2;
    s_stuff2->terminus = s_antip;
    s_antip->conjuncts = (s_anynode, s_nm);
    s_anynode->terminus = id;
    s_nm->negand = inst;
                        
    Configure( COMPARE_REPLACE, s_all, stuff1 );
}


CleanupNestedIf::CleanupNestedIf()
{
    auto s_outer_if = MakePatternNode<If>();
    auto s_inner_if = MakePatternNode<If>();
    auto r_if = MakePatternNode<If>();
    auto body = MakePatternNode<Statement>();
    auto s_inner_nop = MakePatternNode<Nop>();
    auto s_outer_nop = MakePatternNode<Nop>();
    auto r_nop = MakePatternNode<Nop>();
    auto inner_cond = MakePatternNode<Expression>();
    auto outer_cond = MakePatternNode<Expression>();
    auto r_and = MakePatternNode<LogicalAnd>();
    
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
