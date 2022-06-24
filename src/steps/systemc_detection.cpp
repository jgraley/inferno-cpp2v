
#include "systemc_detection.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


DetectSCType::DetectSCType( TreePtr< SCNamedConstruct > lr_scnode )
{
    auto over = MakePatternPtr< Delta<Node> >();
    auto s_scope = MakePatternPtr< Scope >();
    auto r_scope = MakePatternPtr< Scope >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto s_usertype = MakePatternPtr< UserType >();
    auto s_token = MakePatternPtr< TypeIdentifierByNameAgent >( lr_scnode->GetToken() );                
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Node> >( over, s_token, lr_scnode );    
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_usertype);
    s_usertype->identifier = s_token;
    
    r_scope->members = (decls);          
       
    Configure( COMPARE_REPLACE, over, r_slave );
}


DetectSCBase::DetectSCBase( TreePtr< SCNamedRecord > lr_scclass )
{
    auto over = MakePatternPtr< Delta<Node> >();
    auto s_scope = MakePatternPtr< Scope >();
    auto r_scope = MakePatternPtr< Scope >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto l_decls = MakePatternPtr< Star<Declaration> >();
    auto l_bases = MakePatternPtr< Star<Base> >();
    auto s_usertype = MakePatternPtr< UserType >();
    auto ls_class = MakePatternPtr< InheritanceRecord >();
    auto ls_base = MakePatternPtr< Base >();
    auto l_tid = MakePatternPtr< TypeIdentifier >();
    auto s_token = MakePatternPtr< TypeIdentifierByNameAgent >( lr_scclass->GetToken() ); 
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Node> >( over, ls_class, lr_scclass );    
    
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
       
    Configure( COMPARE_REPLACE, over, r_slave );
}


DetectSCDynamic::DetectSCDynamic( TreePtr<SCDynamicNamedFunction> r_dynamic )
{
    auto s_call = MakePatternPtr< Call >();
    auto s_arg = MakePatternPtr< MapOperand >();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_dynamic->GetToken() ); 
    auto s_param_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "p1" ); 
    auto eexpr = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = eexpr;
    eexpr->pattern = MakePatternPtr<Event>();
    r_dynamic->event = eexpr;       
      
    Configure( SEARCH_REPLACE, s_call, r_dynamic );
}


DetectSCStatic::DetectSCStatic( TreePtr<SCNamedFunction> r_static )
{
    auto s_call = MakePatternPtr< Call >();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_static->GetToken() ); 
                      
    s_call->callee = s_token;   
    //s_call->operands = ();       
       
    Configure( SEARCH_REPLACE, s_call, r_static );
}


DetectSCDelta::DetectSCDelta( TreePtr<SCNamedFunction> r_delta )
{
    auto s_call = MakePatternPtr< Call >();
    auto s_arg = MakePatternPtr< MapOperand >();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_delta->GetToken() ); 
    auto s_param_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "p1" ); 
    auto s_arg_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "SC_ZERO_TIME" ); 
                    
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = s_arg_id;
      
    Configure( SEARCH_REPLACE, s_call, r_delta );
}


DetectTerminationFunction::DetectTerminationFunction( TreePtr<TerminationFunction> r_tf )
{
    auto event = MakePatternPtr< Expression >();
    auto s_call = MakePatternPtr< Call >();
    auto s_arg = MakePatternPtr< MapOperand >();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_tf->GetToken() ); 
    auto s_param_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "p1" ); 
            
    s_call->callee = s_token;       
    s_call->operands = (s_arg);
    s_arg->identifier = s_param_id;
    s_arg->value = event;
    r_tf->code = event;       
      
    Configure( SEARCH_REPLACE, s_call, r_tf );
}


DetectSCProcess::DetectSCProcess( TreePtr< Process > lr_scprocess )
{
    auto over = MakePatternPtr< Delta<Node> >();
    auto s_scope = MakePatternPtr< Scope >();
    auto r_scope = MakePatternPtr< Scope >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto l_decls = MakePatternPtr< Star<Declaration> >();
    auto l_cdecls = MakePatternPtr< Star<Declaration> >();
    auto s_instance = MakePatternPtr< Static >();
    auto ls_comp = MakePatternPtr< Compound >();
    auto lr_comp = MakePatternPtr< Compound >();
    auto l_module = MakePatternPtr< Module >();
    auto ls_pcall = MakePatternPtr< Call >();
    auto ls_arg = MakePatternPtr< MapOperand >();
    auto l_overcons = MakePatternPtr< Delta<Instance> >();
    auto l_overtype = MakePatternPtr< Delta<Type> >();
    auto ls_cons = MakePatternPtr< Instance >();
    auto lr_cons = MakePatternPtr< Instance >();
    auto l_process = MakePatternPtr< Instance >();
    auto l_pre = MakePatternPtr< Star<Statement> >();
    auto l_post = MakePatternPtr< Star<Statement> >();
    auto ls_id = MakePatternPtr< InstanceIdentifier >();
    auto l_bases = MakePatternPtr< Star<Base> >();
    auto l_ctype = MakePatternPtr<Constructor>();
    auto l_ident = MakePatternPtr<InstanceIdentifier>();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( lr_scprocess->GetToken() ); 
    auto s_arg_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "func" );
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Node> >( over, l_module, l_module );            
    
    // Eliminate the declaration that came from isystemc.h
    over->through = s_scope;
    over->overlay = r_scope;
    s_scope->members = (decls, s_instance);
    s_instance->identifier = s_token;        
    s_instance->type = MakePatternPtr<Callable>(); // just narrow things a little        
    r_scope->members = (decls);   
            
    l_module->members = (l_overcons, l_process, l_decls);
    l_module->bases = (l_bases);
    l_overcons->through = ls_cons;       
    ls_cons->initialiser = ls_comp;
    ls_comp->members = l_cdecls;
    ls_comp->statements = (l_pre, ls_pcall, l_post);
    ls_cons->type = l_ctype;
    ls_cons->identifier = l_ident;
    l_ctype->members = (MakePatternPtr<Automatic>()); // one parameter
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
    l_overtype->through = MakePatternPtr<Callable>();
    l_overtype->overlay = lr_scprocess;
    
    Configure( COMPARE_REPLACE, over, r_slave );
}


DetectSCNotifyImmediate::DetectSCNotifyImmediate()
{
    auto s_call = MakePatternPtr<Call>();
    auto s_lookup = MakePatternPtr<Lookup>();
    auto s_event = MakePatternPtr<Event>();
    auto r_notify = MakePatternPtr<NotifyImmediate>();
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_notify->GetToken() );                
    auto eexpr = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternPtr< Expression > eexpr; 
            
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
    auto s_call = MakePatternPtr<Call>();
    auto s_lookup = MakePatternPtr<Lookup>();
    auto s_event = MakePatternPtr<Event>();
    auto r_notify = MakePatternPtr<NotifyDelta>();
    auto s_arg = MakePatternPtr<MapOperand>();
    auto s_zero_token = MakePatternPtr< InstanceIdentifierByNameAgent >( "SC_ZERO_TIME" );                
    auto s_arg_id = MakePatternPtr< InstanceIdentifierByNameAgent >( "p1" ); 
    auto s_token = MakePatternPtr< InstanceIdentifierByNameAgent >( r_notify->GetToken() );                
    auto eexpr = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternPtr< Expression > eexpr; 
            
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
    auto stuff = MakePatternPtr< Stuff<Scope> >();
    auto over = MakePatternPtr< Delta<Scope> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto l_decls = MakePatternPtr< Star<Declaration> >();
    auto l_pre = MakePatternPtr< Star<Statement> >();
    auto l_post = MakePatternPtr< Star<Statement> >();
    auto ls_args = MakePatternPtr< Star<MapOperand> >();
    auto s_comp = MakePatternPtr< Compound >();
    auto ls_comp = MakePatternPtr< Compound >();
    auto lr_comp = MakePatternPtr< Compound >();
    auto s_module = MakePatternPtr< Module >();
    auto r_module = MakePatternPtr< Module >();
    auto ls_call = MakePatternPtr< Call >();
    auto ls_lookup = MakePatternPtr< Lookup >();
    auto s_cons = MakePatternPtr< Instance >();
    auto s_id = MakePatternPtr< InstanceIdentifier >();
    auto s_params = MakePatternPtr< Star<Automatic> >();
    auto s_ctype = MakePatternPtr<Constructor>();
    auto bases = MakePatternPtr< Star<Base> >();
    auto module_typeid = MakePatternPtr< TypeIdentifier >();
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Node> >( stuff, ls_comp, lr_comp );            
                    
    // dispense with an empty constructor                 
    stuff->terminus = over;
    over->through = s_module;
    over->overlay = r_module;
    s_module->members = (s_cons, decls);
    s_module->bases = (bases);
    s_module->identifier = module_typeid;
    s_cons->type = MakePatternPtr<Constructor>();        
    s_cons->initialiser = s_comp;
    // s_comp's members and statements left empty to signify empty constructor
    s_cons->identifier = s_id;
    s_cons->type = s_ctype;
    s_ctype->members = (s_params); // any parameters
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
            
    Configure( COMPARE_REPLACE, stuff, r_slave );
}


RemoveVoidInstances::RemoveVoidInstances()
{
    auto s_scope = MakePatternPtr<Program>();
    auto r_scope = MakePatternPtr<Program>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto s_instance = MakePatternPtr<Static>();
    auto s_any = MakePatternPtr< Disjunction<Type> >();
    auto s_callable = MakePatternPtr<CallableParams>();
    auto s_params = MakePatternPtr< Star<Instance> >();
    auto s_void_param = MakePatternPtr<Instance>();
    
    // Eliminate the declaration that came from isystemc.h
    s_scope->members = (decls, s_instance);
    s_instance->type = s_any;
    s_any->disjuncts = (s_callable, MakePatternPtr<Void>() ); // match void instances (pointless) or functions as below...
    s_callable->members = (s_params, s_void_param); // one void param is enough, but don't match no params
    s_void_param->type = MakePatternPtr<Void>();
    
    r_scope->members = (decls);   
       
    Configure( COMPARE_REPLACE, s_scope, r_scope );
}


void DetectAllSCTypes::Build( vector< shared_ptr<VNTransformation> > *sequence )
{
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCType( MakePatternPtr<Event>() ) ) );
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCBase( MakePatternPtr<Module>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCBase( MakePatternPtr<Interface>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCDynamic( MakePatternPtr<WaitDynamic>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCStatic( MakePatternPtr<WaitStatic>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCDelta( MakePatternPtr<WaitDelta>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCDynamic( MakePatternPtr<NextTriggerDynamic>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCStatic( MakePatternPtr<NextTriggerStatic>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCDelta( MakePatternPtr<NextTriggerDelta>() ) ) );        
    sequence->push_back( shared_ptr<VNTransformation>( new DetectTerminationFunction( MakePatternPtr<Exit>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectTerminationFunction( MakePatternPtr<Cease>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCProcess( MakePatternPtr<Thread>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCProcess( MakePatternPtr<ClockedThread>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCProcess( MakePatternPtr<Method>() ) ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCNotifyImmediate ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new DetectSCNotifyDelta ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new RemoveEmptyModuleConstructors ) );    
    sequence->push_back( shared_ptr<VNTransformation>( new RemoveVoidInstances ) );    
}

