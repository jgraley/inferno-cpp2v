
#include "systemc_raising.hpp"
#include "clean_up.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "vn/agents/all.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


RaiseSCType::RaiseSCType( TreePtr< Type > lr_sctype )
{
    auto over = MakePatternNode< Delta<Node> >();
    auto s_scope = MakePatternNode< Scope >();
    auto r_scope = MakePatternNode< Scope >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto s_usertype = MakePatternNode< UserType >();
    auto s_token = MakePatternNode< TypeIdentifierByNameAgent >( lr_sctype->GetToken() );                
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Node> >( over, s_token, lr_sctype );    
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_usertype);
    s_usertype->identifier = s_token;
    
    r_scope->members = (decls);          
       
    Configure( COMPARE_REPLACE, over, r_embedded );
}


RaiseSCHierarchicalClass::RaiseSCHierarchicalClass( TreePtr< SCRecord > lr_scclass )
{
    auto over = MakePatternNode< Delta<Node> >();
    auto s_scope = MakePatternNode< Scope >();
    auto r_scope = MakePatternNode< Scope >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_bases = MakePatternNode< Star<Base> >();
    auto s_usertype = MakePatternNode< UserType >();
    auto ls_class = MakePatternNode< InheritanceRecord >();
    auto ls_base = MakePatternNode< Base >();
    auto l_tid = MakePatternNode< TypeIdentifier >();
    auto s_token = MakePatternNode< TypeIdentifierByNameAgent >( lr_scclass->GetToken() ); 
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Node> >( over, ls_class, lr_scclass );    
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_usertype);
    s_usertype->identifier = s_token;        
    r_scope->members = (decls);   
    
    ls_class->identifier = l_tid;       
    ls_class->members = (l_decls);
    ls_class->bases = (l_bases, ls_base);       
    ls_base->record = s_token;
    lr_scclass->identifier = l_tid;       
    lr_scclass->members = (l_decls);
    lr_scclass->bases = (l_bases);
       
    Configure( COMPARE_REPLACE, over, r_embedded );
}


RaiseSCDynamic::RaiseSCDynamic( TreePtr<SCDynamicFunction> r_dynamic )
{
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_dynamic->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "p1" ); 
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->key = s_param_id;
    s_arg->value = eexpr;
    eexpr->pattern = MakePatternNode<Event>();
    r_dynamic->event = eexpr;       
      
    Configure( SEARCH_REPLACE, s_call, r_dynamic );
}


RaiseSCStatic::RaiseSCStatic( TreePtr<SCFunction> r_static )
{
    auto s_call = MakePatternNode< Call >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_static->GetToken() ); 
                      
    s_call->callee = s_token;   
    //s_call->operands = ();       
       
    Configure( SEARCH_REPLACE, s_call, r_static );
}


RaiseSCDelta::RaiseSCDelta( TreePtr<SCFunction> r_delta )
{
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_delta->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "p1" ); 
    auto s_arg_id = MakePatternNode< InstanceIdentifierByNameAgent >( "SC_ZERO_TIME" ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->key = s_param_id;
    s_arg->value = s_arg_id;
      
    // TODO This is the last trans to remove calls to r_delta->GetToken(), so   
    // clear the declaration away.
      
    Configure( SEARCH_REPLACE, s_call, r_delta );
}


RaiseTerminationFunction::RaiseTerminationFunction( TreePtr<TerminationFunction> r_tf )
{
    auto event = MakePatternNode< Expression >();
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_tf->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "exit_code" ); 
            
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->key = s_param_id;
    s_arg->value = event;
    r_tf->code = event;       
      
    // TODO clear the declaration away.      
      
    Configure( SEARCH_REPLACE, s_call, r_tf );
}


RaiseSCProcess::RaiseSCProcess( TreePtr< Process > lr_scprocess )
{
    auto over = MakePatternNode< Delta<Node> >();
    auto s_scope = MakePatternNode< Scope >();
    auto r_scope = MakePatternNode< Scope >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_cdecls = MakePatternNode< Star<Declaration> >();
    auto s_instance = MakePatternNode< Static >();
    auto ls_comp = MakePatternNode< Compound >();
    auto lr_comp = MakePatternNode< Compound >();
    auto l_module = MakePatternNode< Module >();
    auto ls_pcall = MakePatternNode< Call >();
    auto ls_arg = MakePatternNode< MapOperand >();
    auto l_overcons = MakePatternNode< Delta<Instance> >();
    auto l_overtype = MakePatternNode< Delta<Type> >();
    auto ls_cons = MakePatternNode< Instance >();
    auto lr_cons = MakePatternNode< Instance >();
    auto l_process = MakePatternNode< Instance >();
    auto l_pre = MakePatternNode< Star<Statement> >();
    auto l_post = MakePatternNode< Star<Statement> >();
    auto ls_id = MakePatternNode< InstanceIdentifier >();
    auto l_bases = MakePatternNode< Star<Base> >();
    auto l_ctype = MakePatternNode<Constructor>();
    auto l_ident = MakePatternNode<InstanceIdentifier>();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( lr_scprocess->GetToken() ); 
    auto s_arg_id = MakePatternNode< InstanceIdentifierByNameAgent >( "func" );
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Node> >( over, l_module, l_module );            
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_instance);
    s_instance->identifier = s_token;        
    s_instance->type = MakePatternNode<Callable>(); // just narrow things a little        
    r_scope->members = (decls);   
            
    l_module->members = (l_overcons, l_process, l_decls);
    l_module->bases = (l_bases);
    l_overcons->through = ls_cons;       
    ls_cons->initialiser = ls_comp;
    ls_comp->members = l_cdecls;
    ls_comp->statements = (l_pre, ls_pcall, l_post);
    ls_cons->type = l_ctype;
    ls_cons->identifier = l_ident;
    l_ctype->params = (MakePatternNode<Parameter>()); // one parameter
    ls_pcall->callee = s_token;
    ls_pcall->operands = (ls_arg);
    ls_arg->key = s_arg_id;
    ls_arg->value = ls_id;
    l_overcons->overlay = lr_cons;
    lr_cons->initialiser = lr_comp;
    lr_comp->members = l_cdecls;
    lr_comp->statements = (l_pre, l_post);
    lr_cons->type = l_ctype;
    lr_cons->identifier = l_ident;
    
    l_process->identifier = ls_id;
    l_process->type = l_overtype;
    l_overtype->through = MakePatternNode<Callable>();
    l_overtype->overlay = lr_scprocess;
    
    Configure( COMPARE_REPLACE, over, r_embedded );
}


RaiseSCDeltaCount::RaiseSCDeltaCount()
{
    auto r_delta_count = MakePatternNode<DeltaCount>();

    auto s_call = MakePatternNode<SysCall>();
    auto s_token = MakePatternNode< SpecificInstanceIdentifier >( r_delta_count->GetToken() );                
                
    s_call->callee = s_token;
    //s_call->operands = (); // no operands
       
    Configure( SEARCH_REPLACE, s_call, r_delta_count );
}


RaiseSCNotifyImmediate::RaiseSCNotifyImmediate()
{
    auto s_call = MakePatternNode<Call>();
    auto s_lookup = MakePatternNode<Lookup>();
    auto s_event = MakePatternNode<Event>();
    auto r_notify = MakePatternNode<NotifyImmediate>();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_notify->GetToken() );                
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternNode< Expression > eexpr; 
            
    s_call->callee = s_lookup;
    //s_call->operands = ();
    s_lookup->object = eexpr;          
    eexpr->pattern = s_event;     // ensure base really evaluates to an event 
    s_lookup->member = s_token;        

    r_notify->event = eexpr;
       
    Configure( SEARCH_REPLACE, s_call, r_notify );
}


RaiseSCNotifyDelta::RaiseSCNotifyDelta()
{
    auto s_call = MakePatternNode<Call>();
    auto s_lookup = MakePatternNode<Lookup>();
    auto s_event = MakePatternNode<Event>();
    auto r_notify = MakePatternNode<NotifyDelta>();
    auto s_arg = MakePatternNode<MapOperand>();
    auto s_zero_token = MakePatternNode< InstanceIdentifierByNameAgent >( "SC_ZERO_TIME" );                
    auto s_arg_id = MakePatternNode< InstanceIdentifierByNameAgent >( "p1" ); 
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_notify->GetToken() );                
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternNode< Expression > eexpr; 
            
    s_call->callee = s_lookup;
    s_call->operands = (s_arg);
    s_arg->key = s_arg_id;
    s_arg->value = s_zero_token;        
    s_lookup->object = eexpr;          
    eexpr->pattern = s_event;     // ensure base really evaluates to an event 
    s_lookup->member = s_token;        

    r_notify->event = eexpr;
       
    Configure( SEARCH_REPLACE, s_call, r_notify );
}


RemoveEmptyModuleConstructors::RemoveEmptyModuleConstructors()
{
    auto module_typeid = MakePatternNode< TypeIdentifier >();
    auto stuff = MakePatternNode< Stuff<Scope> >();
    auto over = MakePatternNode< Delta<Scope> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_pre = MakePatternNode< Star<Statement> >();
    auto l_post = MakePatternNode< Star<Statement> >();
    auto l1s_args = MakePatternNode< Star<MapOperand> >();
    auto s_cons = MakePatternNode< Field >();
    auto s_comp = MakePatternNode< Compound >();
    auto s_id = MakePatternNode< InstanceIdentifier >();
    auto s_ctype = MakePatternNode<Constructor>();
    auto s_params = MakePatternNode< Star<Parameter> >();
    auto ls_comp = MakePatternNode< Compound >();
    auto lr_comp = MakePatternNode< Compound >();
    auto s_module = MakePatternNode< Module >();
    auto r_module = MakePatternNode< Module >();
    auto l1s_call = MakePatternNode< Call >();
    auto l1s_lookup = MakePatternNode< Lookup >();
    auto l_instance = MakePatternNode<Instance>();  
    auto l_delta = MakePatternNode<Delta<Initialiser>>();  
    auto l2s_call = MakePatternNode<Call>();
    auto l2s_lookup = MakePatternNode<Lookup>();
	auto l2s_args = MakePatternNode< Star<MapOperand> >();

    auto bases = MakePatternNode< Star<Base> >();
    auto r_embedded_2 = MakePatternNode< EmbeddedSearchReplace<Node> >( stuff, l_instance, l_instance );            
    auto r_embedded_1 = MakePatternNode< EmbeddedSearchReplace<Node> >( r_embedded_2, ls_comp, lr_comp );            
                    
    // dispense with an empty constructor                 
    stuff->terminus = over;
    over->through = s_module;
    over->overlay = r_module;
    s_module->members = (s_cons, decls);
    s_module->bases = (bases);
    s_module->identifier = module_typeid;
    s_cons->type = MakePatternNode<Constructor>();        
    s_cons->constancy = MakePatternNode<NonConst>();
    s_cons->initialiser = s_comp;
    s_cons->identifier = s_id;
    s_cons->type = s_ctype;
    s_ctype->params = (s_params); // any parameters
    r_module->members = (decls);
    r_module->bases = (bases);
    r_module->identifier = module_typeid;
            
    // Embedded 1: dispense with any calls to it from member inits
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_pre, l1s_call, l_post);
    l1s_call->callee = l1s_lookup;
    l1s_call->operands = (l1s_args); // any number of args, it doesn't matter, ctor is still empty so does nothing
    l1s_lookup->object = MakePatternNode< InstanceIdentifier >();
    l1s_lookup->member = s_id;        
    lr_comp->members = (l_decls);
    lr_comp->statements = (l_pre, l_post);

    // Embedded 2: dispense with any calls to it from instances
    l_instance->type = module_typeid;
    l_instance->initialiser = l_delta;
    l_delta->through = l2s_call;
    l_delta->overlay = MakePatternNode< Uninitialised >();
    l2s_call->callee = l2s_lookup;
    l2s_call->operands = (l2s_args); // any number of args, it doesn't matter, ctor is still empty so does nothing
    l2s_lookup->object = l_instance->identifier;
    l2s_lookup->member = s_id;
            
    Configure( COMPARE_REPLACE, stuff, r_embedded_1 );
}


RemoveVoidInstances::RemoveVoidInstances()
{
    auto s_scope = MakePatternNode<Program>();
    auto r_scope = MakePatternNode<Program>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto s_instance = MakePatternNode<Static>();
    auto s_any = MakePatternNode< Disjunction<Type> >();
    auto s_callable = MakePatternNode<CallableParams>();
    auto s_params = MakePatternNode< Star<Parameter> >();
    auto s_void_param = MakePatternNode<Parameter>();
    
    // Eliminate the declaration that came from isystemc.h
    s_scope->members = (decls, s_instance);
    s_instance->type = s_any;
    s_any->disjuncts = (s_callable, MakePatternNode<Void>() ); // match void instances (pointless) or functions as below...
    s_callable->params = (s_params, s_void_param); // one void param is enough, but don't match on zero params
    s_void_param->type = MakePatternNode<Void>();
    
    r_scope->members = (decls);   
       
    Configure( COMPARE_REPLACE, s_scope, r_scope );
}


RemoveSCPrototypes::RemoveSCPrototypes()
{
    auto s_scope = MakePatternNode<Program>();
    auto r_scope = MakePatternNode<Program>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto s_any = MakePatternNode< Disjunction<Instance> >();
    
    auto s_cease_inst = MakePatternNode<Static>();
    auto s_cease_type = MakePatternNode<Function>();   
    auto s_cease_param = MakePatternNode<Parameter>();   
    auto s_exit_inst = MakePatternNode<Static>();
    auto s_exit_type = MakePatternNode<Function>();   
    auto s_exit_param = MakePatternNode<Parameter>();   
    auto s_wait_inst = MakePatternNode<Static>();
    auto s_wait_type = MakePatternNode<Function>();   
    auto s_wait_param = MakePatternNode<Parameter>();   
    auto s_next_trigger_inst = MakePatternNode<Static>();
    auto s_next_trigger_type = MakePatternNode<Function>();   
    auto s_next_trigger_param = MakePatternNode<Parameter>();   
    auto s_delta_count_inst = MakePatternNode<Static>();
    auto s_delta_count_type = MakePatternNode<Function>();   

    auto s_unsigned_char = MakePatternNode<Unsigned>();   
    auto s_int = MakePatternNode<Signed>();   
    auto s_int2 = MakePatternNode<Signed>();   
    auto s_int3 = MakePatternNode<Signed>();   
    
    s_scope->members = (decls, s_any);
    s_any->disjuncts = (s_cease_inst, s_exit_inst, s_wait_inst, s_next_trigger_inst, s_delta_count_inst);
    
    // void cease( unsigned char exit_code );
    s_cease_inst->identifier = MakePatternNode< InstanceIdentifierByNameAgent >( "cease" ); 
    s_cease_inst->type = s_cease_type;
    s_cease_type->return_type = MakePatternNode<Void>();
    s_cease_type->params = (s_cease_param);
    s_cease_param->type = s_unsigned_char;
    s_unsigned_char->width = MakePatternNode<SpecificInteger>(8);
    s_cease_param->identifier = MakePatternNode<InstanceIdentifierByNameAgent>( "exit_code" );   
    
    // void exit( int exit_code );
    s_exit_inst->identifier = MakePatternNode< InstanceIdentifierByNameAgent >( "exit" ); 
    s_exit_inst->type = s_exit_type;
    s_exit_type->return_type = MakePatternNode<Void>();
    s_exit_type->params = (s_exit_param);
    s_exit_param->type = s_int;
    s_int->width = MakePatternNode<SpecificInteger>(32);
    s_exit_param->identifier = MakePatternNode<InstanceIdentifierByNameAgent>( "exit_code" );   
    
    // void wait( int p1 );
    s_wait_inst->identifier = MakePatternNode< InstanceIdentifierByNameAgent >( "wait" ); 
    s_wait_inst->type = s_wait_type;
    s_wait_type->return_type = MakePatternNode<Void>();
    s_wait_type->params = (s_wait_param);
    s_wait_param->type = s_int2;
    s_int2->width = MakePatternNode<SpecificInteger>(32);
    s_wait_param->identifier = MakePatternNode<InstanceIdentifierByNameAgent>( "p1" );   
    
    // void next_trigger( int p1 );
    s_next_trigger_inst->identifier = MakePatternNode< InstanceIdentifierByNameAgent >( "next_trigger" ); 
    s_next_trigger_inst->type = s_next_trigger_type;
    s_next_trigger_type->return_type = MakePatternNode<Void>();
    s_next_trigger_type->params = (s_next_trigger_param);
    s_next_trigger_param->type = s_int3;
    s_int3->width = MakePatternNode<SpecificInteger>(32);
    s_next_trigger_param->identifier = MakePatternNode<InstanceIdentifierByNameAgent>( "p1" );   
    
    // void sc_delta_count();
    s_delta_count_inst->identifier = MakePatternNode< InstanceIdentifierByNameAgent >( "sc_delta_count" ); 
    s_delta_count_inst->type = s_delta_count_type;
    s_delta_count_type->return_type = MakePatternNode<Void>();
    //s_delta_count_type->params = ();
    
    r_scope->members = (decls);   


    Configure( COMPARE_REPLACE, s_scope, r_scope );
}


void SystemCRaising::Build( vector< shared_ptr<VNStep> > *sequence )
{
    sequence->push_back( make_shared<RaiseSCType>( MakePatternNode<Event>() ) );
    sequence->push_back( make_shared<RaiseSCHierarchicalClass>( MakePatternNode<Module>() ) );
    sequence->push_back( make_shared<RaiseSCHierarchicalClass>( MakePatternNode<Interface>() ) );
    sequence->push_back( make_shared<RaiseSCDynamic>( MakePatternNode<WaitDynamic>() ) );
    sequence->push_back( make_shared<RaiseSCStatic>( MakePatternNode<WaitStatic>() ) );
    sequence->push_back( make_shared<RaiseSCDelta>( MakePatternNode<WaitDelta>() ) );
    sequence->push_back( make_shared<RaiseSCDynamic>( MakePatternNode<NextTriggerDynamic>() ) );
    sequence->push_back( make_shared<RaiseSCStatic>( MakePatternNode<NextTriggerStatic>() ) );
    sequence->push_back( make_shared<RaiseSCDelta>( MakePatternNode<NextTriggerDelta>() ) );
    sequence->push_back( make_shared<RaiseTerminationFunction>( MakePatternNode<Exit>() ) );
    sequence->push_back( make_shared<RaiseTerminationFunction>( MakePatternNode<Cease>() ) );
    sequence->push_back( make_shared<RaiseSCProcess>( MakePatternNode<Thread>() ) );
    sequence->push_back( make_shared<RaiseSCProcess>( MakePatternNode<ClockedThread>() ) );
    sequence->push_back( make_shared<RaiseSCProcess>( MakePatternNode<Method>() ) );
    sequence->push_back( make_shared<RaiseSCDeltaCount>() );
    sequence->push_back( make_shared<RaiseSCNotifyImmediate>() );
    sequence->push_back( make_shared<RaiseSCNotifyDelta>() );
    sequence->push_back( make_shared<RemoveEmptyModuleConstructors>() );
    sequence->push_back( make_shared<RemoveVoidInstances>() );
    sequence->push_back( make_shared<RemoveSCPrototypes>() );
    sequence->push_back( make_shared<CleanupUnusedVariables>() );    // for SC_ZERO_TIME
}

