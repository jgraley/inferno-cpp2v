
#include "systemc_detection.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_patterns.hpp"

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
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< UserType > s_usertype;
        MakeTreePtr< SCNODE > lr_scnode;
        MakeTreePtr< TypeIdentifierByName > s_token( lr_scnode->GetToken() );                
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, s_token, lr_scnode );    
        
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
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls, l_decls;
        MakeTreePtr< Star<Base> > l_bases;
        MakeTreePtr< UserType > s_usertype;
        MakeTreePtr< SCCLASS > lr_scclass;
        MakeTreePtr< InheritanceRecord > ls_class;
        MakeTreePtr< Base > ls_base;            
        MakeTreePtr< TypeIdentifier > l_tid;
        MakeTreePtr< TypeIdentifierByName > s_token( lr_scclass->GetToken() ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_class, lr_scclass );    
        
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
        MakeTreePtr< SCFUNC > r_dynamic;
        MakeTreePtr< Call > s_call;
        MakeTreePtr< MapOperand > s_arg;            
        MakeTreePtr< InstanceIdentifierByName > s_token( r_dynamic->GetToken() ); 
        MakeTreePtr< InstanceIdentifierByName > s_param_id( "p1" ); 
        MakeTreePtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
                        
        s_call->callee = s_token;       
        s_call->operands = (s_arg);
        s_arg->identifier = s_param_id;
        s_arg->value = eexpr;
        eexpr->pattern = MakeTreePtr<Event>();
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
        MakeTreePtr< SCFUNC > r_static;
        MakeTreePtr< Call > s_call;
        MakeTreePtr< InstanceIdentifierByName > s_token( r_static->GetToken() ); 
                          
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
        MakeTreePtr< SCFUNC > r_delta;
        MakeTreePtr< Call > s_call;
        MakeTreePtr< MapOperand > s_arg;            
        MakeTreePtr< InstanceIdentifierByName > s_token( r_delta->GetToken() ); 
        MakeTreePtr< InstanceIdentifierByName > s_param_id( "p1" ); 
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "SC_ZERO_TIME" ); 
                        
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
        MakeTreePtr< SCFUNC > r_tf;
        MakeTreePtr< Expression > event;
        MakeTreePtr< Call > s_call;
        MakeTreePtr< MapOperand > s_arg;            
        MakeTreePtr< InstanceIdentifierByName > s_token( r_tf->GetToken() ); 
        MakeTreePtr< InstanceIdentifierByName > s_param_id( "p1" ); 
                
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
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls, l_decls, l_cdecls;
        MakeTreePtr< Static > s_instance;
        MakeTreePtr< Compound > ls_comp, lr_comp;
        MakeTreePtr< Module > l_module;
        MakeTreePtr< Call > ls_pcall;
        MakeTreePtr< MapOperand > ls_arg;            
        MakeTreePtr< Overlay<Instance> > l_overcons;
        MakeTreePtr< Overlay<Type> > l_overtype;
        MakeTreePtr< Instance > ls_cons, lr_cons, l_process;
        MakeTreePtr< Star<Statement> > l_pre, l_post;
        MakeTreePtr< InstanceIdentifier > ls_id;
        MakeTreePtr< Star<Base> > l_bases;        
        MakeTreePtr<Constructor> l_ctype;
        MakeTreePtr< InstanceIdentifierByName > s_token( lr_scprocess->GetToken() ); 
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "func" );
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, l_module, l_module );            
        
        // Eliminate the declaration that came from isystemc.h
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_token;        
        s_instance->type = MakeTreePtr<Callable>(); // just narrow things a little        
        r_scope->members = (decls);   
                
        l_module->members = (l_overcons, l_process, l_decls);
        l_module->bases = (l_bases);
        l_overcons->through = ls_cons;
        ls_cons->type = MakeTreePtr<Constructor>();        
        ls_cons->initialiser = ls_comp;
        ls_comp->members = l_cdecls;
        ls_comp->statements = (l_pre, ls_pcall, l_post);
        ls_cons->type = l_ctype;
        l_ctype->members = (MakeTreePtr<Automatic>()); // one parameter
        ls_pcall->callee = s_token;
        ls_pcall->operands = (ls_arg);
        ls_arg->identifier = s_arg_id;
        ls_arg->value = ls_id;
        l_overcons->overlay = lr_cons;
        lr_cons->initialiser = lr_comp;
        lr_comp->members = l_cdecls;
        lr_comp->statements = (l_pre, l_post);
        lr_cons->type = l_ctype;
        
        l_process->identifier = ls_id;
        l_process->type = l_overtype;
        l_overtype->through = MakeTreePtr<Callable>();
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
        MakeTreePtr<Call> s_call;
        MakeTreePtr<Lookup> s_lookup;
        MakeTreePtr<Event> s_event;
        MakeTreePtr<NotifyImmediate> r_notify;
        MakeTreePtr< InstanceIdentifierByName > s_token( r_notify->GetToken() );                
        MakeTreePtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
        //MakeTreePtr< Expression > eexpr; 
                
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
        MakeTreePtr<Call> s_call;
        MakeTreePtr<Lookup> s_lookup;
        MakeTreePtr<Event> s_event;
        MakeTreePtr<NotifyDelta> r_notify;
        MakeTreePtr<MapOperand> s_arg;
        MakeTreePtr< InstanceIdentifierByName > s_zero_token( "SC_ZERO_TIME" );                
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "p1" ); 
        MakeTreePtr< InstanceIdentifierByName > s_token( r_notify->GetToken() );                
        MakeTreePtr< TransformOf<Expression> > eexpr( &TypeOf::instance ); 
        //MakeTreePtr< Expression > eexpr; 
                
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
        MakeTreePtr< Stuff<Scope> > stuff;
        MakeTreePtr< Overlay<Scope> > over;
        MakeTreePtr< Star<Declaration> > decls, l_decls;
        MakeTreePtr< Star<Statement> > l_pre, l_post;
        MakeTreePtr< Star<MapOperand> > ls_args;
        MakeTreePtr< Compound > s_comp, ls_comp, lr_comp;
        MakeTreePtr< Module > s_module, r_module;
        MakeTreePtr< Call > ls_call;
        MakeTreePtr< Lookup > ls_lookup;
        MakeTreePtr< Instance > s_cons;
        MakeTreePtr< InstanceIdentifier > s_id;
        MakeTreePtr< Star<Automatic> > s_params;
        MakeTreePtr<Constructor> s_ctype;
        MakeTreePtr< Star<Base> > bases;        
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( stuff, ls_comp, lr_comp );            
                        
        // dispense with an empty constructor                 
        stuff->terminus = over;
        over->through = s_module;
        over->overlay = r_module;
        s_module->members = (s_cons, decls);
        s_module->bases = (bases);
        s_cons->type = MakeTreePtr<Constructor>();        
        s_cons->initialiser = s_comp;
        // s_comp's members and statements left empty to signify empty constructor
        s_cons->identifier = s_id;
        s_cons->type = s_ctype;
        s_ctype->members = (s_params); // any parameters
        r_module->members = (decls);
        r_module->bases = (bases);
                
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
        MakeTreePtr<Program> s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr<Static> s_instance;
        MakeTreePtr< MatchAny<Type> > s_any;
        MakeTreePtr<CallableParams> s_callable;
        MakeTreePtr< Star<Instance> > s_params;
        MakeTreePtr<Instance> s_void_param;
        
        // Eliminate the declaration that came from isystemc.h
        s_scope->members = (decls, s_instance);
        s_instance->type = s_any;
        s_any->patterns = (s_callable, MakeTreePtr<Void>() ); // match void instances (pointless) or functions as below...
        s_callable->members = (s_params, s_void_param); // one void param is enough, but don't match no params
        s_void_param->type = MakeTreePtr<Void>();
        
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
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<Thread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<ClockedThread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<Method>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCNotifyImmediate ) );    
    push_back( shared_ptr<Transformation>( new DetectSCNotifyDelta ) );    
    push_back( shared_ptr<Transformation>( new RemoveEmptyModuleConstructors ) );    
    push_back( shared_ptr<Transformation>( new RemoveVoidInstances ) );    
}

