
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
    MakePatternPtr<Compound> sx_comp, r_comp;
    MakePatternPtr< Star<Declaration> > sx_decls;
    MakePatternPtr< Star<Statement> > sx_pre, sx_post;    
    MakePatternPtr<Wait> wait;
    MakePatternPtr< Negation<Statement> > notmatch;
    MakePatternPtr< Conjunction<Statement> > all;
    MakePatternPtr< AnyNode<Statement> > anynode;
    MakePatternPtr< Overlay<Statement> > over, all_over;
    MakePatternPtr<Goto> sx_goto, r_goto;
    MakePatternPtr<Label> r_label;
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("YIELD");
          
    all_over->through = all;
    all_over->overlay = anynode;
    all->patterns = (anynode, notmatch);
    anynode->terminus = over;
    over->through = wait;
    notmatch->pattern = sx_comp;
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
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Wait> wait;
    MakePatternPtr< Negation<Statement> > notmatch;
    MakePatternPtr<Goto> sx_goto, r_goto;
    MakePatternPtr<Label> r_label;
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("YIELD");
        
    s_comp->members = (decls);
    s_comp->statements = (pre, wait, notmatch, post);
    notmatch->pattern = sx_goto;
    
    r_comp->members = (decls);
    r_comp->statements = (pre, wait, r_goto, r_label, notmatch, post);
    r_goto->destination = r_labelid;
    r_label->identifier = r_labelid;
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}
*/

NormaliseConditionalGotos::NormaliseConditionalGotos()
{
    MakePatternPtr< If > iif;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp, sx_comp;  
    MakePatternPtr< Goto > then_goto, s_else_goto, r_goto, sx_goto;// TODO sx_goto could be any departure, like Return or Cease etc
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, sx_pre, post, sx_post;
    MakePatternPtr< ConditionalOperator > mult;
    MakePatternPtr< Label > label;    
    MakePatternPtr< BuildLabelIdentifierAgent > label_id("PROCEED");
    MakePatternPtr< Conjunction<Statement> > s_all;
    MakePatternPtr< Negation<Statement> > sx_not;    
    
    s_all->patterns = (s_comp, sx_not);
    sx_not->pattern = sx_comp;    
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
    MakePatternPtr< If > s_if;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp;  
    MakePatternPtr< Goto > s_then_goto, s_else_goto, r_goto;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< ConditionalOperator > mult;
    
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
    MakePatternPtr< If > s_if;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp, sx_comp;  
    MakePatternPtr< Goto > r_goto;
    MakePatternPtr< Star<Declaration> > decls, sx_decls;
    MakePatternPtr< Star<Statement> > pre, post, sx_pre, sx_post;
    MakePatternPtr< ConditionalOperator > mult;
    MakePatternPtr< Label > label;    
    MakePatternPtr< LabelIdentifier > label_id;
    MakePatternPtr< Conjunction<Compound> > s_all;
    MakePatternPtr< Negation<Compound> > s_not;
        
    s_all->patterns = (s_comp, s_not);
    s_not->pattern = sx_comp;
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
    MakePatternPtr<Assign> ass;
    MakePatternPtr< TransformOf<InstanceIdentifier> > decl( &GetDeclaration::instance );
    decl->pattern = MakePatternPtr<LocalVariable>();
    ass->operands = (decl, MakePatternPtr<Literal>());    
    return ass;
}


EnsureBootstrap::EnsureBootstrap()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< Conjunction<Compound> > s_all;
    MakePatternPtr< Negation<Compound> > s_not;    
    MakePatternPtr<Compound> s_body, r_body, sx_body;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, sx_pre, post;
    MakePatternPtr<Goto> r_goto;
    MakePatternPtr<Label> r_label;    
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("BOOTSTRAP");
    MakePatternPtr< Negation<Statement> > stop;
    MakePatternPtr<Goto> sx_goto;
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (s_not, s_body);
    s_not->pattern = sx_body;
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
    stop->pattern = MakeResetAssignmentPattern();
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;
    r_body->members = decls;
    r_body->statements = (pre, r_goto, r_label, stop, post);    

    Configure( SEARCH_REPLACE, fn );
}


AddStateLabelVar::AddStateLabelVar()
{
    MakePatternPtr<Compound> s_comp, r_comp, lr_compound;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Goto> ls_goto, lr_goto, sx_goto;
    MakePatternPtr<Assign> lr_assign;
    MakePatternPtr<Automatic> state_var;
    MakePatternPtr< Negation<Expression> > sx_not, lsx_not;
    MakePatternPtr< BuildInstanceIdentifierAgent > state_var_id("state");
    
    ls_goto->destination = lsx_not;
    lsx_not->pattern = state_var_id; //  MakePatternPtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lsx_not);
    lr_goto->destination = state_var_id;
            
    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave( r_comp, ls_goto, lr_compound );
     
    s_comp->members = (decls);
    s_comp->statements = (pre, sx_goto, post); 
    sx_goto->destination = sx_not;
    sx_not->pattern = MakePatternPtr<InstanceIdentifier>();
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, sx_goto, post); 
    state_var->identifier = state_var_id;
    state_var->type = MakePatternPtr<Labeley>();    
    state_var->initialiser = MakePatternPtr<Uninitialised>();

    Configure( SEARCH_REPLACE, s_comp, r_slave );
}


EnsureSuperLoop::EnsureSuperLoop()
{   
    MakePatternPtr<Instance> fn;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< Conjunction<Compound> > s_all;
    MakePatternPtr< Negation<Statement> > sx_not, s_limit;    
    MakePatternPtr< Stuff<Compound> > sx_stuff;
    MakePatternPtr< Goto > sx_goto, first_goto;
    MakePatternPtr<Compound> s_body, r_body, r_loop_body;
    MakePatternPtr< Star<Statement> > pre, post;    
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr<Do> r_loop;
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (sx_stuff, s_body);
    sx_stuff->terminus = sx_goto;
    sx_stuff->recurse_restriction = sx_not;
    sx_not->pattern = MakePatternPtr<Do>();
    s_body->members = (decls);
    s_body->statements = (pre, first_goto, post);
    pre->restriction = s_limit;
    s_limit->pattern = MakePatternPtr<Goto>();
    
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
    MakePatternPtr<Do> loop;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr<Compound> s_body, r_body;
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr< Star<Statement> > pre, post;    
    MakePatternPtr< Goto > first_goto, r_goto;
    MakePatternPtr<Label> r_label;    
    MakePatternPtr<BuildLabelIdentifierAgent> r_labelid("ITERATE");
                    
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
    MakePatternPtr<Compound> r_comp, s_body, r_body;
    MakePatternPtr<Switch> s_switch, r_switch;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > main, tail;
    MakePatternPtr<Label> label;
    MakePatternPtr<Expression> cond;
    MakePatternPtr< Negation<Statement> > sx_not_tail, sx_not_main;
    MakePatternPtr< Disjunction<Statement> > sx_any_tail;

    s_switch->condition = cond;
    s_switch->body = s_body;
    s_body->members = decls;
    s_body->statements = (main, label, tail);
    main->restriction = sx_not_main;
    sx_not_main->pattern = MakePatternPtr<Break>();
    tail->restriction = sx_not_tail;
    sx_not_tail->pattern = sx_any_tail;
    sx_any_tail->patterns = (MakePatternPtr<Break>(), MakePatternPtr<Case>());
    
    r_comp->statements = (r_switch, label, tail);
    r_switch->condition = cond;
    r_switch->body = r_body;
    r_body->members = decls;
    r_body->statements = (main);    
    
    Configure( SEARCH_REPLACE, s_switch, r_comp );        
}


InferBreak::InferBreak()
{
    MakePatternPtr<Goto> ls_goto;
    MakePatternPtr<Label> label;
    MakePatternPtr<LabelIdentifier> label_id;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Breakable> breakable;
    MakePatternPtr<Break> lr_break;
    MakePatternPtr<Compound> s_comp, r_comp;

    ls_goto->destination = label_id;

    MakePatternPtr< SlaveSearchReplace<Breakable> > slave( breakable, ls_goto, lr_break ); // todo restrict to not go through more breakables

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
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, cb1, cb2, post;
    MakePatternPtr<Case> case1, case2;
    MakePatternPtr<Break> breakk;
    MakePatternPtr< Negation<Statement> > s_not1, s_not2;
    
    s_comp->members = (decls);
    s_comp->statements = (pre, case1, cb1,              case2, cb2, breakk, post);
    r_comp->members = (decls);
    r_comp->statements = (pre, case1, cb1, cb2, breakk, case2, cb2, breakk, post);
    cb1->restriction = s_not1;
    s_not1->pattern = MakePatternPtr<Break>();
    cb2->restriction = s_not2;
    s_not2->pattern = MakePatternPtr<Case>();
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );            
}


AddYieldFlag::AddYieldFlag()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Callable> sub;
    MakePatternPtr<Compound> s_func_comp, r_func_comp, s_comp, r_comp, ms_comp, mr_comp, msx_comp;
    MakePatternPtr< Star<Declaration> > enums, decls, func_decls, m_decls, msx_decls;
    MakePatternPtr<Instance> var_decl;
    MakePatternPtr<InstanceIdentifier> var_id;    
    MakePatternPtr<TypeIdentifier> enum_id;
    MakePatternPtr< Star<Statement> > func_pre, m_pre, msx_pre, m_post, msx_post, stmts;
    MakePatternPtr< Star<If> > l_pre, l_post;
    MakePatternPtr<Loop> loop;
    MakePatternPtr<If> ls_if, lr_if, ms_if, mr_if;
    MakePatternPtr<Wait> m_wait;
    MakePatternPtr<Enum> enum_decl;
    MakePatternPtr<Equal> l_equal;
    MakePatternPtr<LogicalAnd> lr_and;
    MakePatternPtr<LogicalNot> lr_not;
    MakePatternPtr< Overlay<Compound> > func_over, over;
    MakePatternPtr<Temporary> r_flag_decl;
    MakePatternPtr<Assign> r_flag_init, mr_assign, msx_assign;
    MakePatternPtr<BuildInstanceIdentifierAgent> r_flag_id("yield_flag");
    MakePatternPtr< Conjunction<Compound> > ms_all;
    MakePatternPtr< Negation<Compound> > ms_not;
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slavem( r_func_comp, ms_all, mr_comp );
    MakePatternPtr< SlaveSearchReplace<Compound> > slave( r_comp, ls_if, lr_if );  
      
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

    ms_all->patterns = (ms_comp, ms_not);
    ms_comp->members = (m_decls);
    ms_comp->statements = (m_pre, m_wait, m_post);
    ms_not->pattern = msx_comp;
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
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    MakePatternPtr<Compound> func_comp, s_comp, sx_comp, r_comp;
    MakePatternPtr< Star<Declaration> > func_decls;
    MakePatternPtr< Star<Statement> > func_pre, stmts, sx_pre;    
    MakePatternPtr< Overlay<Statement> > over;    
    MakePatternPtr<LocalVariable> flag_decl; 
    MakePatternPtr<InstanceIdentifier> flag_id;   
    MakePatternPtr<WaitDelta> r_yield;
    MakePatternPtr<Loop> loop;
    MakePatternPtr<If> r_if, sx_if;
    MakePatternPtr< Conjunction<Compound> > s_all;
    MakePatternPtr< Negation<Compound> > s_notmatch;
    MakePatternPtr< LogicalNot > r_not, sx_not;
    MakePatternPtr< Assign > assign;
          
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
    s_all->patterns = (s_comp, s_notmatch);
    s_comp->members = (flag_decl);
    s_comp->statements = (stmts);
    s_notmatch->pattern = sx_comp;
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
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce SC stuff
    MakePatternPtr< Star<Declaration> > func_decls;
    MakePatternPtr< Star<Statement> > inits, stmts;    
    MakePatternPtr<Loop> loop;
    MakePatternPtr<Compound> s_func_comp, r_func_comp, s_comp, r_comp, r_if_comp;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Equal> r_equal;
    MakePatternPtr< Overlay<Compound> > func_over, over;    
    MakePatternPtr<Statement> first_init;
                    
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
    MakePatternPtr<Instance> fn, s_var_decl;
    MakePatternPtr<InstanceIdentifier> fn_id, s_var_id, s_cur_enum_id, s_outer_enum_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce SC stuff
    MakePatternPtr< Star<Declaration> > func_decls, s_enums;
    MakePatternPtr< Star<Declaration> > comp_loop_decls, comp_yield_decls, x_comp_decls;
    MakePatternPtr<Static> s_cur_enum, s_outer_enum;
    MakePatternPtr< Star<Statement> > inits, stmts, prepre, prepost, postpre, postpost;    
    MakePatternPtr< Star<Statement> > comp_loop_pre, comp_loop_post, comp_yield_pre, comp_yield_post;    
    MakePatternPtr<Loop> loop;
    MakePatternPtr<Compound> func_comp, s_comp_loop, s_comp_yield, r_comp, r_if_comp, sx_comp;
    MakePatternPtr<If> loop_top, loop_bottom, yield, outer_bottom, outer_top;
    MakePatternPtr< Star<If> > loop_body, pre_yield, post_yield;
    MakePatternPtr<Equal> r_equal;
    MakePatternPtr< Overlay<Compound> > func_over, over;    
    MakePatternPtr< Conjunction<Compound> > s_all;
    MakePatternPtr<Enum> s_enum;
    MakePatternPtr<TypeIdentifier> s_enum_id;
    MakePatternPtr< Stuff<Expression> > loop_top_stuff, outer_top_stuff;
    MakePatternPtr<Equal> loop_top_equal, outer_top_equal;
    MakePatternPtr< Stuff<Statement> > loop_bottom_stuff_enum, outer_bottom_stuff_enum, 
                                    loop_bottom_stuff_noyield, yield_stuff, outer_bottom_stuff_noyield;
    MakePatternPtr< Conjunction<Statement> > loop_bottom_matchall, outer_bottom_matchall;
    MakePatternPtr< Negation<Statement> > loop_bottom_notmatch, outer_bottom_notmatch;
    MakePatternPtr< Negation<Compound> > s_notmatch;
    MakePatternPtr< Disjunction<If> > inner_state;
    
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
    s_all->patterns = (s_comp_loop, s_comp_yield, s_notmatch);
    s_comp_loop->members = (comp_loop_decls);
    // Search for a loop. Assume that a state enum value in a body means "could transition to the state" and one in
    // the condition means "acts on the state". If we see the latter with the former blow it somewhere, we call
    // it a loop and assume the upward branch is normally takner as with C compilers.    
    s_comp_loop->statements = (comp_loop_pre, loop_top, loop_body, loop_bottom, comp_loop_post);    
    loop_top->condition = loop_top_stuff;
    loop_top_stuff->terminus = loop_top_equal;
    loop_top_equal->operands = (s_var_id, s_cur_enum_id);
    loop_bottom->body = loop_bottom_matchall;
    loop_bottom_matchall->patterns = (loop_bottom_stuff_enum, loop_bottom_notmatch);
    loop_bottom_stuff_enum->terminus = s_cur_enum_id;
    loop_bottom_notmatch->pattern = loop_bottom_stuff_noyield;
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
    s_notmatch->pattern = sx_comp;
    sx_comp->members = (x_comp_decls);
    sx_comp->statements = (prepre, outer_top, postpre, inner_state, prepost, outer_bottom, postpost);
    outer_top->condition = outer_top_stuff;
    outer_top_stuff->terminus = outer_top_equal;
    outer_top_equal->operands = (s_var_id, s_outer_enum_id);
    outer_bottom->body = outer_bottom_matchall;
    outer_bottom_matchall->patterns = (outer_bottom_stuff_enum, outer_bottom_notmatch);
    outer_bottom_stuff_enum->terminus = s_outer_enum_id;
    outer_bottom_notmatch->pattern = outer_bottom_stuff_noyield;
    outer_bottom_stuff_noyield->terminus = MakePatternPtr<Wait>();    
    inner_state->patterns = (loop_top, loop_bottom); // outer loop can share top or bottom state with inner loop; but not both, so at least one must be here
   
    over->overlay = r_comp;
    r_comp->members = (comp_loop_decls);
    r_comp->statements = (comp_loop_pre, loop_bottom, loop_top, loop_body, comp_loop_post);    // rotated version of s_comp_loop
        
    Configure( SEARCH_REPLACE, fn );            
}


