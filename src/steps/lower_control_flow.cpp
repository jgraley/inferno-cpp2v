
#include "steps/lower_control_flow.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "vn/agents/all.hpp"
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
    auto s_all = MakePatternNode< Conjunction<Switch> >();
    auto sx_not = MakePatternNode< Negation<Switch> >();
    auto sx_uswitch = MakePatternNode<UncombableSwitch>();
    auto s_switch = MakePatternNode<Switch>();
    auto expr = MakePatternNode<Expression>();
    auto comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto x_not = MakePatternNode< Negation<Statement> >();
    auto x_break = MakePatternNode<Break>();
    auto target = MakePatternNode<SwitchTarget>();
    auto r_uswitch = MakePatternNode<UncombableSwitch>();
    
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
    auto s_all = MakePatternNode< Conjunction<For> >();
    auto s_not = MakePatternNode< Negation<For> >();
    auto sx_ufor = MakePatternNode<UncombableFor>();
    auto s_for = MakePatternNode<For>();
    auto init = MakePatternNode<Statement>();
    auto test = MakePatternNode<Expression>();
    auto inc = MakePatternNode<Expression>();
    auto body = MakePatternNode<Statement>();
    auto r_ufor = MakePatternNode<UncombableFor>();
    
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
    auto s_ufor = MakePatternNode<UncombableFor>();
    auto init = MakePatternNode<Assign>();
    auto test = MakePatternNode< Disjunction<Operator> >();
    auto lt = MakePatternNode<Less>();
    auto le = MakePatternNode<LessOrEqual>();
    auto gt = MakePatternNode<Greater>();
    auto ge = MakePatternNode<GreaterOrEqual>();
    auto ne = MakePatternNode<NotEqual>();
    auto init_val = MakePatternNode<Integer>();
    auto inc = MakePatternNode< Disjunction<AssignmentOperator> >();
    auto postinc = MakePatternNode<PostIncrement>();
    auto preinc = MakePatternNode<PreIncrement>();
    auto postdec = MakePatternNode<PostDecrement>();
    auto predec = MakePatternNode<PreDecrement>();
    auto asadd = MakePatternNode<AssignmentAdd>();
    auto assub = MakePatternNode<AssignmentSubtract>();
    auto assign1 = MakePatternNode<Assign>();
    auto assign2 = MakePatternNode<Assign>();
    auto add = MakePatternNode<Add>();
    auto sub = MakePatternNode<Subtract>();
    auto body = MakePatternNode< Negation<Statement> >();
    auto astuff = MakePatternNode< Stuff<Statement> >();
    auto assignop = MakePatternNode<AssignmentOperator>();
    
    auto r_for = MakePatternNode<CombableFor>();
    auto loopvar = MakePatternNode< TransformOf<InstanceIdentifier> >( &TypeOf::instance );
    auto type = MakePatternNode<Integral>();
    
    s_ufor->initialisation = init;
    init->operands = (loopvar, init_val);
    s_ufor->condition = test;
    test->disjuncts = (gt, ge, lt, le, ne);
    gt->operands = (loopvar, MakePatternNode<Integer>());
    ge->operands = (loopvar, MakePatternNode<Integer>()); 
    lt->operands = (loopvar, MakePatternNode<Integer>());
    le->operands = (loopvar, MakePatternNode<Integer>());
    ne->operands = (loopvar, MakePatternNode<Integer>());
    s_ufor->increment = inc;
    inc->disjuncts = (preinc, postinc, predec, postdec, asadd, assub, assign1, assign2);
    preinc->operands = (loopvar);
    postinc->operands = (loopvar);
    predec->operands = (loopvar);
    postdec->operands = (loopvar);
    asadd->operands = (loopvar, MakePatternNode<Integer>());
    assub->operands = (loopvar, MakePatternNode<Integer>());
    assign1->operands = (loopvar, add);
    add->operands = (loopvar, MakePatternNode<Integer>());
    assign2->operands = (loopvar, sub);
    sub->operands = (loopvar, MakePatternNode<Integer>());
    s_ufor->body = body;
    body->negand = astuff;
    astuff->terminus = assignop;
    assignop->operands = (loopvar, MakePatternNode< Star<Expression> >());
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
    auto s_not = MakePatternNode< Negation<Break> >();
    auto sx_ubreak = MakePatternNode<UncombableBreak>();
    auto s_break = MakePatternNode<Break>();
    auto init = MakePatternNode<Statement>();
    auto test = MakePatternNode<Expression>();
    auto inc = MakePatternNode<Statement>();
    auto body = MakePatternNode<Statement>();
    auto r_ubreak = MakePatternNode<UncombableBreak>();
    
    s_not->negand = sx_ubreak;
        
    Configure( SEARCH_REPLACE, s_not, r_ubreak );
}


// Detect combable breaks - these are the ones at the top level of
// a combable switch. Run the compound statemnts cleanup before this
// to get breaks in compound blocks. But we do not accept breaks 
// under constructs like if
DetectCombableBreak::DetectCombableBreak()
{
    auto all = MakePatternNode< Conjunction<Switch> >();
    auto x_not = MakePatternNode< Negation<Switch> >();
    auto uswitch = MakePatternNode<UncombableSwitch>();
    auto swtch = MakePatternNode<Switch>();
    auto expr = MakePatternNode<Expression>();
    auto comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto over = MakePatternNode< Delta<Break> >();
    auto s_ubreak = MakePatternNode<UncombableBreak>();
    auto r_break = MakePatternNode<CombableBreak>();
    
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
    
    auto s_for = MakePatternNode<For>();
    auto forbody = MakePatternNode<Statement>();
    auto init = MakePatternNode<Statement>();
    auto cond = MakePatternNode<Expression>();
    auto inc = MakePatternNode<Expression>();
    auto r_while = MakePatternNode<While>();
    auto r_outer = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto l_stuff = MakePatternNode< Stuff<Statement> >();
    auto l_overlay = MakePatternNode< Delta<Statement> >();
    auto l_s_not = MakePatternNode< Negation<Statement> >();
    auto l_s_loop = MakePatternNode< Loop >();
    
    auto l_s_cont = MakePatternNode<Continue>();
    auto l_r_nop = MakePatternNode<Nop>();
    auto r_cont_labelid = MakePatternNode<BuildLabelIdentifierAgent>("CONTINUE");
    auto r_cont_label = MakePatternNode<Label>();
    auto lr_goto = MakePatternNode<Goto>();

    l_stuff->terminus = l_overlay;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = lr_goto;
    lr_goto->destination = r_cont_labelid;
    l_s_not->negand = l_s_loop;
    auto r_embedded = MakePatternNode< EmbeddedCompareReplace<Statement> >( forbody, l_stuff, l_stuff );
    
    s_for->body = forbody;
    s_for->initialisation = init;
    s_for->condition = cond;
    s_for->increment = inc;

    r_outer->statements = (init, r_while);
    r_while->body = r_body;
    r_while->condition = cond;
    r_body->statements = (r_embedded, r_cont_label, inc);
    r_cont_label->identifier = r_cont_labelid;

    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_for), r_outer );
}

WhileToDo::WhileToDo()
{
    // Just need to insert an "if" statement for the case 
    // where there are 0 iterations.
    auto s_while = MakePatternNode<While>();
    auto body = MakePatternNode<Statement>();
    auto cond = MakePatternNode<Expression>();
    auto r_nop = MakePatternNode<Nop>();
    auto r_if = MakePatternNode<If>();
    auto r_do = MakePatternNode<Do>();

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
    auto s_and = MakePatternNode< Conjunction<Statement> >();
    auto s_if = MakePatternNode<If>();
    auto l_r_if = MakePatternNode<If>();
    auto r_if = MakePatternNode<If>();
    auto body = MakePatternNode<Statement>();
    auto else_body = MakePatternNode<Statement>();
    auto cond = MakePatternNode<Expression>();
    auto l_r_not = MakePatternNode< Negation<Statement> >();
    auto l_r_goto = MakePatternNode<Goto>();
    auto r_goto = MakePatternNode<Goto>();
    auto r_goto_else = MakePatternNode<Goto>();
    auto l_r_nop = MakePatternNode<Nop>();
    auto r_nop = MakePatternNode<Nop>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_not = MakePatternNode<LogicalNot>();
    auto r_labelid1 = MakePatternNode<BuildLabelIdentifierAgent>("THEN");
    auto r_labelid2 = MakePatternNode<BuildLabelIdentifierAgent>("ELSE");
    auto r_label1 = MakePatternNode<Label>();
    auto r_label2 = MakePatternNode<Label>();
    
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
    // will hold the value passed to switch. Using embedded patterns, replace case/default 
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
    auto s_switch = MakePatternNode<Switch>();
    auto r_comp = MakePatternNode<Compound>();
    auto body = MakePatternNode<Statement>();
    auto cond_type = MakePatternNode<Type>();
    auto r_decl = MakePatternNode<Automatic>();
    auto id = MakePatternNode<BuildInstanceIdentifierAgent>("switch_value");
    auto s_cond = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    
    // EmbeddedSearchReplace for default
    auto l1_s_body = MakePatternNode<Compound>();
    auto l1_r_body = MakePatternNode<Compound>();
    auto l1_decls = MakePatternNode< Star<Declaration> >();
    auto l1_pre = MakePatternNode< Star<Statement> >();
    auto l1_post = MakePatternNode< Star<Statement> >();
    auto l1_s_default = MakePatternNode< Default >();
    auto l1_r_label = MakePatternNode< Label >();
    auto l1_r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("DEFAULT");
    auto l1_r_goto = MakePatternNode<Goto>();
    
    l1_s_body->members = l1_decls;
    l1_s_body->statements = (l1_pre, l1_s_default, l1_post);
    
    l1_r_body->members = l1_decls;
    l1_r_body->statements = (l1_r_goto, l1_pre, l1_r_label, l1_post);
    l1_r_goto->destination = l1_r_labelid;
    l1_r_label->identifier = l1_r_labelid;
    
    auto r_embedded_1 = MakePatternNode< EmbeddedCompareReplace<Statement> >( body, l1_s_body, l1_r_body );

    // embedded pattern for normal case statements (single value)
    auto l2_s_body = MakePatternNode<Compound>();
    auto l2_r_body = MakePatternNode<Compound>();
    auto l2_decls = MakePatternNode< Star<Declaration> >();
    auto l2_pre = MakePatternNode< Star<Statement> >();
    auto l2_post = MakePatternNode< Star<Statement> >();
    auto l2_s_case = MakePatternNode< Case >();
    auto l2_r_label = MakePatternNode< Label >();
    auto l2_r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("CASE");
    auto l2_r_if = MakePatternNode<If>();
    auto l2_r_nop = MakePatternNode<Nop>();
    auto l2_r_goto = MakePatternNode<Goto>();
    auto l2_r_equal = MakePatternNode<Equal>();
    auto l2_exp = MakePatternNode<Expression>();
    
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
    
    auto r_embedded_2 = MakePatternNode< EmbeddedCompareReplace<Statement> >( r_embedded_1, l2_s_body, l2_r_body );
    
    // EmbeddedSearchReplace for range cases (GCC extension) eg case 5..7:    
    auto l3_s_body = MakePatternNode<Compound>();
    auto l3_r_body = MakePatternNode<Compound>();
    auto l3_decls = MakePatternNode< Star<Declaration> >();
    auto l3_pre = MakePatternNode< Star<Statement> >();
    auto l3_post = MakePatternNode< Star<Statement> >();
    auto l3_s_case = MakePatternNode< RangeCase >();
    auto l3_r_label = MakePatternNode< Label >();
    auto l3_r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("CASE");
    auto l3_r_if = MakePatternNode<If>();
    auto l3_r_nop = MakePatternNode<Nop>();
    auto l3_r_goto = MakePatternNode<Goto>();
    auto l3_r_and = MakePatternNode<LogicalAnd>();
    auto l3_r_ge = MakePatternNode<GreaterOrEqual>();
    auto l3_r_le = MakePatternNode<LessOrEqual>();
    auto l3_exp_lo = MakePatternNode<Expression>();
    auto l3_exp_hi = MakePatternNode<Expression>();
    
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
    
    auto r_embedded_3 = MakePatternNode< EmbeddedCompareReplace<Statement> >( r_embedded_2, l3_s_body, l3_r_body );

    // Finish up top-level
    s_cond->pattern = cond_type;
    s_switch->body = body; // will only match when body is a compound
    s_switch->condition = s_cond;
    
    r_decl->identifier = id;
    r_decl->type = cond_type;
    r_decl->initialiser = s_cond;
    r_comp->statements = (r_decl, r_embedded_3);
    
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
    
    auto s_do = MakePatternNode<Do>();
    auto r_if = MakePatternNode<If>();
    auto body = MakePatternNode<Statement>();
    auto cond = MakePatternNode<Expression>();
    auto r_goto = MakePatternNode<Goto>();
    auto l_r_goto = MakePatternNode<Goto>();
    auto r_nop = MakePatternNode<Nop>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("NEXT");
    auto l_r_cont_labelid = MakePatternNode<BuildLabelIdentifierAgent>("CONTINUE");
    auto r_label = MakePatternNode<Label>();
    auto r_cont_label = MakePatternNode<Label>();
    auto l_stuff = MakePatternNode< Stuff<Statement> >();
    auto l_overlay = MakePatternNode< Delta<Statement> >();
    auto l_s_cont = MakePatternNode<Continue>();
    auto l_s_not = MakePatternNode< Negation<Statement> >();
    auto l_s_loop = MakePatternNode< Loop >();

    l_s_not->negand = l_s_loop;
    l_overlay->through = l_s_cont;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = l_r_goto;
    l_r_goto->destination = l_r_cont_labelid;
    
    auto r_embedded = MakePatternNode< EmbeddedCompareReplace<Statement> >( body, l_stuff, l_stuff );
    l_stuff->terminus = l_overlay;
    
    s_do->condition = cond;
    s_do->body = body;
    
    r_comp->statements = (r_label, r_embedded, r_cont_label, r_if);
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
    auto breakable = MakePatternNode<Breakable>();
    auto sx_breakable = MakePatternNode<Breakable>();
    auto stuff = MakePatternNode< Stuff<Statement> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    auto s_break = MakePatternNode<Break>();
    auto r_goto = MakePatternNode<Goto>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("BREAK");
    auto r_label = MakePatternNode<Label>();
    auto r_comp = MakePatternNode<Compound>();
    
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
    auto s_and = MakePatternNode<LogicalAnd>();
    auto op1 = MakePatternNode<Expression>();
    auto op2 = MakePatternNode<Expression>();
    auto r_comp = MakePatternNode<StatementExpression>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("andtemp");
    auto r_temp = MakePatternNode<Temporary>();
    auto r_boolean = MakePatternNode<Boolean>();
    auto r_if = MakePatternNode<If>();
    auto r_assign1 = MakePatternNode<Assign>();
    auto r_assign2 = MakePatternNode<Assign>();
    
    s_and->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakePatternNode<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = r_assign2;
    r_if->else_body = MakePatternNode<Nop>();
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable( s_and ), r_comp );
}


LogicalOrToIf::LogicalOrToIf()
{
    auto s_or = MakePatternNode<LogicalOr>();
    auto op1 = MakePatternNode<Expression>();
    auto op2 = MakePatternNode<Expression>();
    auto r_comp = MakePatternNode<StatementExpression>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("ortemp");
    auto r_temp = MakePatternNode<Temporary>();
    auto r_boolean = MakePatternNode<Boolean>();
    auto r_if = MakePatternNode<If>();
    auto r_assign1 = MakePatternNode<Assign>();
    auto r_assign2 = MakePatternNode<Assign>();
    
    s_or->operands = (op1, op2);
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= r_boolean;
    r_temp->initialiser = MakePatternNode<Uninitialised>();
    r_comp->statements = (r_assign1, r_if, r_temp_id);
    r_assign1->operands = (r_temp_id, op1);
    r_if->condition = r_temp_id;
    r_if->body = MakePatternNode<Nop>();
    r_if->else_body = r_assign2;
    r_assign2->operands = (r_temp_id, op2);    
    
    Configure( SEARCH_REPLACE, MakeCheckUncombable(s_or), r_comp );
}


ConditionalOperatorToIf::ConditionalOperatorToIf()
{
    auto s_mux = MakePatternNode<ConditionalOperator>();
    auto op1 = MakePatternNode<Expression>();
    auto op3 = MakePatternNode<Expression>();
    auto r_comp = MakePatternNode<StatementExpression>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("muxtemp");
    auto r_temp = MakePatternNode<Temporary>();
    auto op2 = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternNode<Type>();
    auto r_if = MakePatternNode<If>();
    auto r_assignt = MakePatternNode<Assign>();
    auto r_assignf = MakePatternNode<Assign>();
    
    s_mux->operands = (op1, op2, op3);
    op2->pattern = type;
    
    r_comp->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp->type= type;
    r_temp->initialiser = MakePatternNode<Uninitialised>();
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
    auto module = MakePatternNode<Module>();
    auto other_decls = MakePatternNode<Star<Declaration>>();
    auto stuff = MakePatternNode<Stuff<Declaration>>();
    auto delta = MakePatternNode<Delta<Node>>();
    auto field = MakePatternNode<Field>();
    auto func = MakePatternNode<Function>();
    auto func_id = MakePatternNode<InstanceIdentifier>();
    
    auto s_call = MakePatternNode<Call>();
    auto r_call = MakePatternNode<Call>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("temp_%s");
    auto r_temp = MakePatternNode<Temporary>();
    auto r_ce = MakePatternNode<StatementExpression>();
    auto r_assign = MakePatternNode<Assign>();
    auto params = MakePatternNode< Star<IdValuePair> >();
    auto s_param = MakePatternNode<IdValuePair>();
    auto r_param = MakePatternNode<IdValuePair>();
    auto value = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternNode<Type>();
    auto id = MakePatternNode<InstanceIdentifier>();
    auto all = MakePatternNode< Conjunction<Expression> >();
    auto x_not = MakePatternNode< Negation<Expression> >();
    auto x_id = MakePatternNode<InstanceIdentifier>();
    
    module->members = (stuff, field, other_decls);
    stuff->terminus = delta;
    delta->through =    MakeCheckUncombable(s_call);
    delta->overlay =    r_ce;
    field->type = func; 
    func->params = MakePatternNode<Star<Parameter>>();
    field->identifier = func_id;
    
    s_call->operands = (params, s_param);
    s_param->value = all;
    all->conjuncts = (value, x_not);
    s_param->key = id;
    value->pattern = type;
    s_call->callee = func_id;
    x_not->negand = x_id; // this restriction to become light-touch restriction
    
    r_ce->members = (r_temp);
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (id);
    r_temp->initialiser = MakePatternNode<Uninitialised>();
    r_temp->type = type;
    r_ce->statements = (r_assign, r_call);
    r_assign->operands = (r_temp_id, value);
    r_call->operands = (params, r_param);
    r_param->value = r_temp_id;    
    r_param->key = id;
    r_call->callee = func_id;
    
    Configure( SEARCH_REPLACE, module );
}
