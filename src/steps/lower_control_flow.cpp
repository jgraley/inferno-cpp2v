
#include "steps/lower_control_flow.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"
#include "steps/uncombable.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;

// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : Switch, Uncombable { NODE_FUNCTIONS_FINAL };
struct UncombableFor : For, Uncombable { NODE_FUNCTIONS_FINAL };
struct CombableFor : For { NODE_FUNCTIONS_FINAL };
   

DetectUncombableSwitch::DetectUncombableSwitch()
{
    MakeTreePtr< MatchAll<Switch> > s_all;
    MakeTreePtr< NotMatch<Switch> > sx_not;
    MakeTreePtr<UncombableSwitch> sx_uswitch;
    MakeTreePtr<Switch> s_switch;
    MakeTreePtr<Expression> expr;
    MakeTreePtr<Compound> comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< NotMatch<Statement> > x_not;
    MakeTreePtr<Break> x_break;
    MakeTreePtr<SwitchTarget> target;
    MakeTreePtr<UncombableSwitch> r_uswitch;
    
    s_all->patterns = (sx_not, s_switch);
    sx_not->pattern = sx_uswitch;
    s_switch->body = comp;
    s_switch->condition = expr;
    comp->members = decls;
    comp->statements = (pre, x_not, target, post);
    x_not->pattern = x_break;
    
    r_uswitch->body = comp;
    r_uswitch->condition = expr;
    
    Configure( s_all, r_uswitch );
}


// Turn all for into uncombablefor, so the next step can go the other
// way, and can avoid a top-level NOT
MakeAllForUncombable::MakeAllForUncombable()
{
    MakeTreePtr< MatchAll<For> > s_all;
    MakeTreePtr< NotMatch<For> > s_not;
    MakeTreePtr<UncombableFor> sx_ufor;
    MakeTreePtr<For> s_for;
    MakeTreePtr<Statement> init;
    MakeTreePtr<Expression> test; 
    MakeTreePtr<Statement> inc; 
    MakeTreePtr<Statement> body;
    MakeTreePtr<UncombableFor> r_ufor;
    
    s_all->patterns = (s_not, s_for);
    s_not->pattern = sx_ufor;
    s_for->initialisation = init;
    s_for->condition = test;
    s_for->increment = inc;
    s_for->body = body;
    
    r_ufor->initialisation = init;
    r_ufor->condition = test;
    r_ufor->increment = inc;
    r_ufor->body = body;
    
    Configure( s_all, r_ufor );
}


DetectCombableFor::DetectCombableFor()
{
    MakeTreePtr<UncombableFor> s_ufor;
    MakeTreePtr<Assign> init;
    MakeTreePtr<Less> test; // TODO or <= or !=
    MakeTreePtr<PostIncrement> inc; // TODO or pre-inc or maybe +=n
    MakeTreePtr< NotMatch<Statement> > body, bnot, cnot;
    MakeTreePtr< MatchAny<Statement> > any;
    MakeTreePtr< Stuff<Statement> > astuff, bstuff, cstuff;
    MakeTreePtr<AssignmentOperator> assign;
    MakeTreePtr<Break> brk;
    MakeTreePtr<Continue> cont;
    
    MakeTreePtr<CombableFor> r_for;
    MakeTreePtr< TransformOf<InstanceIdentifier> > loopvar( &TypeOf::instance );
    MakeTreePtr<Integral> type;
    
    s_ufor->initialisation = init;
    init->operands = (loopvar, MakeTreePtr<Integer>());
    s_ufor->condition = test;
    test->operands = (loopvar, MakeTreePtr<Integer>());
    s_ufor->increment = inc;
    inc->operands = (loopvar);
    s_ufor->body = body;
    body->pattern = any;
    any->patterns = (astuff, bstuff, cstuff);
    astuff->terminus = assign;
    bstuff->terminus = brk;
    bstuff->recurse_restriction = bnot;
    bnot->pattern = MakeTreePtr<Breakable>(); // don't "see" somebody else's break
    cstuff->terminus = cont;
    cstuff->recurse_restriction = cnot;
    cnot->pattern = MakeTreePtr<Loop>(); // don't "see" somebody else's continue
    
    assign->operands = (loopvar, MakeTreePtr< Star<Expression> >());
    loopvar->pattern = type;
    
    r_for->initialisation = init;
    r_for->condition = test;
    r_for->increment = inc;
    r_for->body = body;
    
    Configure( s_ufor, r_for );
}


ForToWhile::ForToWhile()
{
    // Add new compunds inside and outside the While node, and place the For
    // node's statements and expressions in the appropriate places.
    //
    // Deal with continue by inserting a copy of the increment just before the 
    // continue, and leave the continue in place. This avoids the need for 
    // gotos in case that matters. Not ideal if the increment statement 
    // is huge. 
    //
    // Avoid matching continues that don't belong to use by using a recurse 
    // restriction, like in BreakToGoto.
    //
    // We have to use the GreenGrass hack to prevent the slave spinning 
    // forever. The continue in the slave search pattern has a GreenGrass
    // node under it indicating that it must be from the input program and
    // not one we just inserted on an earlier iteration.
    
    MakeTreePtr<For> s_for;
    MakeTreePtr<Statement> forbody, inc, init;
    MakeTreePtr<Expression> cond;
    MakeTreePtr<While> r_while;
    MakeTreePtr<Compound> r_outer, r_body;
    MakeTreePtr< Stuff<Statement> > l_stuff;
    MakeTreePtr< Overlay<Statement> > l_overlay;
    MakeTreePtr< NotMatch<Statement> > l_s_not;
    MakeTreePtr< Loop > l_s_loop;
    
    MakeTreePtr<Continue> l_s_cont;
    MakeTreePtr<Nop> l_r_nop;
    MakeTreePtr<BuildLabelIdentifier> r_cont_labelid("CONTINUE");
    MakeTreePtr<Label> r_cont_label;
    MakeTreePtr<Goto> lr_goto;

    l_stuff->terminus = l_overlay;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = lr_goto;
    lr_goto->destination = r_cont_labelid;
    l_s_not->pattern = l_s_loop;
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave( forbody, l_stuff, l_stuff );
    
    s_for->body = forbody;
    s_for->initialisation = init;
    s_for->condition = cond;
    s_for->increment = inc;

    r_outer->statements = (init, r_while);
    r_while->body = r_body;
    r_while->condition = cond;
    r_body->statements = (r_slave, r_cont_label, inc);
    r_cont_label->identifier = r_cont_labelid;

    Configure( MakeCheckUncombable(s_for), r_outer );
}

WhileToDo::WhileToDo()
{
    // Just need to insert an "if" statement for the case 
    // where there are 0 iterations.
    MakeTreePtr<While> s_while;
    MakeTreePtr<Statement> body;
    MakeTreePtr<Expression> cond;
    MakeTreePtr<Nop> r_nop;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Do> r_do;

    s_while->body = body;
    s_while->condition = cond;

    r_if->condition = cond;
    r_if->body = r_do;
    r_if->else_body = r_nop;
    r_do->condition = cond;
    r_do->body = body;
    
    Configure( MakeCheckUncombable(s_while), r_if );
}

IfToIfGoto::IfToIfGoto()
{
    // Identify a general if statements and build a compuond with the usual
    // laep-frogging gotos. Since we are converting from a general kind of if
    // to a more specific kind (the condiitonal goto pattern) we have to 
    // exclude the conditional goto explicitly using and-not in the search 
    // pattern. Otherwise we would spin forever expanding them over and over.
    MakeTreePtr< MatchAll<Statement> > s_and;    
    MakeTreePtr<If> s_if, l_r_if, r_if;
    MakeTreePtr<Statement> body, else_body;
    MakeTreePtr<Expression> cond;
    MakeTreePtr< NotMatch<Statement> > l_r_not;
    MakeTreePtr<Goto> l_r_goto, r_goto, r_goto_else;
    MakeTreePtr<Nop> l_r_nop, r_nop;
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<LogicalNot> r_not;
    MakeTreePtr<BuildLabelIdentifier> r_labelid1("THEN"), r_labelid2("ELSE");
    MakeTreePtr<Label> r_label1, r_label2;
    
    s_and->patterns = (s_if, l_r_not);
    s_if->condition = cond;
    s_if->body = body;
    s_if->else_body = else_body;
    
    // exclude if(x) goto y;
    l_r_not->pattern = l_r_if;
    l_r_if->body = l_r_goto;
    l_r_if->else_body = l_r_nop;
    
    r_comp->statements = (r_if, body, r_goto_else, r_label1, else_body, r_label2);
    r_if->condition = r_not;
    r_not->operands = (cond);
    r_if->body = r_goto;
    r_if->else_body = r_nop;
    r_goto->destination = r_labelid1;
    r_goto_else->destination = r_labelid2;
    r_label1->identifier = r_labelid1;
    r_label2->identifier = r_labelid2;
    
    Configure( MakeCheckUncombable(s_and), r_comp );
}


SwitchToIfGoto::SwitchToIfGoto()
{
    // Add a surrounding compound that declares a variable switch_value which 
    // will hold the value passed to switch. Using slaves, replace case/default 
    // with ordinary labels and, for each one, add a conditional goto (plain goto
    // for default) at the top of the funciton. Condition tests switch_value
    // against the case value or range where GCC's range-case has been used.
    //
    // The order in which we do things is important. We insert the gotos and 
    // conditional gotos at the top of the block, so that they appear in 
    // reverse order of the order in which we inserted them. We want the 
    // plain goto that default produces to be at the bottom, so we do it last.
    //
    // The order of the conditional gotos that result from cases should not matter
    // because cases should not overlap. 
    MakeTreePtr<Switch> s_switch;
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<Statement> body;
    MakeTreePtr<Type> cond_type;
    MakeTreePtr<Automatic> r_decl;
    MakeTreePtr<BuildInstanceIdentifier> id("switch_value");
    MakeTreePtr< TransformOf<Expression> > s_cond( &TypeOf::instance );
    
    // SlaveSearchReplace for default
    MakeTreePtr<Compound> l1_s_body, l1_r_body;
    MakeTreePtr< Star<Declaration> > l1_decls;
    MakeTreePtr< Star<Statement> > l1_pre, l1_post;
    MakeTreePtr< Default > l1_s_default;
    MakeTreePtr< Label > l1_r_label;
    MakeTreePtr<BuildLabelIdentifier> l1_r_labelid("DEFAULT");
    MakeTreePtr<Goto> l1_r_goto;
    
    l1_s_body->members = l1_decls;
    l1_s_body->statements = (l1_pre, l1_s_default, l1_post);
    
    l1_r_body->members = l1_decls;
    l1_r_body->statements = (l1_r_goto, l1_pre, l1_r_label, l1_post);
    l1_r_goto->destination = l1_r_labelid;
    l1_r_label->identifier = l1_r_labelid;
    
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave1( body, l1_s_body, l1_r_body );

    // slave for normal case statements (single value)
    MakeTreePtr<Compound> l2_s_body, l2_r_body;
    MakeTreePtr< Star<Declaration> > l2_decls;
    MakeTreePtr< Star<Statement> > l2_pre, l2_post;
    MakeTreePtr< Case > l2_s_case;
    MakeTreePtr< Label > l2_r_label;
    MakeTreePtr<BuildLabelIdentifier> l2_r_labelid("CASE");
    MakeTreePtr<If> l2_r_if;
    MakeTreePtr<Nop> l2_r_nop;
    MakeTreePtr<Goto> l2_r_goto;
    MakeTreePtr<Equal> l2_r_equal;
    MakeTreePtr<Expression> l2_exp;
    
    l2_s_body->members = l2_decls;
    l2_s_body->statements = (l2_pre, l2_s_case, l2_post);
    l2_s_case->value = l2_exp;
    
    l2_r_body->members = l2_decls;
    l2_r_body->statements = (l2_r_if, l2_pre, l2_r_label, l2_post);
    l2_r_if->condition = l2_r_equal;
    l2_r_if->body = l2_r_goto;
    l2_r_if->else_body = l2_r_nop;
    l2_r_equal->operands = (id, l2_exp);
    l2_r_goto->destination = l2_r_labelid;
    l2_r_label->identifier = l2_r_labelid;
    
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave2( r_slave1, l2_s_body, l2_r_body );
    
    // SlaveSearchReplace for range cases (GCC extension) eg case 5..7:    
    MakeTreePtr<Compound> l3_s_body, l3_r_body;
    MakeTreePtr< Star<Declaration> > l3_decls;
    MakeTreePtr< Star<Statement> > l3_pre, l3_post;
    MakeTreePtr< RangeCase > l3_s_case;
    MakeTreePtr< Label > l3_r_label;
    MakeTreePtr<BuildLabelIdentifier> l3_r_labelid("CASE");
    MakeTreePtr<If> l3_r_if;
    MakeTreePtr<Nop> l3_r_nop;
    MakeTreePtr<Goto> l3_r_goto;
    MakeTreePtr<LogicalAnd> l3_r_and;
    MakeTreePtr<GreaterOrEqual> l3_r_ge;
    MakeTreePtr<LessOrEqual> l3_r_le;
    MakeTreePtr<Expression> l3_exp_lo, l3_exp_hi;
    
    l3_s_body->members = l3_decls;
    l3_s_body->statements = (l3_pre, l3_s_case, l3_post);
    l3_s_case->value_lo = l3_exp_lo;
    l3_s_case->value_hi = l3_exp_hi;
    
    l3_r_body->members = l3_decls;
    l3_r_body->statements = (l3_r_if, l3_pre, l3_r_label, l3_post);
    l3_r_if->condition = l3_r_and;
    l3_r_if->body = l3_r_goto;
    l3_r_if->else_body = l3_r_nop;
    l3_r_and->operands = (l3_r_ge, l3_r_le);
    l3_r_ge->operands = (id, l3_exp_lo);
    l3_r_le->operands = (id, l3_exp_hi);
    l3_r_goto->destination = l3_r_labelid;
    l3_r_label->identifier = l3_r_labelid;
    
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave3( r_slave2, l3_s_body, l3_r_body );

    // Finish up master
    s_cond->pattern = cond_type;
    s_switch->body = body; // will only match when body is a compound
    s_switch->condition = s_cond;
    
    r_decl->identifier = id;
    r_decl->type = cond_type;
    r_decl->initialiser = s_cond;
    r_comp->statements = (r_decl, r_slave3);
    
    Configure( MakeCheckUncombable(s_switch), r_comp );
}


DoToIfGoto::DoToIfGoto()
{
    // Create a compound block, put the body of the loop in there with a Label
    // at the top and a conditional goto (if(x) goto y;) at the bottom, using the
    // same expression as the Do. continue just becomes a Goto directly to the If.
    //
    // We prevent the continue transformation from acting on continues in nested 
    // blocks using the same method as seen in BreakToGoto. 
    
    MakeTreePtr<Do> s_do;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Statement> body;
    MakeTreePtr<Expression> cond;
    MakeTreePtr<Goto> r_goto, l_r_goto;
    MakeTreePtr<Nop> r_nop;    
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<BuildLabelIdentifier> r_labelid("NEXT"), l_r_cont_labelid("CONTINUE");
    MakeTreePtr<Label> r_label, r_cont_label;
    MakeTreePtr< Stuff<Statement> > l_stuff;
    MakeTreePtr< Overlay<Statement> > l_overlay;
    MakeTreePtr<Continue> l_s_cont;
    MakeTreePtr< NotMatch<Statement> > l_s_not;
    MakeTreePtr< Loop > l_s_loop;

    l_s_not->pattern = l_s_loop;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = l_r_goto;
    l_r_goto->destination = l_r_cont_labelid;
    
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave( body, l_stuff, l_stuff );
    l_stuff->terminus = l_overlay;
    
    s_do->condition = cond;
    s_do->body = body;
    
    r_comp->statements = (r_label, r_slave, r_cont_label, r_if);
    r_label->identifier = r_labelid;
    r_cont_label->identifier = l_r_cont_labelid;
    r_if->condition = cond;
    r_if->body = r_goto;
    r_if->else_body = r_nop;
    r_goto->destination = r_labelid;
        
    Configure( MakeCheckUncombable(s_do), r_comp );
}

BreakToGoto::BreakToGoto()
{
    // We determine the right block to exit by:
    // 1. Creating an intermediate node Breakable, which is a base for 
    //    Switch and Loop (and hence For, While, Do). We search for this.
    // 2. The Break is reached via a Stuff node whose recurse 
    //    restriction is set to not recurse through any Breakable
    //    blocks, so we won't find a Break that is not for us.
    MakeTreePtr<Breakable> breakable, sx_breakable;
    MakeTreePtr< Stuff<Statement> > stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr<Break> s_break;
    MakeTreePtr<Goto> r_goto;
    MakeTreePtr<BuildLabelIdentifier> r_labelid("BREAK");
    MakeTreePtr<Label> r_label;
    MakeTreePtr<Compound> r_comp;
    
    sx_not->pattern = sx_breakable;
    stuff->terminus = overlay;
    overlay->through = s_break;
    stuff->recurse_restriction = sx_not;
    overlay->overlay = r_goto;
    r_goto->destination = r_labelid;
    breakable->body = stuff;   
   
    r_comp->statements = (breakable, r_label);
    r_label->identifier = r_labelid;
    
    Configure( MakeCheckUncombable( breakable ), r_comp );
}


LogicalAndToIf::LogicalAndToIf()
{
    MakeTreePtr<LogicalAnd> s_and;
    MakeTreePtr<Expression> op1, op2;
    MakeTreePtr<CompoundExpression> r_comp;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("andtemp");
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr<Boolean> r_boolean;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Assign> r_assign1, r_assign2;
    
    s_and->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = r_assign2;
    r_if->else_body = MakeTreePtr<Nop>();
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( MakeCheckUncombable( s_and ), r_comp );
}


LogicalOrToIf::LogicalOrToIf()
{
    MakeTreePtr<LogicalOr> s_or;
    MakeTreePtr<Expression> op1, op2;
    MakeTreePtr<CompoundExpression> r_comp;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("ortemp");
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr<Boolean> r_boolean;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Assign> r_assign1, r_assign2;
    
    s_or->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = MakeTreePtr<Nop>();
    r_if->else_body = r_assign2;
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( MakeCheckUncombable(s_or), r_comp );
}


MultiplexorToIf::MultiplexorToIf()
{
    MakeTreePtr<Multiplexor> s_mux;
    MakeTreePtr<Expression> op1, op3;
    MakeTreePtr<CompoundExpression> r_comp;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("muxtemp");
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr< TransformOf<Expression> > op2( &TypeOf::instance );
    MakeTreePtr<Type> type;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Assign> r_assignt, r_assignf;
    
    s_mux->operands = (op1, op2, op3);
    op2->pattern = type;
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= type;
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_comp->statements = (r_if, r_temp_id);
    r_if->condition = op1;
    r_if->body = r_assignt;
    r_if->else_body = r_assignf;
    r_assignt->operands = (r_temp_id, op2);    
    r_assignf->operands = (r_temp_id, op3);    
    
    Configure( MakeCheckUncombable(s_mux), r_comp );
}


ExtractCallParams::ExtractCallParams()
{
    MakeTreePtr<Call> s_call, r_call;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("temp_%s");
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr<CompoundExpression> r_ce;
    MakeTreePtr<Assign> r_assign;
    MakeTreePtr< Star<MapOperand> > params;
    MakeTreePtr<MapOperand> s_param, r_param;
    MakeTreePtr< TransformOf<Expression> > value( &TypeOf::instance );
    MakeTreePtr<Type> type;
    MakeTreePtr<Expression> callee;
    MakeTreePtr<InstanceIdentifier> id;
    MakeTreePtr< MatchAll<Expression> > all;
    MakeTreePtr< NotMatch<Expression> > x_not;
    MakeTreePtr<InstanceIdentifier> x_id;
    
    s_call->operands = (params, s_param);
    s_param->value = all;
    all->patterns = (value, x_not);
    s_param->identifier = id;
    value->pattern = type;
    s_call->callee = callee;
    x_not->pattern = x_id; // this restriction to become light-touch restriction
    
    r_ce->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (id);
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_temp->type = type;
    r_ce->statements = (r_assign, r_call);
    r_assign->operands = (r_temp_id, value);
    r_call->operands = (params, r_param);
    r_param->value = r_temp_id;    
    r_param->identifier = id;
    r_call->callee = callee;
    
    Configure( MakeCheckUncombable(s_call), r_ce );
}
