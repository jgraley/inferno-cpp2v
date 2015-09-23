
#include "steps/lower_control_flow.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/pointer_is_agent.hpp"
#include "sr/transform_of_agent.hpp"
#include "tree/typeof.hpp"
#include "steps/uncombable.hpp"
#include "inferno_patterns.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;

// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : Switch, Uncombable { NODE_FUNCTIONS_FINAL };
struct UncombableFor : For, Uncombable { NODE_FUNCTIONS_FINAL };
struct CombableFor : For { NODE_FUNCTIONS_FINAL };
struct UncombableBreak : Break, Uncombable { NODE_FUNCTIONS_FINAL };
struct CombableBreak : Break { NODE_FUNCTIONS_FINAL };
   

DetectUncombableSwitch::DetectUncombableSwitch()
{
    MakePatternPtr< MatchAll<Switch> > s_all;
    MakePatternPtr< NotMatch<Switch> > sx_not;
    MakePatternPtr<UncombableSwitch> sx_uswitch;
    MakePatternPtr<Switch> s_switch;
    MakePatternPtr<Expression> expr;
    MakePatternPtr<Compound> comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< NotMatch<Statement> > x_not;
    MakePatternPtr<Break> x_break;
    MakePatternPtr<SwitchTarget> target;
    MakePatternPtr<UncombableSwitch> r_uswitch;
    
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
    MakePatternPtr< MatchAll<For> > s_all;
    MakePatternPtr< NotMatch<For> > s_not;
    MakePatternPtr<UncombableFor> sx_ufor;
    MakePatternPtr<For> s_for;
    MakePatternPtr<Statement> init;
    MakePatternPtr<Expression> test; 
    MakePatternPtr<Statement> inc; 
    MakePatternPtr<Statement> body;
    MakePatternPtr<UncombableFor> r_ufor;
    
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
    MakePatternPtr<UncombableFor> s_ufor;
    MakePatternPtr<Assign> init;
    MakePatternPtr< MatchAny<Operator> > test;
    MakePatternPtr<Less> lt;
    MakePatternPtr<LessOrEqual> le;
    MakePatternPtr<Greater> gt;
    MakePatternPtr<GreaterOrEqual> ge;
    MakePatternPtr<NotEqual> ne;    
    MakePatternPtr<Integer> init_val, test_val, inc_val;
    MakePatternPtr< MatchAny<AssignmentOperator> > inc;
    MakePatternPtr<PostIncrement> postinc; 
    MakePatternPtr<PreIncrement> preinc; 
    MakePatternPtr<PostDecrement> postdec; 
    MakePatternPtr<PreDecrement> predec; 
    MakePatternPtr<AssignmentAdd> asadd; 
    MakePatternPtr<AssignmentSubtract> assub; 
    MakePatternPtr<Assign> assign1, assign2;
    MakePatternPtr<Add> add;
    MakePatternPtr<Subtract> sub;    
    MakePatternPtr< NotMatch<Statement> > body;
    MakePatternPtr< Stuff<Statement> > astuff;
    MakePatternPtr<AssignmentOperator> assignop;
    
    MakePatternPtr<CombableFor> r_for;
    MakePatternPtr< TransformOf<InstanceIdentifier> > loopvar( &TypeOf::instance );
    MakePatternPtr<Integral> type;
    
    s_ufor->initialisation = init;
    init->operands = (loopvar, init_val);
    s_ufor->condition = test;
    test->patterns = (gt, ge, lt, le, ne);
    gt->operands = (loopvar, test_val);
    ge->operands = (loopvar, test_val); 
    lt->operands = (loopvar, test_val);
    le->operands = (loopvar, test_val);
    ne->operands = (loopvar, test_val);
    s_ufor->increment = inc;
    inc->patterns = (preinc, postinc, predec, postdec, asadd, assub, assign1, assign2);
    preinc->operands = (loopvar);
    postinc->operands = (loopvar);
    predec->operands = (loopvar);
    postdec->operands = (loopvar);
    asadd->operands = (loopvar, inc_val);
    assub->operands = (loopvar, inc_val);
    assign1->operands = (loopvar, add);
    add->operands = (loopvar, inc_val);
    assign2->operands = (loopvar, sub);
    sub->operands = (loopvar, inc_val);
    s_ufor->body = body;
    body->pattern = astuff;
    astuff->terminus = assignop;
    assignop->operands = (loopvar, MakePatternPtr< Star<Expression> >());
    loopvar->pattern = type;
    
    r_for->initialisation = init;
    r_for->condition = test;
    r_for->increment = inc;
    r_for->body = body;
    
    Configure( s_ufor, r_for );
}


// Turn all break into uncombable break, so the next step can go the other
// way, and can avoid a top-level NOT
MakeAllBreakUncombable::MakeAllBreakUncombable()
{
    MakePatternPtr< NotMatch<Break> > s_not;
    MakePatternPtr<UncombableBreak> sx_ubreak;
    MakePatternPtr<Break> s_break;
    MakePatternPtr<Statement> init;
    MakePatternPtr<Expression> test; 
    MakePatternPtr<Statement> inc; 
    MakePatternPtr<Statement> body;
    MakePatternPtr<UncombableBreak> r_ubreak;
    
    s_not->pattern = sx_ubreak;
        
    Configure( s_not, r_ubreak );
}


// Detect combable breaks - these are the ones at the top level of
// a combable switch. Run the compound statemnts cleanup before this
// to get breaks in compound blocks. But we do not accept breaks 
// under constructs like if
DetectCombableBreak::DetectCombableBreak()
{
    MakePatternPtr< MatchAll<Switch> > all;
    MakePatternPtr< NotMatch<Switch> > x_not;
    MakePatternPtr<UncombableSwitch> uswitch;
    MakePatternPtr<Switch> swtch;
    MakePatternPtr<Expression> expr;
    MakePatternPtr<Compound> comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< Overlay<Break> > over;
    MakePatternPtr<UncombableBreak> s_ubreak;
    MakePatternPtr<CombableBreak> r_break;
    
    all->patterns = (x_not, swtch);
    x_not->pattern = uswitch;
    swtch->condition = expr;
    swtch->body = comp;
    comp->members = decls;
    comp->statements = (pre, over, post);
    over->through = s_ubreak;
    over->overlay = r_break;
    
    Configure( all, swtch );
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
    
    MakePatternPtr<For> s_for;
    MakePatternPtr<Statement> forbody, inc, init;
    MakePatternPtr<Expression> cond;
    MakePatternPtr<While> r_while;
    MakePatternPtr<Compound> r_outer, r_body;
    MakePatternPtr< Stuff<Statement> > l_stuff;
    MakePatternPtr< Overlay<Statement> > l_overlay;
    MakePatternPtr< NotMatch<Statement> > l_s_not;
    MakePatternPtr< Loop > l_s_loop;
    
    MakePatternPtr<Continue> l_s_cont;
    MakePatternPtr<Nop> l_r_nop;
    MakePatternPtr<BuildLabelIdentifier> r_cont_labelid("CONTINUE");
    MakePatternPtr<Label> r_cont_label;
    MakePatternPtr<Goto> lr_goto;

    l_stuff->terminus = l_overlay;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = lr_goto;
    lr_goto->destination = r_cont_labelid;
    l_s_not->pattern = l_s_loop;
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave( forbody, l_stuff, l_stuff );
    
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
    MakePatternPtr<While> s_while;
    MakePatternPtr<Statement> body;
    MakePatternPtr<Expression> cond;
    MakePatternPtr<Nop> r_nop;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Do> r_do;

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
    MakePatternPtr< MatchAll<Statement> > s_and;    
    MakePatternPtr<If> s_if, l_r_if, r_if;
    MakePatternPtr<Statement> body, else_body;
    MakePatternPtr<Expression> cond;
    MakePatternPtr< NotMatch<Statement> > l_r_not;
    MakePatternPtr<Goto> l_r_goto, r_goto, r_goto_else;
    MakePatternPtr<Nop> l_r_nop, r_nop;
    MakePatternPtr<Compound> r_comp;
    MakePatternPtr<LogicalNot> r_not;
    MakePatternPtr<BuildLabelIdentifier> r_labelid1("THEN"), r_labelid2("ELSE");
    MakePatternPtr<Label> r_label1, r_label2;
    
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
    MakePatternPtr<Switch> s_switch;
    MakePatternPtr<Compound> r_comp;
    MakePatternPtr<Statement> body;
    MakePatternPtr<Type> cond_type;
    MakePatternPtr<Automatic> r_decl;
    MakePatternPtr<BuildInstanceIdentifier> id("switch_value");
    MakePatternPtr< TransformOf<Expression> > s_cond( &TypeOf::instance );
    
    // SlaveSearchReplace for default
    MakePatternPtr<Compound> l1_s_body, l1_r_body;
    MakePatternPtr< Star<Declaration> > l1_decls;
    MakePatternPtr< Star<Statement> > l1_pre, l1_post;
    MakePatternPtr< Default > l1_s_default;
    MakePatternPtr< Label > l1_r_label;
    MakePatternPtr<BuildLabelIdentifier> l1_r_labelid("DEFAULT");
    MakePatternPtr<Goto> l1_r_goto;
    
    l1_s_body->members = l1_decls;
    l1_s_body->statements = (l1_pre, l1_s_default, l1_post);
    
    l1_r_body->members = l1_decls;
    l1_r_body->statements = (l1_r_goto, l1_pre, l1_r_label, l1_post);
    l1_r_goto->destination = l1_r_labelid;
    l1_r_label->identifier = l1_r_labelid;
    
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave1( body, l1_s_body, l1_r_body );

    // slave for normal case statements (single value)
    MakePatternPtr<Compound> l2_s_body, l2_r_body;
    MakePatternPtr< Star<Declaration> > l2_decls;
    MakePatternPtr< Star<Statement> > l2_pre, l2_post;
    MakePatternPtr< Case > l2_s_case;
    MakePatternPtr< Label > l2_r_label;
    MakePatternPtr<BuildLabelIdentifier> l2_r_labelid("CASE");
    MakePatternPtr<If> l2_r_if;
    MakePatternPtr<Nop> l2_r_nop;
    MakePatternPtr<Goto> l2_r_goto;
    MakePatternPtr<Equal> l2_r_equal;
    MakePatternPtr<Expression> l2_exp;
    
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
    
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave2( r_slave1, l2_s_body, l2_r_body );
    
    // SlaveSearchReplace for range cases (GCC extension) eg case 5..7:    
    MakePatternPtr<Compound> l3_s_body, l3_r_body;
    MakePatternPtr< Star<Declaration> > l3_decls;
    MakePatternPtr< Star<Statement> > l3_pre, l3_post;
    MakePatternPtr< RangeCase > l3_s_case;
    MakePatternPtr< Label > l3_r_label;
    MakePatternPtr<BuildLabelIdentifier> l3_r_labelid("CASE");
    MakePatternPtr<If> l3_r_if;
    MakePatternPtr<Nop> l3_r_nop;
    MakePatternPtr<Goto> l3_r_goto;
    MakePatternPtr<LogicalAnd> l3_r_and;
    MakePatternPtr<GreaterOrEqual> l3_r_ge;
    MakePatternPtr<LessOrEqual> l3_r_le;
    MakePatternPtr<Expression> l3_exp_lo, l3_exp_hi;
    
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
    
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave3( r_slave2, l3_s_body, l3_r_body );

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
    
    MakePatternPtr<Do> s_do;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Statement> body;
    MakePatternPtr<Expression> cond;
    MakePatternPtr<Goto> r_goto, l_r_goto;
    MakePatternPtr<Nop> r_nop;    
    MakePatternPtr<Compound> r_comp;
    MakePatternPtr<BuildLabelIdentifier> r_labelid("NEXT"), l_r_cont_labelid("CONTINUE");
    MakePatternPtr<Label> r_label, r_cont_label;
    MakePatternPtr< Stuff<Statement> > l_stuff;
    MakePatternPtr< Overlay<Statement> > l_overlay;
    MakePatternPtr<Continue> l_s_cont;
    MakePatternPtr< NotMatch<Statement> > l_s_not;
    MakePatternPtr< Loop > l_s_loop;

    l_s_not->pattern = l_s_loop;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = l_r_goto;
    l_r_goto->destination = l_r_cont_labelid;
    
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave( body, l_stuff, l_stuff );
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
    MakePatternPtr<Breakable> breakable, sx_breakable;
    MakePatternPtr< Stuff<Statement> > stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr< NotMatch<Statement> > sx_not;
    MakePatternPtr<Break> s_break;
    MakePatternPtr<Goto> r_goto;
    MakePatternPtr<BuildLabelIdentifier> r_labelid("BREAK");
    MakePatternPtr<Label> r_label;
    MakePatternPtr<Compound> r_comp;
    
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
    MakePatternPtr<LogicalAnd> s_and;
    MakePatternPtr<Expression> op1, op2;
    MakePatternPtr<CompoundExpression> r_comp;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("andtemp");
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr<Boolean> r_boolean;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Assign> r_assign1, r_assign2;
    
    s_and->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = r_assign2;
    r_if->else_body = MakePatternPtr<Nop>();
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( MakeCheckUncombable( s_and ), r_comp );
}


LogicalOrToIf::LogicalOrToIf()
{
    MakePatternPtr<LogicalOr> s_or;
    MakePatternPtr<Expression> op1, op2;
    MakePatternPtr<CompoundExpression> r_comp;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("ortemp");
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr<Boolean> r_boolean;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Assign> r_assign1, r_assign2;
    
    s_or->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = MakePatternPtr<Nop>();
    r_if->else_body = r_assign2;
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( MakeCheckUncombable(s_or), r_comp );
}


MultiplexorToIf::MultiplexorToIf()
{
    MakePatternPtr<Multiplexor> s_mux;
    MakePatternPtr<Expression> op1, op3;
    MakePatternPtr<CompoundExpression> r_comp;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("muxtemp");
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr< TransformOf<Expression> > op2( &TypeOf::instance );
    MakePatternPtr<Type> type;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Assign> r_assignt, r_assignf;
    
    s_mux->operands = (op1, op2, op3);
    op2->pattern = type;
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= type;
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
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
    MakePatternPtr<Call> s_call, r_call;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("temp_%s");
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr<CompoundExpression> r_ce;
    MakePatternPtr<Assign> r_assign;
    MakePatternPtr< Star<MapOperand> > params;
    MakePatternPtr<MapOperand> s_param, r_param;
    MakePatternPtr< TransformOf<Expression> > value( &TypeOf::instance );
    MakePatternPtr<Type> type;
    MakePatternPtr<Expression> callee;
    MakePatternPtr<InstanceIdentifier> id;
    MakePatternPtr< MatchAll<Expression> > all;
    MakePatternPtr< NotMatch<Expression> > x_not;
    MakePatternPtr<InstanceIdentifier> x_id;
    
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
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_temp->type = type;
    r_ce->statements = (r_assign, r_call);
    r_assign->operands = (r_temp_id, value);
    r_call->operands = (params, r_param);
    r_param->value = r_temp_id;    
    r_param->identifier = id;
    r_call->callee = callee;
    
    Configure( MakeCheckUncombable(s_call), r_ce );
}
