
#include "systemc_detection.hpp"
#include "clean_up.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


DetectSCType::DetectSCType( TreePtr< SCNamedConstruct > lr_scnode )
{
    auto over = MakePatternNode< Delta<Node> >();
    auto s_scope = MakePatternNode< Scope >();
    auto r_scope = MakePatternNode< Scope >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto s_usertype = MakePatternNode< UserType >();
    auto s_token = MakePatternNode< TypeIdentifierByNameAgent >( lr_scnode->GetToken() );                
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Node> >( over, s_token, lr_scnode );    
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_usertype);
    s_usertype->identifier = s_token;
    
    r_scope->members = (decls);          
       
    Configure( COMPARE_REPLACE, over, r_embedded );
}


DetectSCBase::DetectSCBase( TreePtr< SCNamedRecord > lr_scclass )
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


DetectSCDynamic::DetectSCDynamic( TreePtr<SCDynamicNamedFunction> r_dynamic )
{
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_dynamic->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "p1" ); 
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = eexpr;
    eexpr->pattern = MakePatternNode<Event>();
    r_dynamic->event = eexpr;       
      
    Configure( SEARCH_REPLACE, s_call, r_dynamic );
}


DetectSCStatic::DetectSCStatic( TreePtr<SCNamedFunction> r_static )
{
    auto s_call = MakePatternNode< Call >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_static->GetToken() ); 
                      
    s_call->callee = s_token;   
    //s_call->operands = ();       
       
    Configure( SEARCH_REPLACE, s_call, r_static );
}


DetectSCDelta::DetectSCDelta( TreePtr<SCNamedFunction> r_delta )
{
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_delta->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "p1" ); 
    auto s_arg_id = MakePatternNode< InstanceIdentifierByNameAgent >( "SC_ZERO_TIME" ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = s_arg_id;
      
    Configure( SEARCH_REPLACE, s_call, r_delta );
}


DetectTerminationFunction::DetectTerminationFunction( TreePtr<TerminationFunction> r_tf )
{
    auto event = MakePatternNode< Expression >();
    auto s_call = MakePatternNode< Call >();
    auto s_arg = MakePatternNode< MapOperand >();
    auto s_token = MakePatternNode< InstanceIdentifierByNameAgent >( r_tf->GetToken() ); 
    auto s_param_id = MakePatternNode< InstanceIdentifierByNameAgent >( "exit_code" ); 
            
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = event;
    r_tf->code = event;       
      
    Configure( SEARCH_REPLACE, s_call, r_tf );
}


DetectSCProcess::DetectSCProcess( TreePtr< Process > lr_scprocess )
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
    ls_arg->identifier = s_arg_id;
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


DetectSCNotifyImmediate::DetectSCNotifyImmediate()
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
    s_lookup->base = eexpr;          
    eexpr->pattern = s_event;     // ensure base really evaluates to an event 
    s_lookup->member = s_token;        

    r_notify->event = eexpr;
       
    Configure( SEARCH_REPLACE, s_call, r_notify );
}


DetectSCNotifyDelta::DetectSCNotifyDelta()
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
    s_arg->identifier = s_arg_id;
    s_arg->value = s_zero_token;        
    s_lookup->base = eexpr;          
    eexpr->pattern = s_event;     // ensure base really evaluates to an event 
    s_lookup->member = s_token;        

    r_notify->event = eexpr;
       
    Configure( SEARCH_REPLACE, s_call, r_notify );
}


RemoveEmptyModuleConstructors::RemoveEmptyModuleConstructors()
{
    auto stuff = MakePatternNode< Stuff<Scope> >();
    auto over = MakePatternNode< Delta<Scope> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_pre = MakePatternNode< Star<Statement> >();
    auto l_post = MakePatternNode< Star<Statement> >();
    auto ls_args = MakePatternNode< Star<MapOperand> >();
    auto s_comp = MakePatternNode< Compound >();
    auto ls_comp = MakePatternNode< Compound >();
    auto lr_comp = MakePatternNode< Compound >();
    auto s_module = MakePatternNode< Module >();
    auto r_module = MakePatternNode< Module >();
    auto ls_call = MakePatternNode< Call >();
    auto ls_lookup = MakePatternNode< Lookup >();
    auto s_cons = MakePatternNode< Instance >();
    auto s_id = MakePatternNode< InstanceIdentifier >();
    auto s_params = MakePatternNode< Star<Parameter> >();
    auto s_ctype = MakePatternNode<Constructor>();
    auto bases = MakePatternNode< Star<Base> >();
    auto module_typeid = MakePatternNode< TypeIdentifier >();
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Node> >( stuff, ls_comp, lr_comp );            
                    
    // dispense with an empty constructor                 
    stuff->terminus = over;
    over->through = s_module;
    over->overlay = r_module;
    s_module->members = (s_cons, decls);
    s_module->bases = (bases);
    s_module->identifier = module_typeid;
    s_cons->type = MakePatternNode<Constructor>();        
    s_cons->initialiser = s_comp;
    // s_comp's members and statements left empty to signify empty constructor
    s_cons->identifier = s_id;
    s_cons->type = s_ctype;
    s_ctype->params = (s_params); // any parameters
    r_module->members = (decls);
    r_module->bases = (bases);
    r_module->identifier = module_typeid;
            
    // dispense with any calls to it
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_pre, ls_call, l_post);
    ls_call->callee = ls_lookup;
    ls_call->operands = ls_args; // any number of args, it doesn't matter, ctor is still empty so does nothing
    ls_lookup->member = s_id;        
    lr_comp->members = (l_decls);
    lr_comp->statements = (l_pre, l_post);
            
    Configure( COMPARE_REPLACE, stuff, r_embedded );
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
    s_callable->params = (s_params, s_void_param); // one void param is enough, but don't match no params
    s_void_param->type = MakePatternNode<Void>();
    
    r_scope->members = (decls);   
       
    Configure( COMPARE_REPLACE, s_scope, r_scope );
}


void DetectAllSCTypes::Build( vector< shared_ptr<VNStep> > *sequence )
{
    sequence->push_back( make_shared<DetectSCType>( MakePatternNode<Event>() ) );
    sequence->push_back( make_shared<DetectSCBase>( MakePatternNode<Module>() ) );
    sequence->push_back( make_shared<DetectSCBase>( MakePatternNode<Interface>() ) );
    sequence->push_back( make_shared<DetectSCDynamic>( MakePatternNode<WaitDynamic>() ) );
    sequence->push_back( make_shared<DetectSCStatic>( MakePatternNode<WaitStatic>() ) );
    sequence->push_back( make_shared<DetectSCDelta>( MakePatternNode<WaitDelta>() ) );
    sequence->push_back( make_shared<DetectSCDynamic>( MakePatternNode<NextTriggerDynamic>() ) );
    sequence->push_back( make_shared<DetectSCStatic>( MakePatternNode<NextTriggerStatic>() ) );
    sequence->push_back( make_shared<DetectSCDelta>( MakePatternNode<NextTriggerDelta>() ) );
    sequence->push_back( make_shared<DetectTerminationFunction>( MakePatternNode<Exit>() ) );
    sequence->push_back( make_shared<DetectTerminationFunction>( MakePatternNode<Cease>() ) );
    sequence->push_back( make_shared<DetectSCProcess>( MakePatternNode<Thread>() ) );
    sequence->push_back( make_shared<DetectSCProcess>( MakePatternNode<ClockedThread>() ) );
    sequence->push_back( make_shared<DetectSCProcess>( MakePatternNode<Method>() ) );
    sequence->push_back( make_shared<DetectSCNotifyImmediate>() );
    sequence->push_back( make_shared<DetectSCNotifyDelta>() );
    sequence->push_back( make_shared<RemoveEmptyModuleConstructors>() );
    sequence->push_back( make_shared<RemoveVoidInstances>() );
    sequence->push_back( make_shared<CleanupUnusedVariables>() );    // for SC_ZERO_TIME
}

