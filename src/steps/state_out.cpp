
#include "steps/state_out.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_agents.hpp"

 
using namespace CPPTree;
using namespace SCTree;
using namespace Steps;
 
// TOOD go through step impls and use inline decls of leaf nodes, to reduce wordiness

GotoAfterWait::GotoAfterWait()
{
    // This step could have been tricky: we'd have needed 3 cases:
    // 1. { *, wait, !goto, * }
    // 2. { *, wait } // not covered by previous case because !goto needs to match a statement
    // 3. eg if() wait // not directly inside a compound block 
    // we use the but-not pattern, in which we principally search for anything->wait, but exclude
    // the only case that needs excluding which is a compound like in case 1 above but without
    // notting the goto. Cases 2 and 3 work because the excluded compound does not match.
    // Overall, a good example of but-not
    auto sx_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto sx_decls = MakePatternPtr< Star<Declaration> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto sx_post = MakePatternPtr< Star<Statement> >();
    auto wait = MakePatternPtr<Wait>();
    auto notmatch = MakePatternPtr< Negation<Statement> >();
    auto all = MakePatternPtr< Conjunction<Statement> >();
    auto anynode = MakePatternPtr< AnyNode<Statement> >();
    auto over = MakePatternPtr< Delta<Statement> >();
    auto all_over = MakePatternPtr< Delta<Statement> >();
    auto sx_goto = MakePatternPtr<Goto>();
    auto r_goto = MakePatternPtr<Goto>();
    auto r_label = MakePatternPtr<Label>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("YIELD");
          
    all_over->through = all;
    all_over->overlay = anynode;
    all->conjuncts = (anynode, notmatch);
    anynode->terminus = over;
    over->through = wait;
    notmatch->negand = sx_comp;
    sx_comp->members = sx_decls;
    sx_comp->statements = (sx_pre, wait, sx_goto, sx_post);    
    
    over->overlay = r_comp;
    //r_comp->members = ();
    r_comp->statements = (wait, r_goto, r_label);
    r_goto->destination = r_labelid;
    r_label->identifier = r_labelid;
    
    Configure( SEARCH_REPLACE, all_over );
}
/*
GotoAfterWait::GotoAfterWait()
{
    // TODO will miss a yield at the very end
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto wait = MakePatternPtr<Wait>();
    auto notmatch = MakePatternPtr< Negation<Statement> >();
    auto sx_goto = MakePatternPtr<Goto>();
    auto r_goto = MakePatternPtr<Goto>();
    auto r_label = MakePatternPtr<Label>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("YIELD");
        
    s_comp->members = (decls);
    s_comp->statements = (pre, wait, notmatch, post);
    notmatch->negand = sx_goto;
    
    r_comp->members = (decls);
    r_comp->statements = (pre, wait, r_goto, r_label, notmatch, post);
    r_goto->destination = r_labelid;
    r_label->identifier = r_labelid;
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}
*/

NormaliseConditionalGotos::NormaliseConditionalGotos()
{
    auto iif = MakePatternPtr< If >();
    auto cond = MakePatternPtr< Expression >();
    auto s_comp = MakePatternPtr< Compound >();
    auto r_comp = MakePatternPtr< Compound >();
    auto sx_comp = MakePatternPtr< Compound >();
    auto then_goto = MakePatternPtr< Goto >();// TODO sx_goto could be any departure, like Return or Cease etc
    auto s_else_goto = MakePatternPtr< Goto >();
    auto r_goto = MakePatternPtr< Goto >();
    auto sx_goto = MakePatternPtr< Goto >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto sx_post = MakePatternPtr< Star<Statement> >();
    auto mult = MakePatternPtr< ConditionalOperator >();
    auto label = MakePatternPtr< Label >();
    auto label_id = MakePatternPtr< BuildLabelIdentifierAgent >("PROCEED");
    auto s_all = MakePatternPtr< Conjunction<Statement> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    
    s_all->conjuncts = (s_comp, sx_not);
    sx_not->negand = sx_comp;    
    iif->condition = cond;
    iif->body = then_goto;
    iif->else_body = MakePatternPtr<Nop>(); 
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, iif, post );    
    sx_comp->statements = ( sx_pre, iif, sx_goto, sx_post );    

    label->identifier = label_id;
    r_goto->destination = label_id;
    r_comp->members = ( decls );
    r_comp->statements = ( pre, iif, r_goto, label, post );
    
    Configure( SEARCH_REPLACE, s_all, r_comp );
}


CompactGotos::CompactGotos()
{
    auto s_if = MakePatternPtr< If >();
    auto cond = MakePatternPtr< Expression >();
    auto s_comp = MakePatternPtr< Compound >();
    auto r_comp = MakePatternPtr< Compound >();
    auto s_then_goto = MakePatternPtr< Goto >();
    auto s_else_goto = MakePatternPtr< Goto >();
    auto r_goto = MakePatternPtr< Goto >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto mult = MakePatternPtr< ConditionalOperator >();
    
    s_then_goto->destination = MakePatternPtr<Expression>();    
    s_else_goto->destination = MakePatternPtr<Expression>();
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakePatternPtr<Nop>(); // standed conditional branch has no else clause - our "else" is the next statement
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, s_if, s_else_goto, post );    

    mult->operands = (cond, s_then_goto->destination, s_else_goto->destination);
    r_goto->destination = mult;
    r_comp->statements = ( pre, r_goto, post );
    r_comp->members = ( decls );    
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


AddGotoBeforeLabel::AddGotoBeforeLabel() // TODO really slow!!11
{
    auto s_if = MakePatternPtr< If >();
    auto cond = MakePatternPtr< Expression >();
    auto s_comp = MakePatternPtr< Compound >();
    auto r_comp = MakePatternPtr< Compound >();
    auto sx_comp = MakePatternPtr< Compound >();
    auto r_goto = MakePatternPtr< Goto >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto sx_decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto sx_post = MakePatternPtr< Star<Statement> >();
    auto mult = MakePatternPtr< ConditionalOperator >();
    auto label = MakePatternPtr< Label >();
    auto label_id = MakePatternPtr< LabelIdentifier >();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_not = MakePatternPtr< Negation<Compound> >();
        
    s_all->conjuncts = (s_comp, s_not);
    s_not->negand = sx_comp;
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, label, post );    
    label->identifier = label_id;
    sx_comp->members = ( sx_decls );    
    sx_comp->statements = ( sx_pre, MakePatternPtr<Goto>(), label, sx_post );

    r_comp->members = ( decls );    
    r_comp->statements = ( pre, r_goto, label, post );
    r_goto->destination = label_id;

    Configure( SEARCH_REPLACE, s_all, r_comp );
}


static TreePtr<Statement> MakeResetAssignmentPattern()
{
    auto ass = MakePatternPtr<Assign>();
    auto decl = MakePatternPtr< TransformOf<InstanceIdentifier> >( &GetDeclaration::instance );
    decl->pattern = MakePatternPtr<LocalVariable>();
    ass->operands = (decl, MakePatternPtr<Literal>());    
    return ass;
}


EnsureBootstrap::EnsureBootstrap()
{
    auto fn = MakePatternPtr<Instance>();
    auto thread = MakePatternPtr<Thread>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_not = MakePatternPtr< Negation<Compound> >();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto sx_body = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto r_goto = MakePatternPtr<Goto>();
    auto r_label = MakePatternPtr<Label>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("BOOTSTRAP");
    auto stop = MakePatternPtr< Negation<Statement> >();
    auto sx_goto = MakePatternPtr<Goto>();
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->conjuncts = (s_not, s_body);
    s_not->negand = sx_body;
    // only exclude if there is a goto; a goto to anywhere will suffice to boot the state machine
    sx_body->members = (MakePatternPtr< Star<Declaration> >());
    sx_body->statements = (sx_pre, sx_goto, MakePatternPtr< Star<Statement> >());     
    sx_pre->restriction = MakeResetAssignmentPattern();
    sx_goto->destination = MakePatternPtr<LabelIdentifier>(); // must be a hard goto to exclude - otherwise might 
                                                           // have calculations in it which is no good for bootstrapping
    over->overlay = r_body;
    s_body->members = decls;
    s_body->statements = (pre, stop, post);
    pre->restriction = MakeResetAssignmentPattern();    
    stop->negand = MakeResetAssignmentPattern();
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;
    r_body->members = decls;
    r_body->statements = (pre, r_goto, r_label, stop, post);    

    Configure( SEARCH_REPLACE, fn );
}


AddStateLabelVar::AddStateLabelVar()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto lr_compound = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto ls_goto = MakePatternPtr<Goto>();
    auto lr_goto = MakePatternPtr<Goto>();
    auto sx_goto = MakePatternPtr<Goto>();
    auto lr_assign = MakePatternPtr<Assign>();
    auto state_var = MakePatternPtr<Automatic>();
    auto sx_not = MakePatternPtr< Negation<Expression> >();
    auto lsx_not = MakePatternPtr< Negation<Expression> >();
    auto state_var_id = MakePatternPtr< BuildInstanceIdentifierAgent >("state");
    
    ls_goto->destination = lsx_not;
    lsx_not->negand = state_var_id; //  MakePatternPtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lsx_not);
    lr_goto->destination = state_var_id;
            
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Statement> >( r_comp, ls_goto, lr_compound );
     
    s_comp->members = (decls);
    s_comp->statements = (pre, sx_goto, post); 
    sx_goto->destination = sx_not;
    sx_not->negand = MakePatternPtr<InstanceIdentifier>();
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, sx_goto, post); 
    state_var->identifier = state_var_id;
    state_var->type = MakePatternPtr<Labeley>();    
    state_var->initialiser = MakePatternPtr<Uninitialised>();

    Configure( SEARCH_REPLACE, s_comp, r_slave );
}


EnsureSuperLoop::EnsureSuperLoop()
{   
    auto fn = MakePatternPtr<Instance>();
    auto thread = MakePatternPtr<Thread>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto s_limit = MakePatternPtr< Negation<Statement> >();
    auto sx_stuff = MakePatternPtr< Stuff<Compound> >();
    auto sx_goto = MakePatternPtr< Goto >();
    auto first_goto = MakePatternPtr< Goto >();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto r_loop_body = MakePatternPtr<Compound>();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto r_loop = MakePatternPtr<Do>();
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->conjuncts = (sx_stuff, s_body);
    sx_stuff->terminus = sx_goto;
    sx_stuff->recurse_restriction = sx_not;
    sx_not->negand = MakePatternPtr<Do>();
    s_body->members = (decls);
    s_body->statements = (pre, first_goto, post);
    pre->restriction = s_limit;
    s_limit->negand = MakePatternPtr<Goto>();
    
    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (pre, r_loop);
    r_loop->body = r_loop_body;
    r_loop_body->statements = (first_goto, post);
    r_loop->condition = MakePatternPtr<True>();

    Configure( SEARCH_REPLACE, fn );
}

ShareGotos::ShareGotos()
{   
    auto loop = MakePatternPtr<Do>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto first_goto = MakePatternPtr< Goto >();
    auto r_goto = MakePatternPtr< Goto >();
    auto r_label = MakePatternPtr<Label>();
    auto r_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("ITERATE");
                    
    loop->body = over;
    loop->condition = MakePatternPtr<SpecificInteger>(1);
    over->through = s_body;
    s_body->members = (decls);
    s_body->statements = (first_goto, pre, first_goto, post);    

    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (first_goto, pre, r_goto, post, r_label);    
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;

    Configure( SEARCH_REPLACE, loop );
}


SwitchCleanUp::SwitchCleanUp()
{
    auto r_comp = MakePatternPtr<Compound>();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto s_switch = MakePatternPtr<Switch>();
    auto r_switch = MakePatternPtr<Switch>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto main = MakePatternPtr< Star<Statement> >();
    auto tail = MakePatternPtr< Star<Statement> >();
    auto label = MakePatternPtr<Label>();
    auto cond = MakePatternPtr<Expression>();
    auto sx_not_tail = MakePatternPtr< Negation<Statement> >();
    auto sx_not_main = MakePatternPtr< Negation<Statement> >();
    auto sx_any_tail = MakePatternPtr< Disjunction<Statement> >();

    s_switch->condition = cond;
    s_switch->body = s_body;
    s_body->members = decls;
    s_body->statements = (main, label, tail);
    main->restriction = sx_not_main;
    sx_not_main->negand = MakePatternPtr<Break>();
    tail->restriction = sx_not_tail;
    sx_not_tail->negand = sx_any_tail;
    sx_any_tail->disjuncts = (MakePatternPtr<Break>(), MakePatternPtr<Case>());
    
    r_comp->statements = (r_switch, label, tail);
    r_switch->condition = cond;
    r_switch->body = r_body;
    r_body->members = decls;
    r_body->statements = (main);    
    
    Configure( SEARCH_REPLACE, s_switch, r_comp );        
}


InferBreak::InferBreak()
{
    auto ls_goto = MakePatternPtr<Goto>();
    auto label = MakePatternPtr<Label>();
    auto label_id = MakePatternPtr<LabelIdentifier>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto breakable = MakePatternPtr<Breakable>();
    auto lr_break = MakePatternPtr<Break>();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();

    ls_goto->destination = label_id;

    auto slave = MakePatternPtr< SlaveSearchReplace<Breakable> >( breakable, ls_goto, lr_break ); // todo restrict to not go through more breakables

    s_comp->members = decls;
    s_comp->statements = (pre, breakable, label, post);
    label->identifier = label_id;
    
    r_comp->members = decls;
    r_comp->statements = (pre, slave, post); 
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );        
}


FixFallthrough::FixFallthrough()
{
    // don't actually need a switch statement here, just look in the body, pattern contains Case statements
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto cb1 = MakePatternPtr< Star<Statement> >();
    auto cb2 = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto case1 = MakePatternPtr<Case>();
    auto case2 = MakePatternPtr<Case>();
    auto breakk = MakePatternPtr<Break>();
    auto s_not1 = MakePatternPtr< Negation<Statement> >();
    auto s_not2 = MakePatternPtr< Negation<Statement> >();
    
    s_comp->members = (decls);
    s_comp->statements = (pre, case1, cb1,              case2, cb2, breakk, post);
    r_comp->members = (decls);
    r_comp->statements = (pre, case1, cb1, cb2, breakk, case2, cb2, breakk, post);
    cb1->restriction = s_not1;
    s_not1->negand = MakePatternPtr<Break>();
    cb2->restriction = s_not2;
    s_not2->negand = MakePatternPtr<Case>();
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );            
}


AddYieldFlag::AddYieldFlag()
{
    auto fn = MakePatternPtr<Instance>();
    auto fn_id = MakePatternPtr<InstanceIdentifier>();
    auto sub = MakePatternPtr<Callable>();
    auto s_func_comp = MakePatternPtr<Compound>();
    auto r_func_comp = MakePatternPtr<Compound>();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto ms_comp = MakePatternPtr<Compound>();
    auto mr_comp = MakePatternPtr<Compound>();
    auto msx_comp = MakePatternPtr<Compound>();
    auto enums = MakePatternPtr< Star<Declaration> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto m_decls = MakePatternPtr< Star<Declaration> >();
    auto msx_decls = MakePatternPtr< Star<Declaration> >();
    auto var_decl = MakePatternPtr<Instance>();
    auto var_id = MakePatternPtr<InstanceIdentifier>();
    auto enum_id = MakePatternPtr<TypeIdentifier>();
    auto func_pre = MakePatternPtr< Star<Statement> >();
    auto m_pre = MakePatternPtr< Star<Statement> >();
    auto msx_pre = MakePatternPtr< Star<Statement> >();
    auto m_post = MakePatternPtr< Star<Statement> >();
    auto msx_post = MakePatternPtr< Star<Statement> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto l_pre = MakePatternPtr< Star<If> >();
    auto l_post = MakePatternPtr< Star<If> >();
    auto loop = MakePatternPtr<Loop>();
    auto ls_if = MakePatternPtr<If>();
    auto lr_if = MakePatternPtr<If>();
    auto ms_if = MakePatternPtr<If>();
    auto mr_if = MakePatternPtr<If>();
    auto m_wait = MakePatternPtr<Wait>();
    auto enum_decl = MakePatternPtr<Enum>();
    auto l_equal = MakePatternPtr<Equal>();
    auto lr_and = MakePatternPtr<LogicalAnd>();
    auto lr_not = MakePatternPtr<LogicalNot>();
    auto func_over = MakePatternPtr< Delta<Compound> >();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto r_flag_decl = MakePatternPtr<Temporary>();
    auto r_flag_init = MakePatternPtr<Assign>();
    auto mr_assign = MakePatternPtr<Assign>();
    auto msx_assign = MakePatternPtr<Assign>();
    auto r_flag_id = MakePatternPtr<BuildInstanceIdentifierAgent>("yield_flag");
    auto ms_all = MakePatternPtr< Conjunction<Compound> >();
    auto ms_not = MakePatternPtr< Negation<Compound> >();
    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Compound> >( r_func_comp, ms_all, mr_comp );
    auto slave = MakePatternPtr< SlaveSearchReplace<Compound> >( r_comp, ls_if, lr_if );  
      
    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;  
    func_over->through = s_func_comp;    
    s_func_comp->members = (func_decls);
    s_func_comp->statements = (func_pre, loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = decls;
    s_comp->statements = (stmts);
    stmts->restriction = MakePatternPtr<If>(); // anti-spin
    func_over->overlay = slavem; 
    r_func_comp->members = (func_decls);
    r_flag_init->operands = (r_flag_id, MakePatternPtr<False>());
    r_func_comp->statements = (func_pre, loop);
    r_flag_decl->identifier = r_flag_id;
    r_flag_decl->type = MakePatternPtr<Boolean>();
    r_flag_decl->initialiser = MakePatternPtr<Uninitialised>();
    over->overlay = slave;
    r_comp->members = (decls, r_flag_decl);
    r_comp->statements = (r_flag_init, stmts);

    ls_if->condition = l_equal;
    l_equal->operands = (MakePatternPtr<InstanceIdentifier>(), MakePatternPtr<InstanceIdentifier>());
    // TODO yield_id should be of type enum_id?                         
    lr_if->condition = lr_and;
    lr_and->operands = (l_equal, lr_not);
    lr_not->operands = (r_flag_id);

    ms_all->conjuncts = (ms_comp, ms_not);
    ms_comp->members = (m_decls);
    ms_comp->statements = (m_pre, m_wait, m_post);
    ms_not->negand = msx_comp;
    msx_comp->members = msx_decls;
    msx_comp->statements = (msx_pre, msx_assign, msx_post);
    msx_assign->operands = (r_flag_id, MakePatternPtr<Bool>());

    mr_comp->members = (m_decls);
    mr_comp->statements = (m_pre, m_wait, mr_assign, m_post);
    mr_assign->operands = (r_flag_id, MakePatternPtr<True>());

    Configure( SEARCH_REPLACE, fn );            
}

AddInferredYield::AddInferredYield()
{
    auto fn = MakePatternPtr<Instance>();
    auto fn_id = MakePatternPtr<InstanceIdentifier>();
    auto thread = MakePatternPtr<Thread>(); // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    auto func_comp = MakePatternPtr<Compound>();
    auto s_comp = MakePatternPtr<Compound>();
    auto sx_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto func_pre = MakePatternPtr< Star<Statement> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto over = MakePatternPtr< Delta<Statement> >();
    auto flag_decl = MakePatternPtr<LocalVariable>();
    auto flag_id = MakePatternPtr<InstanceIdentifier>();
    auto r_yield = MakePatternPtr<WaitDelta>();
    auto loop = MakePatternPtr<Loop>();
    auto r_if = MakePatternPtr<If>();
    auto sx_if = MakePatternPtr<If>();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_notmatch = MakePatternPtr< Negation<Compound> >();
    auto r_not = MakePatternPtr< LogicalNot >();
    auto sx_not = MakePatternPtr< LogicalNot >();
    auto assign = MakePatternPtr< Assign >();
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    flag_decl->type = MakePatternPtr<Boolean>();
    flag_decl->initialiser = MakePatternPtr<Uninitialised>();
    flag_decl->identifier = flag_id;
    func_comp->statements = (func_pre, loop);
    loop->body = over;
    over->through = s_all;
    s_all->conjuncts = (s_comp, s_notmatch);
    s_comp->members = (flag_decl);
    s_comp->statements = (stmts);
    s_notmatch->negand = sx_comp;
    sx_comp->members = (flag_decl);
    sx_comp->statements = (sx_pre, sx_if);
    sx_if->condition = sx_not;
    sx_not->operands = (flag_id);
    
    over->overlay = r_comp;
    r_comp->members = (flag_decl);
    r_comp->statements = (stmts, r_if);
    r_if->condition = r_not;
    r_not->operands = (flag_id);
    r_if->body = r_yield;
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure( SEARCH_REPLACE, fn );            
}


MoveInitIntoSuperLoop::MoveInitIntoSuperLoop()
{
    auto fn = MakePatternPtr<Instance>();
    auto fn_id = MakePatternPtr<InstanceIdentifier>();
    auto thread = MakePatternPtr<Thread>(); // Must be SC_THREAD since we introduce SC stuff
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto inits = MakePatternPtr< Star<Statement> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto loop = MakePatternPtr<Loop>();
    auto s_func_comp = MakePatternPtr<Compound>();
    auto r_func_comp = MakePatternPtr<Compound>();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_if_comp = MakePatternPtr<Compound>();
    auto r_if = MakePatternPtr<If>();
    auto r_equal = MakePatternPtr<Equal>();
    auto func_over = MakePatternPtr< Delta<Compound> >();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto first_init = MakePatternPtr<Statement>();
                    
    fn->type = thread;
    fn->initialiser = func_over;
    fn->identifier = fn_id;  
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls);
    s_func_comp->statements = (first_init, inits, loop);
    loop->body = over;
    over->through = s_comp;
//    s_comp->members = ();
    s_comp->statements = (stmts);    
    
    func_over->overlay = r_func_comp;
    r_func_comp->members = (func_decls);
    r_func_comp->statements = (loop);
    over->overlay = r_comp;
//    r_comp->members = ();
    r_comp->statements = (r_if, stmts);
    r_if->condition = r_equal;
    r_equal->operands = ( MakePatternPtr<DeltaCount>(), MakePatternPtr<SpecificInteger>(0) );    
    r_if->body = r_if_comp;
//    r_if_comp->members = ();
    r_if_comp->statements = (first_init, inits);//, MakePatternPtr<WaitDelta>());
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure( SEARCH_REPLACE, fn );            
}


// rotate loops to avoid inferred yields when an explicit yield already exists
LoopRotation::LoopRotation()
{
    auto fn = MakePatternPtr<Instance>();
    auto s_var_decl = MakePatternPtr<Instance>();
    auto fn_id = MakePatternPtr<InstanceIdentifier>();
    auto s_var_id = MakePatternPtr<InstanceIdentifier>();
    auto s_cur_enum_id = MakePatternPtr<InstanceIdentifier>();
    auto s_outer_enum_id = MakePatternPtr<InstanceIdentifier>();
    auto thread = MakePatternPtr<Thread>(); // Must be SC_THREAD since we introduce SC stuff
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto s_enums = MakePatternPtr< Star<Declaration> >();
    auto comp_loop_decls = MakePatternPtr< Star<Declaration> >();
    auto comp_yield_decls = MakePatternPtr< Star<Declaration> >();
    auto x_comp_decls = MakePatternPtr< Star<Declaration> >();
    auto s_cur_enum = MakePatternPtr<Static>();
    auto s_outer_enum = MakePatternPtr<Static>();
    auto inits = MakePatternPtr< Star<Statement> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto prepre = MakePatternPtr< Star<Statement> >();
    auto prepost = MakePatternPtr< Star<Statement> >();
    auto postpre = MakePatternPtr< Star<Statement> >();
    auto postpost = MakePatternPtr< Star<Statement> >();
    auto comp_loop_pre = MakePatternPtr< Star<Statement> >();
    auto comp_loop_post = MakePatternPtr< Star<Statement> >();
    auto comp_yield_pre = MakePatternPtr< Star<Statement> >();
    auto comp_yield_post = MakePatternPtr< Star<Statement> >();
    auto loop = MakePatternPtr<Loop>();
    auto func_comp = MakePatternPtr<Compound>();
    auto s_comp_loop = MakePatternPtr<Compound>();
    auto s_comp_yield = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_if_comp = MakePatternPtr<Compound>();
    auto sx_comp = MakePatternPtr<Compound>();
    auto loop_top = MakePatternPtr<If>();
    auto loop_bottom = MakePatternPtr<If>();
    auto yield = MakePatternPtr<If>();
    auto outer_bottom = MakePatternPtr<If>();
    auto outer_top = MakePatternPtr<If>();
    auto loop_body = MakePatternPtr< Star<If> >();
    auto pre_yield = MakePatternPtr< Star<If> >();
    auto post_yield = MakePatternPtr< Star<If> >();
    auto r_equal = MakePatternPtr<Equal>();
    auto func_over = MakePatternPtr< Delta<Compound> >();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_enum = MakePatternPtr<Enum>();
    auto s_enum_id = MakePatternPtr<TypeIdentifier>();
    auto loop_top_stuff = MakePatternPtr< Stuff<Expression> >();
    auto outer_top_stuff = MakePatternPtr< Stuff<Expression> >();
    auto loop_top_equal = MakePatternPtr<Equal>();
    auto outer_top_equal = MakePatternPtr<Equal>();
    auto loop_bottom_stuff_enum = MakePatternPtr< Stuff<Statement> >();
    auto outer_bottom_stuff_enum = MakePatternPtr< Stuff<Statement> >();
    auto loop_bottom_stuff_noyield = MakePatternPtr< Stuff<Statement> >();
    auto yield_stuff = MakePatternPtr< Stuff<Statement> >();
    auto outer_bottom_stuff_noyield = MakePatternPtr< Stuff<Statement> >();
    auto loop_bottom_matchall = MakePatternPtr< Conjunction<Statement> >();
    auto outer_bottom_matchall = MakePatternPtr< Conjunction<Statement> >();
    auto loop_bottom_notmatch = MakePatternPtr< Negation<Statement> >();
    auto outer_bottom_notmatch = MakePatternPtr< Negation<Statement> >();
    auto s_notmatch = MakePatternPtr< Negation<Compound> >();
    auto inner_state = MakePatternPtr< Disjunction<If> >();
    
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls, /*s_enum,*/ s_var_decl); // enum removed because it is class member, not local to the function
    s_enum->identifier = s_enum_id;
    s_enum->members = (s_enums, s_cur_enum);
    s_cur_enum->identifier = s_cur_enum_id;
    s_outer_enum->identifier = s_outer_enum_id;
    s_var_decl->type = s_enum_id;
    s_var_decl->identifier = s_var_id;
    func_comp->statements = (inits, loop);
    loop->body = over;
    over->through = s_all;
    s_all->conjuncts = (s_comp_loop, s_comp_yield, s_notmatch);
    s_comp_loop->members = (comp_loop_decls);
    // Search for a loop. Assume that a state enum value in a body means "could transition to the state" and one in
    // the condition means "acts on the state". If we see the latter with the former blow it somewhere, we call
    // it a loop and assume the upward branch is normally takner as with C compilers.    
    s_comp_loop->statements = (comp_loop_pre, loop_top, loop_body, loop_bottom, comp_loop_post);    
    loop_top->condition = loop_top_stuff;
    loop_top_stuff->terminus = loop_top_equal;
    loop_top_equal->operands = (s_var_id, s_cur_enum_id);
    loop_bottom->body = loop_bottom_matchall;
    loop_bottom_matchall->conjuncts = (loop_bottom_stuff_enum, loop_bottom_notmatch);
    loop_bottom_stuff_enum->terminus = s_cur_enum_id;
    loop_bottom_notmatch->negand = loop_bottom_stuff_noyield;
    loop_bottom_stuff_noyield->terminus = MakePatternPtr<Wait>();    
    s_comp_yield->members = (comp_yield_decls);
    
    // We need to restruct to loops that contain a yield anywhere but the bottom - these are the ones
    // that would benefit from loop rotation.
    s_comp_yield->statements = (comp_yield_pre, pre_yield, yield, post_yield, loop_bottom, comp_yield_post);    
    yield->body = yield_stuff;
    yield_stuff->terminus = MakePatternPtr<Wait>();
    
    // This part is to make sure we operate on the outermost loop first - look for another loop surrounding 
    // the current one, if it does not end in a yield, then do not transform. This way the outer loop will
    // keep getting hits until the yield is at the bottom, then inner loops can have a go.
    s_notmatch->negand = sx_comp;
    sx_comp->members = (x_comp_decls);
    sx_comp->statements = (prepre, outer_top, postpre, inner_state, prepost, outer_bottom, postpost);
    outer_top->condition = outer_top_stuff;
    outer_top_stuff->terminus = outer_top_equal;
    outer_top_equal->operands = (s_var_id, s_outer_enum_id);
    outer_bottom->body = outer_bottom_matchall;
    outer_bottom_matchall->conjuncts = (outer_bottom_stuff_enum, outer_bottom_notmatch);
    outer_bottom_stuff_enum->terminus = s_outer_enum_id;
    outer_bottom_notmatch->negand = outer_bottom_stuff_noyield;
    outer_bottom_stuff_noyield->terminus = MakePatternPtr<Wait>();    
    inner_state->disjuncts = (loop_top, loop_bottom); // outer loop can share top or bottom state with inner loop; but not both, so at least one must be here
   
    over->overlay = r_comp;
    r_comp->members = (comp_loop_decls);
    r_comp->statements = (comp_loop_pre, loop_bottom, loop_top, loop_body, comp_loop_post);    // rotated version of s_comp_loop
        
    Configure( SEARCH_REPLACE, fn );            
}


