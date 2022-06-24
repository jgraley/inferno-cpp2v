
#include "steps/lower_control_flow.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/typeof.hpp"
#include "steps/uncombable.hpp"
#include "inferno_agents.hpp"

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
    auto s_all = MakePatternPtr< Conjunction<Switch> >();
    auto sx_not = MakePatternPtr< Negation<Switch> >();
    auto sx_uswitch = MakePatternPtr<UncombableSwitch>();
    auto s_switch = MakePatternPtr<Switch>();
    auto expr = MakePatternPtr<Expression>();
    auto comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto x_not = MakePatternPtr< Negation<Statement> >();
    auto x_break = MakePatternPtr<Break>();
    auto target = MakePatternPtr<SwitchTarget>();
    auto r_uswitch = MakePatternPtr<UncombableSwitch>();
    
    s_all->conjuncts = (sx_not, s_switch);
    sx_not->negand = sx_uswitch;
    s_switch->body = comp;
    s_switch->condition = expr;
    comp->members = decls;
    comp->statements = (pre, x_not, target, post);
    x_not->negand = x_break;
    
    r_uswitch->body = comp;
    r_uswitch->condition = expr;
    
    Configure( SEARCH_REPLACE, s_all, r_uswitch );
}


// Turn all for into uncombablefor, so the next step can go the other
// way, and can avoid a top-level NOT
MakeAllForUncombable::MakeAllForUncombable()
{
    auto s_all = MakePatternPtr< Conjunction<For> >();
    auto s_not = MakePatternPtr< Negation<For> >();
    auto sx_ufor = MakePatternPtr<UncombableFor>();
    auto s_for = MakePatternPtr<For>();
    auto init = MakePatternPtr<Statement>();
    auto test = MakePatternPtr<Expression>();
    auto inc = MakePatternPtr<Statement>();
    auto body = MakePatternPtr<Statement>();
    auto r_ufor = MakePatternPtr<UncombableFor>();
    
    s_all->conjuncts = (s_not, s_for);
    s_not->negand = sx_ufor;
    s_for->initialisation = init;
    s_for->condition = test;
    s_for->increment = inc;
    s_for->body = body;
    
    r_ufor->initialisation = init;
    r_ufor->condition = test;
    r_ufor->increment = inc;
    r_ufor->body = body;
    
    Configure( SEARCH_REPLACE, s_all, r_ufor );
}


DetectCombableFor::DetectCombableFor()
{
    auto s_ufor = MakePatternPtr<UncombableFor>();
    auto init = MakePatternPtr<Assign>();
    auto test = MakePatternPtr< Disjunction<Operator> >();
    auto lt = MakePatternPtr<Less>();
    auto le = MakePatternPtr<LessOrEqual>();
    auto gt = MakePatternPtr<Greater>();
    auto ge = MakePatternPtr<GreaterOrEqual>();
    auto ne = MakePatternPtr<NotEqual>();
    auto init_val = MakePatternPtr<Integer>();
    auto inc = MakePatternPtr< Disjunction<AssignmentOperator> >();
    auto postinc = MakePatternPtr<PostIncrement>();
    auto preinc = MakePatternPtr<PreIncrement>();
    auto postdec = MakePatternPtr<PostDecrement>();
    auto predec = MakePatternPtr<PreDecrement>();
    auto asadd = MakePatternPtr<AssignmentAdd>();
    auto assub = MakePatternPtr<AssignmentSubtract>();
    auto assign1 = MakePatternPtr<Assign>();
    auto assign2 = MakePatternPtr<Assign>();
    auto add = MakePatternPtr<Add>();
    auto sub = MakePatternPtr<Subtract>();
    auto body = MakePatternPtr< Negation<Statement> >();
    auto astuff = MakePatternPtr< Stuff<Statement> >();
    auto assignop = MakePatternPtr<AssignmentOperator>();
    
    auto r_for = MakePatternPtr<CombableFor>();
    auto loopvar = MakePatternPtr< TransformOf<InstanceIdentifier> >( &TypeOf::instance );
    auto type = MakePatternPtr<Integral>();
    
    s_ufor->initialisation = init;
    init->operands = (loopvar, init_val);
    s_ufor->condition = test;
    test->disjuncts = (gt, ge, lt, le, ne);
    gt->operands = (loopvar, MakePatternPtr<Integer>());
    ge->operands = (loopvar, MakePatternPtr<Integer>()); 
    lt->operands = (loopvar, MakePatternPtr<Integer>());
    le->operands = (loopvar, MakePatternPtr<Integer>());
    ne->operands = (loopvar, MakePatternPtr<Integer>());
    s_ufor->increment = inc;
    inc->disjuncts = (preinc, postinc, predec, postdec, asadd, assub, assign1, assign2);
    preinc->operands = (loopvar);
    postinc->operands = (loopvar);
    predec->operands = (loopvar);
    postdec->operands = (loopvar);
    asadd->operands = (loopvar, MakePatternPtr<Integer>());
    assub->operands = (loopvar, MakePatternPtr<Integer>());
    assign1->operands = (loopvar, add);
    add->operands = (loopvar, MakePatternPtr<Integer>());
    assign2->operands = (loopvar, sub);
    sub->operands = (loopvar, MakePatternPtr<Integer>());
    s_ufor->body = body;
    body->negand = astuff;
    astuff->terminus = assignop;
    assignop->operands = (loopvar, MakePatternPtr< Star<Expression> >());
    loopvar->pattern = type;
    
    r_for->initialisation = init;
    r_for->condition = test;
    r_for->increment = inc;
    r_for->body = body;
    
    Configure( SEARCH_REPLACE, s_ufor, r_for );
}


// Turn all break into uncombable break, so the next step can go the other
// way, and can avoid a top-level NOT
MakeAllBreakUncombable::MakeAllBreakUncombable()
{
    auto s_not = MakePatternPtr< Negation<Break> >();
    auto sx_ubreak = MakePatternPtr<UncombableBreak>();
    auto s_break = MakePatternPtr<Break>();
    auto init = MakePatternPtr<Statement>();
    auto test = MakePatternPtr<Expression>();
    auto inc = MakePatternPtr<Statement>();
    auto body = MakePatternPtr<Statement>();
    auto r_ubreak = MakePatternPtr<UncombableBreak>();
    
    s_not->negand = sx_ubreak;
        
    Configure( SEARCH_REPLACE, s_not, r_ubreak );
}


// Detect combable breaks - these are the ones at the top level of
// a combable switch. Run the compound statemnts cleanup before this
// to get breaks in compound blocks. But we do not accept breaks 
// under constructs like if
DetectCombableBreak::DetectCombableBreak()
{
    auto all = MakePatternPtr< Conjunction<Switch> >();
    auto x_not = MakePatternPtr< Negation<Switch> >();
    auto uswitch = MakePatternPtr<UncombableSwitch>();
    auto swtch = MakePatternPtr<Switch>();
    auto expr = MakePatternPtr<Expression>();
    auto comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto over = MakePatternPtr< Delta<Break> >();
    auto s_ubreak = MakePatternPtr<UncombableBreak>();
    auto r_break = MakePatternPtr<CombableBreak>();
    
    all->conjuncts = (x_not, swtch);
    x_not->negand = uswitch;
    swtch->condition = expr;
    swtch->body = comp;
    comp->members = decls;
    comp->statements = (pre, over, post);
    over->through = s_ubreak;
    over->overlay = r_break;
    
    Configure( SEARCH_REPLACE, all, swtch );
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
    
    auto s_for = MakePatternPtr<For>();
    auto forbody = MakePatternPtr<Statement>();
    auto inc = MakePatternPtr<Statement>();
    auto init = MakePatternPtr<Statement>();
    auto cond = MakePatternPtr<Expression>();
    auto r_while = MakePatternPtr<While>();
    auto r_outer = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto l_stuff = MakePatternPtr< Stuff<Statement> >();
    auto l_overlay = MakePatternPtr< Delta<Statement> >();
    auto l_s_not = MakePatternPtr< Negation<Statement> >();
    auto l_s_loop = MakePatternPtr< Loop >();
    
    auto l_s_cont = MakePatternPtr<Continue>();
    auto l_r_nop = MakePatternPtr<Nop>();
    auto r_cont_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("CONTINUE");
    auto r_cont_label = MakePatternPtr<Label>();
    auto lr_goto = MakePatternPtr<Goto>();

    l_stuff->terminus = l_overlay;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = lr_goto;
    lr_goto->destination = r_cont_labelid;
    l_s_not->negand = l_s_loop;
    auto r_slave = MakePatternPtr< SlaveCompareReplace<Statement> >( forbody, l_stuff, l_stuff );
    
    s_for->body = forbody;
    s_for->initialisation = init;
    s_for->condition = cond;
    s_for->increment = inc;

    r_outer->statements = (init, r_while);
    r_while->body = r_body;
    r_while->condition = cond;
    r_body->statements = (r_slave, r_cont_label, inc);
    r_cont_label->identifier = r_cont_labelid;

    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_for), r_outer );
}

WhileToDo::WhileToDo()
{
    // Just need to insert an "if" statement for the case 
    // where there are 0 iterations.
    auto s_while = MakePatternPtr<While>();
    auto body = MakePatternPtr<Statement>();
    auto cond = MakePatternPtr<Expression>();
    auto r_nop = MakePatternPtr<Nop>();
    auto r_if = MakePatternPtr<If>();
    auto r_do = MakePatternPtr<Do>();

    s_while->body = body;
    s_while->condition = cond;

    r_if->condition = cond;
    r_if->body = r_do;
    r_if->else_body = r_nop;
    r_do->condition = cond;
    r_do->body = body;
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_while), r_if );
}

IfToIfGoto::IfToIfGoto()
{
    // Identify a general if statements and build a compuond with the usual
    // laep-frogging gotos. Since we are converting from a general kind of if
    // to a more specific kind (the condiitonal goto pattern) we have to 
    // exclude the conditional goto explicitly using and-not in the search 
    // pattern. Otherwise we would spin forever expanding them over and over.
    auto s_and = MakePatternPtr< Conjunction<Statement> >();
    auto s_if = MakePatternPtr<If>();
    auto l_r_if = MakePatternPtr<If>();
    auto r_if = MakePatternPtr<If>();
    auto body = MakePatternPtr<Statement>();
    auto else_body = MakePatternPtr<Statement>();
    auto cond = MakePatternPtr<Expression>();
    auto l_r_not = MakePatternPtr< Negation<Statement> >();
    auto l_r_goto = MakePatternPtr<Goto>();
    auto r_goto = MakePatternPtr<Goto>();
    auto r_goto_else = MakePatternPtr<Goto>();
    auto l_r_nop = MakePatternPtr<Nop>();
    auto r_nop = MakePatternPtr<Nop>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_not = MakePatternPtr<LogicalNot>();
    auto r_labelid1 = MakePatternPtr<BuildLabelIdentifierAgent>("THEN");
    auto r_labelid2 = MakePatternPtr<BuildLabelIdentifierAgent>("ELSE");
    auto r_label1 = MakePatternPtr<Label>();
    auto r_label2 = MakePatternPtr<Label>();
    
    s_and->conjuncts = (s_if, l_r_not);
    s_if->condition = cond;
    s_if->body = body;
    s_if->else_body = else_body;
    
    // exclude if(x) goto y;
    l_r_not->negand = l_r_if;
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
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_and), r_comp );
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
    auto s_switch = MakePatternPtr<Switch>();
    auto r_comp = MakePatternPtr<Compound>();
    auto body = MakePatternPtr<Statement>();
    auto cond_type = MakePatternPtr<Type>();
    auto r_decl = MakePatternPtr<Automatic>();
    auto id = MakePatternPtr<BuildInstanceIdentifierAgent>("switch_value");
    auto s_cond = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    
    // SlaveSearchReplace for default
    auto l1_s_body = MakePatternPtr<Compound>();
    auto l1_r_body = MakePatternPtr<Compound>();
    auto l1_decls = MakePatternPtr< Star<Declaration> >();
    auto l1_pre = MakePatternPtr< Star<Statement> >();
    auto l1_post = MakePatternPtr< Star<Statement> >();
    auto l1_s_default = MakePatternPtr< Default >();
    auto l1_r_label = MakePatternPtr< Label >();
    auto l1_r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("DEFAULT");
    auto l1_r_goto = MakePatternPtr<Goto>();
    
    l1_s_body->members = l1_decls;
    l1_s_body->statements = (l1_pre, l1_s_default, l1_post);
    
    l1_r_body->members = l1_decls;
    l1_r_body->statements = (l1_r_goto, l1_pre, l1_r_label, l1_post);
    l1_r_goto->destination = l1_r_labelid;
    l1_r_label->identifier = l1_r_labelid;
    
    auto r_slave1 = MakePatternPtr< SlaveCompareReplace<Statement> >( body, l1_s_body, l1_r_body );

    // slave for normal case statements (single value)
    auto l2_s_body = MakePatternPtr<Compound>();
    auto l2_r_body = MakePatternPtr<Compound>();
    auto l2_decls = MakePatternPtr< Star<Declaration> >();
    auto l2_pre = MakePatternPtr< Star<Statement> >();
    auto l2_post = MakePatternPtr< Star<Statement> >();
    auto l2_s_case = MakePatternPtr< Case >();
    auto l2_r_label = MakePatternPtr< Label >();
    auto l2_r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("CASE");
    auto l2_r_if = MakePatternPtr<If>();
    auto l2_r_nop = MakePatternPtr<Nop>();
    auto l2_r_goto = MakePatternPtr<Goto>();
    auto l2_r_equal = MakePatternPtr<Equal>();
    auto l2_exp = MakePatternPtr<Expression>();
    
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
    
    auto r_slave2 = MakePatternPtr< SlaveCompareReplace<Statement> >( r_slave1, l2_s_body, l2_r_body );
    
    // SlaveSearchReplace for range cases (GCC extension) eg case 5..7:    
    auto l3_s_body = MakePatternPtr<Compound>();
    auto l3_r_body = MakePatternPtr<Compound>();
    auto l3_decls = MakePatternPtr< Star<Declaration> >();
    auto l3_pre = MakePatternPtr< Star<Statement> >();
    auto l3_post = MakePatternPtr< Star<Statement> >();
    auto l3_s_case = MakePatternPtr< RangeCase >();
    auto l3_r_label = MakePatternPtr< Label >();
    auto l3_r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("CASE");
    auto l3_r_if = MakePatternPtr<If>();
    auto l3_r_nop = MakePatternPtr<Nop>();
    auto l3_r_goto = MakePatternPtr<Goto>();
    auto l3_r_and = MakePatternPtr<LogicalAnd>();
    auto l3_r_ge = MakePatternPtr<GreaterOrEqual>();
    auto l3_r_le = MakePatternPtr<LessOrEqual>();
    auto l3_exp_lo = MakePatternPtr<Expression>();
    auto l3_exp_hi = MakePatternPtr<Expression>();
    
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
    
    auto r_slave3 = MakePatternPtr< SlaveCompareReplace<Statement> >( r_slave2, l3_s_body, l3_r_body );

    // Finish up master
    s_cond->pattern = cond_type;
    s_switch->body = body; // will only match when body is a compound
    s_switch->condition = s_cond;
    
    r_decl->identifier = id;
    r_decl->type = cond_type;
    r_decl->initialiser = s_cond;
    r_comp->statements = (r_decl, r_slave3);
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_switch), r_comp );
}


DoToIfGoto::DoToIfGoto()
{
    // Create a compound block, put the body of the loop in there with a Label
    // at the top and a conditional goto (if(x) goto y;) at the bottom, using the
    // same expression as the Do. continue just becomes a Goto directly to the If.
    //
    // We prevent the continue transformation from acting on continues in nested 
    // blocks using the same method as seen in BreakToGoto. 
    
    auto s_do = MakePatternPtr<Do>();
    auto r_if = MakePatternPtr<If>();
    auto body = MakePatternPtr<Statement>();
    auto cond = MakePatternPtr<Expression>();
    auto r_goto = MakePatternPtr<Goto>();
    auto l_r_goto = MakePatternPtr<Goto>();
    auto r_nop = MakePatternPtr<Nop>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("NEXT");
    auto l_r_cont_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("CONTINUE");
    auto r_label = MakePatternPtr<Label>();
    auto r_cont_label = MakePatternPtr<Label>();
    auto l_stuff = MakePatternPtr< Stuff<Statement> >();
    auto l_overlay = MakePatternPtr< Delta<Statement> >();
    auto l_s_cont = MakePatternPtr<Continue>();
    auto l_s_not = MakePatternPtr< Negation<Statement> >();
    auto l_s_loop = MakePatternPtr< Loop >();

    l_s_not->negand = l_s_loop;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = l_r_goto;
    l_r_goto->destination = l_r_cont_labelid;
    
    auto r_slave = MakePatternPtr< SlaveCompareReplace<Statement> >( body, l_stuff, l_stuff );
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
        
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_do), r_comp );
}

BreakToGoto::BreakToGoto()
{
    // We determine the right block to exit by:
    // 1. Creating an intermediate node Breakable, which is a base for 
    //    Switch and Loop (and hence For, While, Do). We search for this.
    // 2. The Break is reached via a Stuff node whose recurse 
    //    restriction is set to not recurse through any Breakable
    //    blocks, so we won't find a Break that is not for us.
    auto breakable = MakePatternPtr<Breakable>();
    auto sx_breakable = MakePatternPtr<Breakable>();
    auto stuff = MakePatternPtr< Stuff<Statement> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto s_break = MakePatternPtr<Break>();
    auto r_goto = MakePatternPtr<Goto>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("BREAK");
    auto r_label = MakePatternPtr<Label>();
    auto r_comp = MakePatternPtr<Compound>();
    
    sx_not->negand = sx_breakable;
    stuff->terminus = overlay;
    overlay->through = s_break;
    stuff->recurse_restriction = sx_not;
    overlay->overlay = r_goto;
    r_goto->destination = r_labelid;
    breakable->body = stuff;   
   
    r_comp->statements = (breakable, r_label);
    r_label->identifier = r_labelid;
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable( breakable ), r_comp );
}


LogicalAndToIf::LogicalAndToIf()
{
    auto s_and = MakePatternPtr<LogicalAnd>();
    auto op1 = MakePatternPtr<Expression>();
    auto op2 = MakePatternPtr<Expression>();
    auto r_comp = MakePatternPtr<StatementExpression>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("andtemp");
    auto r_temp = MakePatternPtr<Temporary>();
    auto r_boolean = MakePatternPtr<Boolean>();
    auto r_if = MakePatternPtr<If>();
    auto r_assign1 = MakePatternPtr<Assign>();
    auto r_assign2 = MakePatternPtr<Assign>();
    
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
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable( s_and ), r_comp );
}


LogicalOrToIf::LogicalOrToIf()
{
    auto s_or = MakePatternPtr<LogicalOr>();
    auto op1 = MakePatternPtr<Expression>();
    auto op2 = MakePatternPtr<Expression>();
    auto r_comp = MakePatternPtr<StatementExpression>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("ortemp");
    auto r_temp = MakePatternPtr<Temporary>();
    auto r_boolean = MakePatternPtr<Boolean>();
    auto r_if = MakePatternPtr<If>();
    auto r_assign1 = MakePatternPtr<Assign>();
    auto r_assign2 = MakePatternPtr<Assign>();
    
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
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_or), r_comp );
}


ConditionalOperatorToIf::ConditionalOperatorToIf()
{
    auto s_mux = MakePatternPtr<ConditionalOperator>();
    auto op1 = MakePatternPtr<Expression>();
    auto op3 = MakePatternPtr<Expression>();
    auto r_comp = MakePatternPtr<StatementExpression>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("muxtemp");
    auto r_temp = MakePatternPtr<Temporary>();
    auto op2 = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternPtr<Type>();
    auto r_if = MakePatternPtr<If>();
    auto r_assignt = MakePatternPtr<Assign>();
    auto r_assignf = MakePatternPtr<Assign>();
    
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
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_mux), r_comp );
}


ExtractCallParams::ExtractCallParams()
{
    auto s_call = MakePatternPtr<Call>();
    auto r_call = MakePatternPtr<Call>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("temp_%s");
    auto r_temp = MakePatternPtr<Temporary>();
    auto r_ce = MakePatternPtr<StatementExpression>();
    auto r_assign = MakePatternPtr<Assign>();
    auto params = MakePatternPtr< Star<MapOperand> >();
    auto s_param = MakePatternPtr<MapOperand>();
    auto r_param = MakePatternPtr<MapOperand>();
    auto value = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternPtr<Type>();
    auto callee = MakePatternPtr<Expression>();
    auto id = MakePatternPtr<InstanceIdentifier>();
    auto all = MakePatternPtr< Conjunction<Expression> >();
    auto x_not = MakePatternPtr< Negation<Expression> >();
    auto x_id = MakePatternPtr<InstanceIdentifier>();
    
    s_call->operands = (params, s_param);
    s_param->value = all;
    all->conjuncts = (value, x_not);
    s_param->identifier = id;
    value->pattern = type;
    s_call->callee = callee;
    x_not->negand = x_id; // this restriction to become light-touch restriction
    
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
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_call), r_ce );
}
