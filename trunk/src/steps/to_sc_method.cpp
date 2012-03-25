#include "steps/to_sc_method.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_patterns.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;
 
AutosToModule::AutosToModule()
{
    MakeTreePtr<Scope> rec;
    MakeTreePtr< Star<Declaration> > decls, vdecls;
    MakeTreePtr< Star<Statement> > vstmts;
    MakeTreePtr<Automatic> s_var;
    MakeTreePtr<Field> fn, r_var;
    MakeTreePtr<Callable> ft;
    MakeTreePtr< Stuff<Initialiser> > stuff;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr<Type> type;
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr<Initialiser> init;
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< NotMatch<Compound> > sx_not;
    MakeTreePtr< Stuff<Compound> > sx_stuff;
    MakeTreePtr<Call> sx_call;
    MakeTreePtr< Insert<Declaration> > insert;
        
    rec->members = (decls, fn, insert);
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
     
    insert->insert = (r_var);
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    r_var->type = type;
    r_var->identifier = var_id;
    r_var->initialiser = init;
    r_var->virt = MakeTreePtr<NonVirtual>();
    r_var->access = MakeTreePtr<Private>();
    r_var->constancy = MakeTreePtr<NonConst>();
    
    Configure( rec );
}


TempsAndStaticsToModule::TempsAndStaticsToModule()
{
    MakeTreePtr<Scope> rec;
    MakeTreePtr< Star<Declaration> > decls, vdecls;
    MakeTreePtr< Star<Statement> > vstmts;
    MakeTreePtr< MatchAny<Instance> > var;
    MakeTreePtr<Temporary> tempvar;
    MakeTreePtr<Static> staticvar;
    MakeTreePtr<Field> fn;
    MakeTreePtr<Thread> ft;
    MakeTreePtr< Stuff<Initialiser> > stuff;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr<Type> type;
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr<Initialiser> init;
    MakeTreePtr< Insert<Declaration> > insert;

    rec->members = (decls, fn, insert);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, var);
    s_comp->statements = (vstmts);
    var->patterns = (tempvar, staticvar);
     
    insert->insert = (var);
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( rec );
}


DeclsToModule::DeclsToModule()
{
    MakeTreePtr<Scope> rec;
    MakeTreePtr< Star<Declaration> > decls, vdecls;
    MakeTreePtr< Star<Statement> > vstmts;
    MakeTreePtr<Field> fn;
    MakeTreePtr<UserType> ut;
    MakeTreePtr<Thread> ft;
    MakeTreePtr< Stuff<Initialiser> > stuff;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr< Insert<Declaration> > insert;
    
    rec->members = (decls, fn, insert);
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, ut);
    s_comp->statements = (vstmts);
     
    insert->insert = (ut);
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( rec );
}


ThreadToMethod::ThreadToMethod()
{
    MakeTreePtr<Instance> s_thread, r_method;
    MakeTreePtr<Thread> s_thread_type;
    MakeTreePtr<Method> r_method_type;
    MakeTreePtr<Compound> s_comp, loop_comp;
    MakeTreePtr<InstanceIdentifier> id;
    MakeTreePtr<Do> s_loop;
    MakeTreePtr<True> s_loop_cond;
    MakeTreePtr< Star<Declaration> > loop_decls;
    MakeTreePtr< Star<Statement> > loop_stmts;
    MakeTreePtr<WaitDynamic> ls_wait_dynamic;
    MakeTreePtr<NextTriggerDynamic> lr_nt_dynamic;
    MakeTreePtr<WaitStatic> ms_wait_static;
    MakeTreePtr<NextTriggerStatic> mr_nt_static;
    MakeTreePtr<WaitDelta> ns_wait_delta;
    MakeTreePtr<NextTriggerDelta> nr_nt_delta;
    MakeTreePtr<Expression> l_event;
    MakeTreePtr< SlaveSearchReplace<Compound> > slaven( loop_comp, ns_wait_delta, nr_nt_delta);
    MakeTreePtr< SlaveSearchReplace<Compound> > slavem( slaven, ms_wait_static, mr_nt_static);
    MakeTreePtr< SlaveSearchReplace<Compound> > slavel( slavem, ls_wait_dynamic, lr_nt_dynamic);

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


