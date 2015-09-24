
#include "systemc_detection.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;



/// spot SystemC type by its name and replace with inferno node 
/** We look for the decl and remeove it since the inferno
 Node does not require declaration. Then just switch each appearance
 over to the new node, using a slave */
template< class SCNODE>
class DetectSCType : public CompareReplace  // Note not SearchReplace
{
public:
    DetectSCType()
    {
        MakePatternPtr< Overlay<Node> > over;
        MakePatternPtr< Scope > s_scope, r_scope;
        MakePatternPtr< Star<Declaration> > decls;
        MakePatternPtr< UserType > s_usertype;
        MakePatternPtr< SCNODE > lr_scnode;
        MakePatternPtr< TypeIdentifierByName > s_token( lr_scnode->GetToken() );                
        MakePatternPtr< SlaveSearchReplace<Node> > r_slave( over, s_token, lr_scnode );    
        
        // Eliminate the declaration that came from isystemc.h
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_usertype);
        s_usertype->identifier = s_token;
        
        r_scope->members = (decls);          
           
        Configure( over, r_slave );
    }
};


/// spot SystemC base class by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all class nodes
 that inherit from the suppleid base with the new inferno node and 
 remove the base */
template< class SCCLASS>
class DetectSCBase : public CompareReplace  // Note not SearchReplace
{
public:
    DetectSCBase()
    {
        MakePatternPtr< Overlay<Node> > over;
        MakePatternPtr< Scope > s_scope, r_scope;
        MakePatternPtr< Star<Declaration> > decls, l_decls;
        MakePatternPtr< Star<Base> > l_bases;
        MakePatternPtr< UserType > s_usertype;
        MakePatternPtr< SCCLASS > lr_scclass;
        MakePatternPtr< InheritanceRecord > ls_class;
        MakePatternPtr< Base > ls_base;            
        MakePatternPtr< TypeIdentifier > l_tid;
        MakePatternPtr< TypeIdentifierByName > s_token( lr_scclass->GetToken() ); 
        MakePatternPtr< SlaveSearchReplace<Node> > r_slave( over, ls_class, lr_scclass );    
        
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
           
        Configure( over, r_slave );
    }
};


template<class SCFUNC>
class DetectSCDynamic : public SearchReplace
{
public:
    DetectSCDynamic()
    {
        MakePatternPtr< SCFUNC > r_dynamic;
        MakePatternPtr< Call > s_call;
        MakePatternPtr< MapOperand > s_arg;            
        MakePatternPtr< InstanceIdentifierByName > s_token( r_dynamic->GetToken() ); 
        MakePatternPtr< InstanceIdentifierByName > s_param_id( "p1" ); 
        MakePatternPtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
                        
        s_call->callee = s_token;       
        s_call->operands = (s_arg);
        s_arg->identifier = s_param_id;
        s_arg->value = eexpr;
        eexpr->pattern = MakePatternPtr<Event>();
        r_dynamic->event = eexpr;       
          
        Configure( s_call, r_dynamic );
    }
};

template<class SCFUNC>
class DetectSCStatic : public SearchReplace
{
public:
    DetectSCStatic()
    {
        MakePatternPtr< SCFUNC > r_static;
        MakePatternPtr< Call > s_call;
        MakePatternPtr< InstanceIdentifierByName > s_token( r_static->GetToken() ); 
                          
        s_call->callee = s_token;   
        //s_call->operands = ();       
           
        Configure( s_call, r_static );
    }
};

template<class SCFUNC>
class DetectSCDelta : public SearchReplace
{
public:
    DetectSCDelta()
    {
        MakePatternPtr< SCFUNC > r_delta;
        MakePatternPtr< Call > s_call;
        MakePatternPtr< MapOperand > s_arg;            
        MakePatternPtr< InstanceIdentifierByName > s_token( r_delta->GetToken() ); 
        MakePatternPtr< InstanceIdentifierByName > s_param_id( "p1" ); 
        MakePatternPtr< InstanceIdentifierByName > s_arg_id( "SC_ZERO_TIME" ); 
                        
        s_call->callee = s_token;       
        s_call->operands = (s_arg);
        s_arg->identifier = s_param_id;
        s_arg->value = s_arg_id;
          
        Configure( s_call, r_delta );
    }
};


/// spot syscall exit() or equivalent function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's MapOperator style. */
template<class SCFUNC>
class DetectTerminationFunction : public SearchReplace
{
public:
    DetectTerminationFunction()
    {
        MakePatternPtr< SCFUNC > r_tf;
        MakePatternPtr< Expression > event;
        MakePatternPtr< Call > s_call;
        MakePatternPtr< MapOperand > s_arg;            
        MakePatternPtr< InstanceIdentifierByName > s_token( r_tf->GetToken() ); 
        MakePatternPtr< InstanceIdentifierByName > s_param_id( "p1" ); 
                
        s_call->callee = s_token;       
        s_call->operands = (s_arg);
        s_arg->identifier = s_param_id;
        s_arg->value = event;
        r_tf->code = event;       
          
        Configure( s_call, r_tf );
    }
};


class DetectSCProcess : public CompareReplace // Note not SearchReplace
{
public:
    DetectSCProcess( TreePtr< Process > lr_scprocess )
    {
        MakePatternPtr< Overlay<Node> > over;
        MakePatternPtr< Scope > s_scope, r_scope;
        MakePatternPtr< Star<Declaration> > decls, l_decls, l_cdecls;
        MakePatternPtr< Static > s_instance;
        MakePatternPtr< Compound > ls_comp, lr_comp;
        MakePatternPtr< Module > l_module;
        MakePatternPtr< Call > ls_pcall;
        MakePatternPtr< MapOperand > ls_arg;            
        MakePatternPtr< Overlay<Instance> > l_overcons;
        MakePatternPtr< Overlay<Type> > l_overtype;
        MakePatternPtr< Instance > ls_cons, lr_cons, l_process;
        MakePatternPtr< Star<Statement> > l_pre, l_post;
        MakePatternPtr< InstanceIdentifier > ls_id;
        MakePatternPtr< Star<Base> > l_bases;        
        MakePatternPtr<Constructor> l_ctype;
        MakePatternPtr<InstanceIdentifier> l_ident;
        MakePatternPtr< InstanceIdentifierByName > s_token( lr_scprocess->GetToken() ); 
        MakePatternPtr< InstanceIdentifierByName > s_arg_id( "func" );
        MakePatternPtr< SlaveSearchReplace<Node> > r_slave( over, l_module, l_module );            
        
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
        
        Configure( over, r_slave );
    }
};



/// spot SystemC notify() method by its name and replace with inferno node 
/** Look for myevent.notify() and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class DetectSCNotifyImmediate : public SearchReplace  
{
public:
    DetectSCNotifyImmediate()
    {
        MakePatternPtr<Call> s_call;
        MakePatternPtr<Lookup> s_lookup;
        MakePatternPtr<Event> s_event;
        MakePatternPtr<NotifyImmediate> r_notify;
        MakePatternPtr< InstanceIdentifierByName > s_token( r_notify->GetToken() );                
        MakePatternPtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
        //MakePatternPtr< Expression > eexpr; 
                
        s_call->callee = s_lookup;
        //s_call->operands = ();
        s_lookup->base = eexpr;          
        eexpr->pattern = s_event;     // ensure base really evaluates to an event 
        s_lookup->member = s_token;        

        r_notify->event = eexpr;
           
        Configure( s_call, r_notify );
    }
};


/// spot SystemC notify(SC_ZERO_TIME) method by its name and replace with inferno node 
/** Look for myevent.notify(SC_ZERO_TIME) and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class DetectSCNotifyDelta : public SearchReplace  
{
public:
    DetectSCNotifyDelta()
    {
        MakePatternPtr<Call> s_call;
        MakePatternPtr<Lookup> s_lookup;
        MakePatternPtr<Event> s_event;
        MakePatternPtr<NotifyDelta> r_notify;
        MakePatternPtr<MapOperand> s_arg;
        MakePatternPtr< InstanceIdentifierByName > s_zero_token( "SC_ZERO_TIME" );                
        MakePatternPtr< InstanceIdentifierByName > s_arg_id( "p1" ); 
        MakePatternPtr< InstanceIdentifierByName > s_token( r_notify->GetToken() );                
        MakePatternPtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
        //MakePatternPtr< Expression > eexpr; 
                
        s_call->callee = s_lookup;
        s_call->operands = (s_arg);
        s_arg->identifier = s_arg_id;
        s_arg->value = s_zero_token;        
        s_lookup->base = eexpr;          
        eexpr->pattern = s_event;     // ensure base really evaluates to an event 
        s_lookup->member = s_token;        

        r_notify->event = eexpr;
           
        Configure( s_call, r_notify );
    }
};


/// Remove constructors in SC modules that are now empty thanks to earlier steps
/// Must also remove explicit calls to constructor (which would not do anything)
class RemoveEmptyModuleConstructors : public CompareReplace
{
public:
    RemoveEmptyModuleConstructors()
    {
        MakePatternPtr< Stuff<Scope> > stuff;
        MakePatternPtr< Overlay<Scope> > over;
        MakePatternPtr< Star<Declaration> > decls, l_decls;
        MakePatternPtr< Star<Statement> > l_pre, l_post;
        MakePatternPtr< Star<MapOperand> > ls_args;
        MakePatternPtr< Compound > s_comp, ls_comp, lr_comp;
        MakePatternPtr< Module > s_module, r_module;
        MakePatternPtr< Call > ls_call;
        MakePatternPtr< Lookup > ls_lookup;
        MakePatternPtr< Instance > s_cons;
        MakePatternPtr< InstanceIdentifier > s_id;
        MakePatternPtr< Star<Automatic> > s_params;
        MakePatternPtr<Constructor> s_ctype;
        MakePatternPtr< Star<Base> > bases;        
        MakePatternPtr< TypeIdentifier > module_typeid;        
        MakePatternPtr< SlaveSearchReplace<Node> > r_slave( stuff, ls_comp, lr_comp );            
                        
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
                
        Configure( stuff, r_slave );
    }
};


/// Remove top-level instances that are of type void
/** isystemc.h declares void variables to satisfy parser. Hoover them all up
    efficiently here. */
class RemoveVoidInstances : public CompareReplace  // Note not SearchReplace
{
public:
    RemoveVoidInstances()
    {
        MakePatternPtr<Program> s_scope, r_scope;
        MakePatternPtr< Star<Declaration> > decls;
        MakePatternPtr<Static> s_instance;
        MakePatternPtr< MatchAny<Type> > s_any;
        MakePatternPtr<CallableParams> s_callable;
        MakePatternPtr< Star<Instance> > s_params;
        MakePatternPtr<Instance> s_void_param;
        
        // Eliminate the declaration that came from isystemc.h
        s_scope->members = (decls, s_instance);
        s_instance->type = s_any;
        s_any->patterns = (s_callable, MakePatternPtr<Void>() ); // match void instances (pointless) or functions as below...
        s_callable->members = (s_params, s_void_param); // one void param is enough, but don't match no params
        s_void_param->type = MakePatternPtr<Void>();
        
        r_scope->members = (decls);   
           
        Configure( s_scope, r_scope );
    }
};




DetectAllSCTypes::DetectAllSCTypes()
{
    push_back( shared_ptr<Transformation>( new DetectSCType<Event> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Module> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Interface> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCDynamic<WaitDynamic> ) );        
    push_back( shared_ptr<Transformation>( new DetectSCStatic<WaitStatic> ) );        
    push_back( shared_ptr<Transformation>( new DetectSCDelta<WaitDelta> ) );        
    push_back( shared_ptr<Transformation>( new DetectSCDynamic<NextTriggerDynamic> ) );        
    push_back( shared_ptr<Transformation>( new DetectSCStatic<NextTriggerStatic> ) );        
    push_back( shared_ptr<Transformation>( new DetectSCDelta<NextTriggerDelta> ) );        
    push_back( shared_ptr<Transformation>( new DetectTerminationFunction<Exit> ) );    
    push_back( shared_ptr<Transformation>( new DetectTerminationFunction<Cease> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakePatternPtr<Thread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakePatternPtr<ClockedThread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakePatternPtr<Method>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCNotifyImmediate ) );    
    push_back( shared_ptr<Transformation>( new DetectSCNotifyDelta ) );    
    push_back( shared_ptr<Transformation>( new RemoveEmptyModuleConstructors ) );    
    push_back( shared_ptr<Transformation>( new RemoveVoidInstances ) );    
}

