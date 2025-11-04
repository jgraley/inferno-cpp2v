
#include "steps/state_out.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "pattern_helpers.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"

 
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
    auto sx_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto sx_decls = MakePatternNode< Star<Declaration> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto sx_post = MakePatternNode< Star<Statement> >();
    auto wait = MakePatternNode<Wait>();
    auto notmatch = MakePatternNode< Negation<Statement> >();
    auto all = MakePatternNode< Conjunction<Statement> >();
    auto anynode = MakePatternNode< Child<Statement> >();
    auto over = MakePatternNode< Delta<Statement> >();
    auto all_over = MakePatternNode< Delta<Statement> >();
    auto sx_goto = MakePatternNode<Goto>();
    auto r_goto = MakePatternNode<Goto>();
    auto r_label = MakePatternNode<Label>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("YIELD");
          
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
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto wait = MakePatternNode<Wait>();
    auto notmatch = MakePatternNode< Negation<Statement> >();
    auto sx_goto = MakePatternNode<Goto>();
    auto r_goto = MakePatternNode<Goto>();
    auto r_label = MakePatternNode<Label>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("YIELD");
        
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
    auto iif = MakePatternNode< If >();
    auto cond = MakePatternNode< Expression >();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto sx_comp = MakePatternNode< Compound >();
    auto then_goto = MakePatternNode< Goto >();// TODO sx_goto could be any departure, like Return or Cease etc
    auto s_else_goto = MakePatternNode< Goto >();
    auto r_goto = MakePatternNode< Goto >();
    auto sx_goto = MakePatternNode< Goto >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto sx_post = MakePatternNode< Star<Statement> >();
    auto label = MakePatternNode< Label >();
    auto label_id = MakePatternNode< BuildLabelIdentifierAgent >("PROCEED");
    auto s_all = MakePatternNode< Conjunction<Statement> >();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    
    s_all->conjuncts = (s_comp, sx_not);
    sx_not->negand = sx_comp;    
    iif->condition = cond;
    iif->body = then_goto;
    iif->body_else = MakePatternNode<Nop>(); 
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, iif, post );    
    sx_comp->members = ( decls );    
    sx_comp->statements = ( sx_pre, iif, sx_goto, sx_post );    

    label->identifier = label_id;
    r_goto->destination = label_id;
    r_comp->members = ( decls );
    r_comp->statements = ( pre, iif, r_goto, label, post );
    
    Configure( SEARCH_REPLACE, s_all, r_comp );
}


CompactGotos::CompactGotos()
{
    auto s_if = MakePatternNode< If >();
    auto cond = MakePatternNode< Expression >();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto s_then_goto = MakePatternNode< Goto >();
    auto s_else_goto = MakePatternNode< Goto >();
    auto r_goto = MakePatternNode< Goto >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto multiplexer = MakePatternNode< ConditionalOperator >();
    
    s_then_goto->destination = MakePatternNode<Expression>();    
    s_else_goto->destination = MakePatternNode<Expression>();
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->body_else = MakePatternNode<Nop>(); // standed conditional branch has no else clause - our "else" is the next statement
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, s_if, s_else_goto, post );    

    multiplexer->condition = cond;
    multiplexer->expr_then = s_then_goto->destination;
    multiplexer->expr_else = s_else_goto->destination;
    r_goto->destination = multiplexer;
    r_comp->statements = ( pre, r_goto, post );
    r_comp->members = ( decls );    
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


AddGotoBeforeLabel::AddGotoBeforeLabel() // TODO really slow!!11
{
    auto s_if = MakePatternNode< If >();
    auto cond = MakePatternNode< Expression >();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto sx_comp = MakePatternNode< Compound >();
    auto r_goto = MakePatternNode< Goto >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto sx_decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto sx_post = MakePatternNode< Star<Statement> >();
    auto label = MakePatternNode< Label >();
    auto label_id = MakePatternNode< LabelIdentifier >();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_not = MakePatternNode< Negation<Compound> >();
        
    s_all->conjuncts = (s_comp, s_not);
    s_not->negand = sx_comp;
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, label, post );    
    label->identifier = label_id;
    sx_comp->members = ( sx_decls );    
    sx_comp->statements = ( sx_pre, MakePatternNode<Goto>(), label, sx_post );

    r_comp->members = ( decls );    
    r_comp->statements = ( pre, r_goto, label, post );
    r_goto->destination = label_id;

    Configure( SEARCH_REPLACE, s_all, r_comp );
}


static TreePtr<Statement> MakeResetAssignmentPattern()
{
    auto ass = MakePatternNode<Assign>();
    auto decl = MakePatternNode< TransformOf<InstanceIdentifier> >( &DeclarationOf::instance );
    decl->pattern = MakePatternNode<LocalVariable>();
    ass->operands = (decl, MakePatternNode<Literal>());    
    return ass;
}


EnsureBootstrap::EnsureBootstrap()
{
    auto fn = MakePatternNode<Instance>();
    auto thread = MakePatternNode<Thread>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_not = MakePatternNode< Negation<Compound> >();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto sx_body = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto r_goto = MakePatternNode<Goto>();
    auto r_label = MakePatternNode<Label>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("BOOTSTRAP");
    auto stop = MakePatternNode< Negation<Statement> >();
    auto sx_goto = MakePatternNode<Goto>();
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->conjuncts = (s_not, s_body);
    s_not->negand = sx_body;
    // only exclude if there is a goto; a goto to anywhere will suffice to boot the state machine
    sx_body->members = (MakePatternNode< Star<Declaration> >());
    sx_body->statements = (sx_pre, sx_goto, MakePatternNode< Star<Statement> >());     
    sx_pre->restriction = MakeResetAssignmentPattern();
    sx_goto->destination = MakePatternNode<LabelIdentifier>(); // must be a hard goto to exclude - otherwise might 
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
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto lr_compound = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto ls_goto = MakePatternNode<Goto>();
    auto lr_goto = MakePatternNode<Goto>();
    auto sx_goto = MakePatternNode<Goto>();
    auto lr_assign = MakePatternNode<Assign>();
    auto state_var = MakePatternNode<Automatic>();
    auto sx_not = MakePatternNode< Negation<Expression> >();
    auto lsx_not = MakePatternNode< Negation<Expression> >();
    auto state_var_id = MakePatternNode< BuildInstanceIdentifierAgent >("state");
    
    ls_goto->destination = lsx_not;
    lsx_not->negand = state_var_id; //  MakePatternNode<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lsx_not);
    lr_goto->destination = state_var_id;
            
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Statement> >( r_comp, ls_goto, lr_compound );
     
    s_comp->members = (decls);
    s_comp->statements = (pre, sx_goto, post); 
    sx_goto->destination = sx_not;
    sx_not->negand = MakePatternNode<InstanceIdentifier>();
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, sx_goto, post); 
    state_var->identifier = state_var_id;
    state_var->type = MakePatternNode<Labeley>();    
    state_var->initialiser = MakePatternNode<Uninitialised>();

    Configure( SEARCH_REPLACE, s_comp, r_embedded );
}


EnsureSuperLoop::EnsureSuperLoop()
{   
    auto fn = MakePatternNode<Instance>();
    auto thread = MakePatternNode<Thread>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    auto s_limit = MakePatternNode< Negation<Statement> >();
    auto sx_stuff = MakePatternNode< Stuff<Compound> >();
    auto sx_goto = MakePatternNode< Goto >();
    auto first_goto = MakePatternNode< Goto >();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto r_loop_body = MakePatternNode<Compound>();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto r_loop = MakePatternNode<Do>();
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->conjuncts = (sx_stuff, s_body);
    sx_stuff->terminus = sx_goto;
    sx_stuff->recurse_restriction = sx_not;
    sx_not->negand = MakePatternNode<Do>();
    s_body->members = (decls);
    s_body->statements = (pre, first_goto, post);
    pre->restriction = s_limit;
    s_limit->negand = MakePatternNode<Goto>();
    
    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (pre, r_loop);
    r_loop->body = r_loop_body;
    r_loop_body->statements = (first_goto, post);
    r_loop->condition = MakePatternNode<True>();

    Configure( SEARCH_REPLACE, fn );
}

ShareGotos::ShareGotos()
{   
    auto loop = MakePatternNode<Do>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto first_goto = MakePatternNode< Goto >();
    auto r_goto = MakePatternNode< Goto >();
    auto r_label = MakePatternNode<Label>();
    auto r_labelid = MakePatternNode<BuildLabelIdentifierAgent>("ITERATE");
                    
    loop->body = over;
    loop->condition = MakePatternNode<SpecificInteger>(1);
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
    auto r_comp = MakePatternNode<Compound>();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto s_switch = MakePatternNode<Switch>();
    auto r_switch = MakePatternNode<Switch>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto main = MakePatternNode< Star<Statement> >();
    auto tail = MakePatternNode< Star<Statement> >();
    auto label = MakePatternNode<Label>();
    auto cond = MakePatternNode<Expression>();
    auto sx_not_tail = MakePatternNode< Negation<Statement> >();
    auto sx_not_main = MakePatternNode< Negation<Statement> >();
    auto sx_any_tail = MakePatternNode< Disjunction<Statement> >();

    s_switch->condition = cond;
    s_switch->body = s_body;
    s_body->members = decls;
    s_body->statements = (main, label, tail);
    main->restriction = sx_not_main;
    sx_not_main->negand = MakePatternNode<Break>();
    tail->restriction = sx_not_tail;
    sx_not_tail->negand = sx_any_tail;
    sx_any_tail->disjuncts = (MakePatternNode<Break>(), MakePatternNode<Case>());
    
    r_comp->statements = (r_switch, label, tail);
    r_switch->condition = cond;
    r_switch->body = r_body;
    r_body->members = decls;
    r_body->statements = (main);    
    
    Configure( SEARCH_REPLACE, s_switch, r_comp );        
}


InferBreak::InferBreak()
{
    auto ls_goto = MakePatternNode<Goto>();
    auto label = MakePatternNode<Label>();
    auto label_id = MakePatternNode<LabelIdentifier>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto breakable = MakePatternNode<Breakable>();
    auto lr_break = MakePatternNode<Break>();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();

    ls_goto->destination = label_id;

    auto embedded = MakePatternNode< EmbeddedSearchReplace<Breakable> >( breakable, ls_goto, lr_break ); // todo restrict to not go through more breakables

    s_comp->members = decls;
    s_comp->statements = (pre, breakable, label, post);
    label->identifier = label_id;
    
    r_comp->members = decls;
    r_comp->statements = (pre, embedded, post); 
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );        
}


FixFallthrough::FixFallthrough()
{
    // don't actually need a switch statement here, just look in the body, pattern contains Case statements
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto cb1 = MakePatternNode< Star<Statement> >();
    auto cb2 = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto case1 = MakePatternNode<Case>();
    auto case2 = MakePatternNode<Case>();
    auto breakk = MakePatternNode<Break>();
    auto s_not1 = MakePatternNode< Negation<Statement> >();
    auto s_not2 = MakePatternNode< Negation<Statement> >();
    
    s_comp->members = (decls);
    s_comp->statements = (pre, case1, cb1,              case2, cb2, breakk, post);
    r_comp->members = (decls);
    r_comp->statements = (pre, case1, cb1, cb2, breakk, case2, cb2, breakk, post);
    cb1->restriction = s_not1;
    s_not1->negand = MakePatternNode<Break>();
    cb2->restriction = s_not2;
    s_not2->negand = MakePatternNode<Case>();
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );            
}


AddYieldFlag::AddYieldFlag()
{
    auto fn = MakePatternNode<Instance>();
    auto fn_id = MakePatternNode<InstanceIdentifier>();
    auto sub = MakePatternNode<Callable>();
    auto s_func_comp = MakePatternNode<Compound>();
    auto r_func_comp = MakePatternNode<Compound>();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto ms_comp = MakePatternNode<Compound>();
    auto mr_comp = MakePatternNode<Compound>();
    auto msx_comp = MakePatternNode<Compound>();
    auto enums = MakePatternNode< Star<Declaration> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto m_decls = MakePatternNode< Star<Declaration> >();
    auto msx_decls = MakePatternNode< Star<Declaration> >();
    auto var_decl = MakePatternNode<Instance>();
    auto var_id = MakePatternNode<InstanceIdentifier>();
    auto enum_id = MakePatternNode<TypeIdentifier>();
    auto func_pre = MakePatternNode< Star<Statement> >();
    auto m_pre = MakePatternNode< Star<Statement> >();
    auto msx_pre = MakePatternNode< Star<Statement> >();
    auto m_post = MakePatternNode< Star<Statement> >();
    auto msx_post = MakePatternNode< Star<Statement> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto l_pre = MakePatternNode< Star<If> >();
    auto l_post = MakePatternNode< Star<If> >();
    auto loop = MakePatternNode<Loop>();
    auto ls_if = MakePatternNode<If>();
    auto lr_if = MakePatternNode<If>();
    auto ms_if = MakePatternNode<If>();
    auto mr_if = MakePatternNode<If>();
    auto m_wait = MakePatternNode<Wait>();
    auto enum_decl = MakePatternNode<Enum>();
    auto l_equal = MakePatternNode<Equal>();
    auto lr_and = MakePatternNode<LogicalAnd>();
    auto lr_not = MakePatternNode<LogicalNot>();
    auto func_over = MakePatternNode< Delta<Compound> >();
    auto over = MakePatternNode< Delta<Compound> >();
    auto r_flag_decl = MakePatternNode<Temporary>();
    auto r_flag_init = MakePatternNode<Assign>();
    auto mr_assign = MakePatternNode<Assign>();
    auto msx_assign = MakePatternNode<Assign>();
    auto r_flag_id = MakePatternNode<BuildInstanceIdentifierAgent>("yield_flag");
    auto ms_all = MakePatternNode< Conjunction<Compound> >();
    auto ms_not = MakePatternNode< Negation<Compound> >();
    
    auto embedded_m = MakePatternNode< EmbeddedSearchReplace<Compound> >( r_func_comp, ms_all, mr_comp );
    auto embedded = MakePatternNode< EmbeddedSearchReplace<Compound> >( r_comp, ls_if, lr_if );  
      
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
    stmts->restriction = MakePatternNode<If>(); // anti-spin
    func_over->overlay = embedded_m; 
    r_func_comp->members = (func_decls);
    r_flag_init->operands = (r_flag_id, MakePatternNode<False>());
    r_func_comp->statements = (func_pre, loop);
    r_flag_decl->identifier = r_flag_id;
    r_flag_decl->type = MakePatternNode<Boolean>();
    r_flag_decl->initialiser = MakePatternNode<Uninitialised>();
    over->overlay = embedded;
    r_comp->members = (decls, r_flag_decl);
    r_comp->statements = (r_flag_init, stmts);

    ls_if->condition = l_equal;
    l_equal->operands = (MakePatternNode<InstanceIdentifier>(), MakePatternNode<InstanceIdentifier>());
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
    msx_assign->operands = (r_flag_id, MakePatternNode<Bool>());

    mr_comp->members = (m_decls);
    mr_comp->statements = (m_pre, m_wait, mr_assign, m_post);
    mr_assign->operands = (r_flag_id, MakePatternNode<True>());

    Configure( SEARCH_REPLACE, fn );            
}

AddInferredYield::AddInferredYield()
{
    auto fn = MakePatternNode<Instance>();
    auto fn_id = MakePatternNode<InstanceIdentifier>();
    auto thread = MakePatternNode<Thread>(); // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    auto func_comp = MakePatternNode<Compound>();
    auto s_comp = MakePatternNode<Compound>();
    auto sx_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto func_pre = MakePatternNode< Star<Statement> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto over = MakePatternNode< Delta<Statement> >();
    auto flag_decl = MakePatternNode<LocalVariable>();
    auto flag_id = MakePatternNode<InstanceIdentifier>();
    auto r_yield = MakePatternNode<WaitDelta>();
    auto loop = MakePatternNode<Loop>();
    auto r_if = MakePatternNode<If>();
    auto sx_if = MakePatternNode<If>();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_notmatch = MakePatternNode< Negation<Compound> >();
    auto r_not = MakePatternNode< LogicalNot >();
    auto sx_not = MakePatternNode< LogicalNot >();
    auto assign = MakePatternNode< Assign >();
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    flag_decl->type = MakePatternNode<Boolean>();
    flag_decl->initialiser = MakePatternNode<Uninitialised>();
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
    r_if->body_else = MakePatternNode<Nop>();
    
    Configure( SEARCH_REPLACE, fn );            
}


MoveInitIntoSuperLoop::MoveInitIntoSuperLoop()
{
    auto fn = MakePatternNode<Instance>();
    auto fn_id = MakePatternNode<InstanceIdentifier>();
    auto thread = MakePatternNode<Thread>(); // Must be SC_THREAD since we introduce SC stuff
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto inits = MakePatternNode< Star<Statement> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto loop = MakePatternNode<Loop>();
    auto s_func_comp = MakePatternNode<Compound>();
    auto r_func_comp = MakePatternNode<Compound>();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_if_comp = MakePatternNode<Compound>();
    auto r_if = MakePatternNode<If>();
    auto r_equal = MakePatternNode<Equal>();
    auto func_over = MakePatternNode< Delta<Compound> >();
    auto over = MakePatternNode< Delta<Compound> >();
    auto first_init = MakePatternNode<Statement>();
                    
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
    r_equal->operands = ( MakePatternNode<DeltaCount>(), MakePatternNode<SpecificInteger>(0) );    
    r_if->body = r_if_comp;
//    r_if_comp->members = ();
    r_if_comp->statements = (first_init, inits);//, MakePatternNode<WaitDelta>());
    r_if->body_else = MakePatternNode<Nop>();
    
    Configure( SEARCH_REPLACE, fn );            
}


// rotate loops to avoid inferred yields when an explicit yield already exists
LoopRotation::LoopRotation()
{
    auto fn = MakePatternNode<Instance>();
    auto s_var_decl = MakePatternNode<Instance>();
    auto fn_id = MakePatternNode<InstanceIdentifier>();
    auto s_var_id = MakePatternNode<InstanceIdentifier>();
    auto s_cur_enum_id = MakePatternNode<InstanceIdentifier>();
    auto s_outer_enum_id = MakePatternNode<InstanceIdentifier>();
    auto thread = MakePatternNode<Thread>(); // Must be SC_THREAD since we introduce SC stuff
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto s_enums = MakePatternNode< Star<Declaration> >();
    auto comp_loop_decls = MakePatternNode< Star<Declaration> >();
    auto comp_yield_decls = MakePatternNode< Star<Declaration> >();
    auto x_comp_decls = MakePatternNode< Star<Declaration> >();
    auto s_cur_enum = MakePatternNode<Static>();
    auto s_outer_enum = MakePatternNode<Static>();
    auto inits = MakePatternNode< Star<Statement> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto prepre = MakePatternNode< Star<Statement> >();
    auto prepost = MakePatternNode< Star<Statement> >();
    auto postpre = MakePatternNode< Star<Statement> >();
    auto postpost = MakePatternNode< Star<Statement> >();
    auto comp_loop_pre = MakePatternNode< Star<Statement> >();
    auto comp_loop_post = MakePatternNode< Star<Statement> >();
    auto comp_yield_pre = MakePatternNode< Star<Statement> >();
    auto comp_yield_post = MakePatternNode< Star<Statement> >();
    auto loop = MakePatternNode<Loop>();
    auto func_comp = MakePatternNode<Compound>();
    auto s_comp_loop = MakePatternNode<Compound>();
    auto s_comp_yield = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_if_comp = MakePatternNode<Compound>();
    auto sx_comp = MakePatternNode<Compound>();
    auto loop_top = MakePatternNode<If>();
    auto loop_bottom = MakePatternNode<If>();
    auto yield = MakePatternNode<If>();
    auto outer_bottom = MakePatternNode<If>();
    auto outer_top = MakePatternNode<If>();
    auto loop_body = MakePatternNode< Star<If> >();
    auto pre_yield = MakePatternNode< Star<If> >();
    auto post_yield = MakePatternNode< Star<If> >();
    auto r_equal = MakePatternNode<Equal>();
    auto func_over = MakePatternNode< Delta<Compound> >();
    auto over = MakePatternNode< Delta<Compound> >();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_enum = MakePatternNode<Enum>();
    auto s_enum_id = MakePatternNode<TypeIdentifier>();
    auto loop_top_stuff = MakePatternNode< Stuff<Expression> >();
    auto outer_top_stuff = MakePatternNode< Stuff<Expression> >();
    auto loop_top_equal = MakePatternNode<Equal>();
    auto outer_top_equal = MakePatternNode<Equal>();
    auto loop_bottom_stuff_enum = MakePatternNode< Stuff<Statement> >();
    auto outer_bottom_stuff_enum = MakePatternNode< Stuff<Statement> >();
    auto loop_bottom_stuff_noyield = MakePatternNode< Stuff<Statement> >();
    auto yield_stuff = MakePatternNode< Stuff<Statement> >();
    auto outer_bottom_stuff_noyield = MakePatternNode< Stuff<Statement> >();
    auto loop_bottom_matchall = MakePatternNode< Conjunction<Statement> >();
    auto outer_bottom_matchall = MakePatternNode< Conjunction<Statement> >();
    auto loop_bottom_notmatch = MakePatternNode< Negation<Statement> >();
    auto outer_bottom_notmatch = MakePatternNode< Negation<Statement> >();
    auto s_notmatch = MakePatternNode< Negation<Compound> >();
    auto inner_state = MakePatternNode< Disjunction<If> >();
    
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
    loop_bottom_stuff_noyield->terminus = MakePatternNode<Wait>();    
    s_comp_yield->members = (comp_yield_decls);
    
    // We need to restruct to loops that contain a yield anywhere but the bottom - these are the ones
    // that would benefit from loop rotation.
    s_comp_yield->statements = (comp_yield_pre, pre_yield, yield, post_yield, loop_bottom, comp_yield_post);    
    yield->body = yield_stuff;
    yield_stuff->terminus = MakePatternNode<Wait>();
    
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
    outer_bottom_stuff_noyield->terminus = MakePatternNode<Wait>();    
    inner_state->disjuncts = (loop_top, loop_bottom); // outer loop can share top or bottom state with inner loop; but not both, so at least one must be here
   
    over->overlay = r_comp;
    r_comp->members = (comp_loop_decls);
    r_comp->statements = (comp_loop_pre, loop_bottom, loop_top, loop_body, comp_loop_post);    // rotated version of s_comp_loop
        
    Configure( SEARCH_REPLACE, fn );            
}


