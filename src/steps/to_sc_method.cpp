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
    MakePatternPtr<Scope> s_rec, r_rec;
    MakePatternPtr< Star<Declaration> > decls, vdecls;
    MakePatternPtr< Star<Statement> > vstmts;
    MakePatternPtr<Automatic> s_var;
    MakePatternPtr<Field> fn, r_var;
    MakePatternPtr<Callable> ft;
    MakePatternPtr< Stuff<Initialiser> > stuff;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Type> type;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<Initialiser> init;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Compound> > sx_not;
    MakePatternPtr< Stuff<Compound> > sx_stuff;
    MakePatternPtr<Call> sx_call;
        
    s_rec->members = (decls, fn);
    r_rec->members = (decls, fn, r_var);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_all;
    s_all->patterns = (sx_not, s_comp);
    sx_not->pattern = sx_stuff;
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
    
    Configure( s_rec, r_rec );
}


TempsAndStaticsToModule::TempsAndStaticsToModule()
{
    MakePatternPtr<Scope> s_rec, r_rec;
    MakePatternPtr< Star<Declaration> > decls, vdecls;
    MakePatternPtr< Star<Statement> > vstmts;
    MakePatternPtr< MatchAny<Instance> > var;
    MakePatternPtr<Temporary> tempvar;
    MakePatternPtr<Static> staticvar;
    MakePatternPtr<Field> fn;
    MakePatternPtr<Thread> ft;
    MakePatternPtr< Stuff<Initialiser> > stuff;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Type> type;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<Initialiser> init;

    s_rec->members = (decls, fn);
    r_rec->members = (decls, fn, var);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, var);
    s_comp->statements = (vstmts);
    var->patterns = (tempvar, staticvar);
     
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( s_rec, r_rec );
}


DeclsToModule::DeclsToModule()
{
    MakePatternPtr<Scope> s_rec, r_rec;
    MakePatternPtr< Star<Declaration> > decls, vdecls;
    MakePatternPtr< Star<Statement> > vstmts;
    MakePatternPtr<Field> fn;
    MakePatternPtr<UserType> ut;
    MakePatternPtr<Thread> ft;
    MakePatternPtr< Stuff<Initialiser> > stuff;
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< Star<Base> > bases;
    
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
    
    Configure( s_rec, r_rec );
}


ThreadToMethod::ThreadToMethod()
{
    MakePatternPtr<Instance> s_thread, r_method;
    MakePatternPtr<Thread> s_thread_type;
    MakePatternPtr<Method> r_method_type;
    MakePatternPtr<Compound> s_comp, loop_comp;
    MakePatternPtr<InstanceIdentifier> id;
    MakePatternPtr<Do> s_loop;
    MakePatternPtr<True> s_loop_cond;
    MakePatternPtr< Star<Declaration> > loop_decls;
    MakePatternPtr< Star<Statement> > loop_stmts;
    MakePatternPtr<WaitDynamic> ls_wait_dynamic;
    MakePatternPtr<NextTriggerDynamic> lr_nt_dynamic;
    MakePatternPtr<WaitStatic> ms_wait_static;
    MakePatternPtr<NextTriggerStatic> mr_nt_static;
    MakePatternPtr<WaitDelta> ns_wait_delta;
    MakePatternPtr<NextTriggerDelta> nr_nt_delta;
    MakePatternPtr<Continue> os_continue;
    MakePatternPtr<Return> or_return;
    MakePatternPtr<Uninitialised> or_retval;
    MakePatternPtr<Expression> l_event;
    
    or_return->return_value = or_retval;
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slaveo( loop_comp, os_continue, or_return);
    MakePatternPtr< SlaveSearchReplace<Compound> > slaven( slaveo, ns_wait_delta, nr_nt_delta);
    MakePatternPtr< SlaveSearchReplace<Compound> > slavem( slaven, ms_wait_static, mr_nt_static);
    MakePatternPtr< SlaveSearchReplace<Compound> > slavel( slavem, ls_wait_dynamic, lr_nt_dynamic);

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
    
    Configure( s_thread, r_method );
}


ExplicitiseReturns::ExplicitiseReturns()
{
    MakePatternPtr<Instance> inst;
    MakePatternPtr<Callable> s_callable;
    MakePatternPtr<Compound> s_comp, r_comp, m_comp;
    MakePatternPtr< Overlay<Compound> > over_comp;
    MakePatternPtr< MatchAll<Instance> > s_all;
    MakePatternPtr< Stuff<Instance> > s_stuff;
    MakePatternPtr<Return> s_return, ls_return, m_return;
    MakePatternPtr< Star<Declaration> > decls, m_decls;
    MakePatternPtr< Star<Statement> > stmts, m_pre, m_mid, m_post;
    MakePatternPtr<Temporary> r_flag;
    MakePatternPtr<Boolean> r_boolean;
    MakePatternPtr<BuildInstanceIdentifier> r_flag_id("enabled");
    MakePatternPtr<False> lr_false;
    MakePatternPtr<True> r_true;
    MakePatternPtr<Uninitialised> s_uninit, ls_uninit, m_uninit;
    MakePatternPtr<Assign> lr_assign;
    MakePatternPtr< Stuff<Statement> > m_stuff;
    MakePatternPtr< NotMatch<Statement> > ms_affected;
    MakePatternPtr<If> ms_if, mr_if;
    MakePatternPtr< Overlay<Statement> > m_over;
    
    m_comp->members = (m_decls);
    m_comp->statements = (m_pre, m_stuff, m_mid, m_over, m_post);
    m_stuff->terminus = m_return;
    m_return->return_value = m_uninit;
    m_over->through = ms_affected;
    ms_affected->pattern = ms_if;
    ms_if->condition = r_flag_id;
    m_over->overlay = mr_if;
    mr_if->condition = r_flag_id;
    mr_if->body = ms_affected;
    mr_if->else_body = MakePatternPtr<Nop>();
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slavem( over_comp, m_comp );
    
    ls_return->return_value = ls_uninit;
    lr_assign->operands = (r_flag_id, lr_false);
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slavel( slavem, ls_return, lr_assign);
    
    s_all->patterns = (inst, s_stuff);
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
        
    Configure( s_all, inst );
}
