#include "steps/to_sc_method.hpp"
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
 
AutosToModule::AutosToModule()
{
    auto s_rec = MakePatternPtr<Scope>();
    auto r_rec = MakePatternPtr<Scope>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto vdecls = MakePatternPtr< Star<Declaration> >();
    auto vstmts = MakePatternPtr< Star<Statement> >();
    auto s_var = MakePatternPtr<Automatic>();
    auto fn = MakePatternPtr<Field>();
    auto r_var = MakePatternPtr<Field>();
    auto ft = MakePatternPtr<Callable>();
    auto stuff = MakePatternPtr< Stuff<Initialiser> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto type = MakePatternPtr<Type>();
    auto var_id = MakePatternPtr<InstanceIdentifier>();
    auto init = MakePatternPtr<Initialiser>();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto sx_not = MakePatternPtr< Negation<Compound> >();
    auto sx_stuff = MakePatternPtr< Stuff<Compound> >();
    auto sx_call = MakePatternPtr<Call>();
        
    s_rec->members = (decls, fn);
    r_rec->members = (decls, fn, r_var);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_all;
    s_all->conjuncts = (sx_not, s_comp);
    sx_not->negand = sx_stuff;
    sx_stuff->terminus = sx_call;
    s_comp->members = (vdecls, s_var);
    s_comp->statements = (vstmts);
    s_var->type = type;
    s_var->identifier = var_id;
    s_var->initialiser = init;
     
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    r_var->type = type;
    r_var->identifier = var_id;
    r_var->initialiser = init;
    r_var->virt = MakePatternPtr<NonVirtual>();
    r_var->access = MakePatternPtr<Private>();
    r_var->constancy = MakePatternPtr<NonConst>();
    
    Configure( SEARCH_REPLACE, s_rec, r_rec );
}


TempsAndStaticsToModule::TempsAndStaticsToModule()
{
    auto s_rec = MakePatternPtr<Scope>();
    auto r_rec = MakePatternPtr<Scope>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto vdecls = MakePatternPtr< Star<Declaration> >();
    auto vstmts = MakePatternPtr< Star<Statement> >();
    auto var = MakePatternPtr< Disjunction<Instance> >();
    auto tempvar = MakePatternPtr<Temporary>();
    auto staticvar = MakePatternPtr<Static>();
    auto fn = MakePatternPtr<Field>();
    auto ft = MakePatternPtr<Thread>();
    auto stuff = MakePatternPtr< Stuff<Initialiser> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto type = MakePatternPtr<Type>();
    auto var_id = MakePatternPtr<InstanceIdentifier>();
    auto init = MakePatternPtr<Initialiser>();

    s_rec->members = (decls, fn);
    r_rec->members = (decls, fn, var);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, var);
    s_comp->statements = (vstmts);
    var->disjuncts = (tempvar, staticvar);
     
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( SEARCH_REPLACE, s_rec, r_rec );
}


DeclsToModule::DeclsToModule()
{
    auto s_rec = MakePatternPtr<Scope>();
    auto r_rec = MakePatternPtr<Scope>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto vdecls = MakePatternPtr< Star<Declaration> >();
    auto vstmts = MakePatternPtr< Star<Statement> >();
    auto fn = MakePatternPtr<Field>();
    auto ut = MakePatternPtr<UserType>();
    auto ft = MakePatternPtr<Thread>();
    auto stuff = MakePatternPtr< Stuff<Initialiser> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto bases = MakePatternPtr< Star<Base> >();
    
    s_rec->members = (decls, fn);
    r_rec->members = (decls, fn, ut);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, ut);
    s_comp->statements = (vstmts);
     
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( SEARCH_REPLACE, s_rec, r_rec );
}


ThreadToMethod::ThreadToMethod()
{
    auto s_thread = MakePatternPtr<Instance>();
    auto r_method = MakePatternPtr<Instance>();
    auto s_thread_type = MakePatternPtr<Thread>();
    auto r_method_type = MakePatternPtr<Method>();
    auto s_comp = MakePatternPtr<Compound>();
    auto loop_comp = MakePatternPtr<Compound>();
    auto id = MakePatternPtr<InstanceIdentifier>();
    auto s_loop = MakePatternPtr<Do>();
    auto s_loop_cond = MakePatternPtr<True>();
    auto loop_decls = MakePatternPtr< Star<Declaration> >();
    auto loop_stmts = MakePatternPtr< Star<Statement> >();
    auto ls_wait_dynamic = MakePatternPtr<WaitDynamic>();
    auto lr_nt_dynamic = MakePatternPtr<NextTriggerDynamic>();
    auto ms_wait_static = MakePatternPtr<WaitStatic>();
    auto mr_nt_static = MakePatternPtr<NextTriggerStatic>();
    auto ns_wait_delta = MakePatternPtr<WaitDelta>();
    auto nr_nt_delta = MakePatternPtr<NextTriggerDelta>();
    auto os_continue = MakePatternPtr<Continue>();
    auto or_return = MakePatternPtr<Return>();
    auto or_retval = MakePatternPtr<Uninitialised>();
    auto l_event = MakePatternPtr<Expression>();
    
    or_return->return_value = or_retval;
    
    auto slaveo = MakePatternPtr< SlaveSearchReplace<Compound> >( loop_comp, os_continue, or_return);
    auto slaven = MakePatternPtr< SlaveSearchReplace<Compound> >( slaveo, ns_wait_delta, nr_nt_delta);
    auto slavem = MakePatternPtr< SlaveSearchReplace<Compound> >( slaven, ms_wait_static, mr_nt_static);
    auto slavel = MakePatternPtr< SlaveSearchReplace<Compound> >( slavem, ls_wait_dynamic, lr_nt_dynamic);

    s_thread->type = s_thread_type;
    s_thread->initialiser = s_comp;
    s_thread->identifier = id;
//    s_comp->members = (); important - locals in the body scope not allowed, they would lose their values every itheration in SC_METHOD
    s_comp->statements = s_loop;
    s_loop->condition = s_loop_cond; 
    s_loop->body = loop_comp;
    loop_comp->members = (loop_decls);
    loop_comp->statements = (loop_stmts);
    r_method->type = r_method_type;
    r_method->initialiser = slavel;
    r_method->identifier = id;
    
    ls_wait_dynamic->event = l_event;
    lr_nt_dynamic->event = l_event;
    
    Configure( SEARCH_REPLACE, s_thread, r_method );
}


ExplicitiseReturns::ExplicitiseReturns()
{
    auto inst = MakePatternPtr<Instance>();
    auto s_callable = MakePatternPtr<Callable>();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto m_comp = MakePatternPtr<Compound>();
    auto over_comp = MakePatternPtr< Delta<Compound> >();
    auto s_all = MakePatternPtr< Conjunction<Instance> >();
    auto s_stuff = MakePatternPtr< Stuff<Instance> >();
    auto s_return = MakePatternPtr<Return>();
    auto ls_return = MakePatternPtr<Return>();
    auto m_return = MakePatternPtr<Return>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto m_decls = MakePatternPtr< Star<Declaration> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto m_pre = MakePatternPtr< Star<Statement> >();
    auto m_mid = MakePatternPtr< Star<Statement> >();
    auto m_post = MakePatternPtr< Star<Statement> >();
    auto r_flag = MakePatternPtr<Temporary>();
    auto r_boolean = MakePatternPtr<Boolean>();
    auto r_flag_id = MakePatternPtr<BuildInstanceIdentifierAgent>("enabled");
    auto lr_false = MakePatternPtr<False>();
    auto r_true = MakePatternPtr<True>();
    auto s_uninit = MakePatternPtr<Uninitialised>();
    auto ls_uninit = MakePatternPtr<Uninitialised>();
    auto m_uninit = MakePatternPtr<Uninitialised>();
    auto lr_assign = MakePatternPtr<Assign>();
    auto m_stuff = MakePatternPtr< Stuff<Statement> >();
    auto ms_affected = MakePatternPtr< Negation<Statement> >();
    auto ms_if = MakePatternPtr<If>();
    auto mr_if = MakePatternPtr<If>();
    auto m_over = MakePatternPtr< Delta<Statement> >();
    
    m_comp->members = (m_decls);
    m_comp->statements = (m_pre, m_stuff, m_mid, m_over, m_post);
    m_stuff->terminus = m_return;
    m_return->return_value = m_uninit;
    m_over->through = ms_affected;
    ms_affected->negand = ms_if;
    ms_if->condition = r_flag_id;
    m_over->overlay = mr_if;
    mr_if->condition = r_flag_id;
    mr_if->body = ms_affected;
    mr_if->else_body = MakePatternPtr<Nop>();
    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Compound> >( over_comp, m_comp );
    
    ls_return->return_value = ls_uninit;
    lr_assign->operands = (r_flag_id, lr_false);
    
    auto slavel = MakePatternPtr< SlaveSearchReplace<Compound> >( slavem, ls_return, lr_assign);
    
    s_all->conjuncts = (inst, s_stuff);
    inst->type = s_callable; // TODO when functions are sorted out, set return type to void
    inst->initialiser = slavel;
    s_stuff->terminus = s_return;
    s_return->return_value = s_uninit;
    over_comp->through = s_comp;
    over_comp->overlay = r_comp;
    s_comp->members = (decls);
    r_comp->members = (decls, r_flag);
    s_comp->statements = r_comp->statements = (stmts);
    r_flag->type = r_boolean;
    r_flag->identifier = r_flag_id;
    r_flag->initialiser = r_true;
        
    Configure( SEARCH_REPLACE, s_all, inst );
}
