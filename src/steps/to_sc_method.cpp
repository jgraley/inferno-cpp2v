#include "steps/to_sc_method.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/hastype.hpp"
#include "tree/misc.hpp"
#include "inferno_agents.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;
 
AutosToModule::AutosToModule()
{
    auto s_rec = MakePatternNode<Scope>();
    auto r_rec = MakePatternNode<Scope>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto vdecls = MakePatternNode< Star<Declaration> >();
    auto vstmts = MakePatternNode< Star<Statement> >();
    auto s_var = MakePatternNode<Automatic>();
    auto fn = MakePatternNode<Field>();
    auto r_var = MakePatternNode<Field>();
    auto ft = MakePatternNode<Callable>();
    auto stuff = MakePatternNode< Stuff<Initialiser> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto type = MakePatternNode<Type>();
    auto var_id = MakePatternNode<InstanceIdentifier>();
    auto init = MakePatternNode<Initialiser>();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto sx_not = MakePatternNode< Negation<Compound> >();
    auto sx_stuff = MakePatternNode< Stuff<Compound> >();
    auto sx_call = MakePatternNode<Call>();
        
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
    r_var->virt = MakePatternNode<NonVirtual>();
    r_var->access = MakePatternNode<Private>();
    r_var->constancy = MakePatternNode<NonConst>();
    
    Configure( SEARCH_REPLACE, s_rec, r_rec );
}


TempsAndStaticsToModule::TempsAndStaticsToModule()
{
    auto s_rec = MakePatternNode<Scope>();
    auto r_rec = MakePatternNode<Scope>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto vdecls = MakePatternNode< Star<Declaration> >();
    auto vstmts = MakePatternNode< Star<Statement> >();
    auto var = MakePatternNode< Disjunction<Instance> >();
    auto tempvar = MakePatternNode<Temporary>();
    auto staticvar = MakePatternNode<Static>();
    auto fn = MakePatternNode<Field>();
    auto ft = MakePatternNode<Thread>();
    auto stuff = MakePatternNode< Stuff<Initialiser> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto type = MakePatternNode<Type>();
    auto var_id = MakePatternNode<InstanceIdentifier>();
    auto init = MakePatternNode<Initialiser>();

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
    auto s_rec = MakePatternNode<Scope>();
    auto r_rec = MakePatternNode<Scope>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto vdecls = MakePatternNode< Star<Declaration> >();
    auto vstmts = MakePatternNode< Star<Statement> >();
    auto fn = MakePatternNode<Field>();
    auto ut = MakePatternNode<UserType>();
    auto ft = MakePatternNode<Thread>();
    auto stuff = MakePatternNode< Stuff<Initialiser> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto over = MakePatternNode< Delta<Compound> >();
    auto bases = MakePatternNode< Star<Base> >();
    
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
    auto s_thread = MakePatternNode<Instance>();
    auto r_method = MakePatternNode<Instance>();
    auto s_thread_type = MakePatternNode<Thread>();
    auto r_method_type = MakePatternNode<Method>();
    auto s_comp = MakePatternNode<Compound>();
    auto loop_comp = MakePatternNode<Compound>();
    auto id = MakePatternNode<InstanceIdentifier>();
    auto s_loop = MakePatternNode<Do>();
    auto s_loop_cond = MakePatternNode<True>();
    auto loop_decls = MakePatternNode< Star<Declaration> >();
    auto loop_stmts = MakePatternNode< Star<Statement> >();
    auto ls_wait_dynamic = MakePatternNode<WaitDynamic>();
    auto lr_nt_dynamic = MakePatternNode<NextTriggerDynamic>();
    auto ms_wait_static = MakePatternNode<WaitStatic>();
    auto mr_nt_static = MakePatternNode<NextTriggerStatic>();
    auto ns_wait_delta = MakePatternNode<WaitDelta>();
    auto nr_nt_delta = MakePatternNode<NextTriggerDelta>();
    auto os_continue = MakePatternNode<Continue>();
    auto or_return = MakePatternNode<Return>();
    auto or_retval = MakePatternNode<Uninitialised>();
    auto l_event = MakePatternNode<Expression>();
    
    or_return->return_value = or_retval;
    
    auto slaveo = MakePatternNode< EmbeddedSearchReplace<Compound> >( loop_comp, os_continue, or_return);
    auto slaven = MakePatternNode< EmbeddedSearchReplace<Compound> >( slaveo, ns_wait_delta, nr_nt_delta);
    auto slavem = MakePatternNode< EmbeddedSearchReplace<Compound> >( slaven, ms_wait_static, mr_nt_static);
    auto slavel = MakePatternNode< EmbeddedSearchReplace<Compound> >( slavem, ls_wait_dynamic, lr_nt_dynamic);

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
    auto inst = MakePatternNode<Instance>();
    auto s_callable = MakePatternNode<Callable>();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto m_comp = MakePatternNode<Compound>();
    auto over_comp = MakePatternNode< Delta<Compound> >();
    auto s_all = MakePatternNode< Conjunction<Instance> >();
    auto s_stuff = MakePatternNode< Stuff<Instance> >();
    auto s_return = MakePatternNode<Return>();
    auto ls_return = MakePatternNode<Return>();
    auto m_return = MakePatternNode<Return>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto m_decls = MakePatternNode< Star<Declaration> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto m_pre = MakePatternNode< Star<Statement> >();
    auto m_mid = MakePatternNode< Star<Statement> >();
    auto m_post = MakePatternNode< Star<Statement> >();
    auto r_flag = MakePatternNode<Temporary>();
    auto r_boolean = MakePatternNode<Boolean>();
    auto r_flag_id = MakePatternNode<BuildInstanceIdentifierAgent>("enabled");
    auto lr_false = MakePatternNode<False>();
    auto r_true = MakePatternNode<True>();
    auto s_uninit = MakePatternNode<Uninitialised>();
    auto ls_uninit = MakePatternNode<Uninitialised>();
    auto m_uninit = MakePatternNode<Uninitialised>();
    auto lr_assign = MakePatternNode<Assign>();
    auto m_stuff = MakePatternNode< Stuff<Statement> >();
    auto ms_affected = MakePatternNode< Negation<Statement> >();
    auto ms_if = MakePatternNode<If>();
    auto mr_if = MakePatternNode<If>();
    auto m_over = MakePatternNode< Delta<Statement> >();
    
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
    mr_if->else_body = MakePatternNode<Nop>();
    
    auto slavem = MakePatternNode< EmbeddedSearchReplace<Compound> >( over_comp, m_comp );
    
    ls_return->return_value = ls_uninit;
    lr_assign->operands = (r_flag_id, lr_false);
    
    auto slavel = MakePatternNode< EmbeddedSearchReplace<Compound> >( slavem, ls_return, lr_assign);
    
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
