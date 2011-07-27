
#include "systemc_detection.hpp"
#include "tree/cpptree.hpp"

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


/// spot SystemC wait() function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's MapOperator style. */
class DetectSCWaitParm : public CompareReplace  // Note not SearchReplace
{
public:
    DetectSCWaitParm()
    {
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls, l_decls;
        MakeTreePtr< Static > s_instance;
        MakeTreePtr< Wait > lr_wait;
        MakeTreePtr< Expression > l_event;
        MakeTreePtr< Call > ls_call;
        MakeTreePtr< MapOperand > ls_arg;            
        MakeTreePtr< InstanceIdentifierByName > s_token( "wait" ); 
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "e" ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_call, lr_wait );    
        
        // Eliminate the declaration that came from isystemc.h
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_token;        
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
        r_scope->members = (decls);   
        
        ls_call->callee = s_token;       
        ls_call->operands = (ls_arg);
        ls_arg->identifier = s_arg_id;
        ls_arg->value = l_event;
        lr_wait->event = l_event;       
           
        Configure( over, r_slave );
    }
};


class DetectSCWaitNoParm : public CompareReplace  // Note not SearchReplace
{
public:
    DetectSCWaitNoParm()
    {
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls, l_decls;
        MakeTreePtr< Static > s_instance;
        MakeTreePtr< Wait > lr_wait;
        MakeTreePtr< Call > ls_call;
        MakeTreePtr< InstanceIdentifierByName > s_token( "wait" ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_call, lr_wait );    
        
        // Eliminate the declaration that came from isystemc.h
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_token;        
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
        r_scope->members = (decls);   
        
        ls_call->callee = s_token;       
        lr_wait->event = MakeTreePtr<Uninitialised>();       
           
        Configure( over, r_slave );
    }
};


/// spot syscall exit() function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's MapOperator style. */
class DetectExit : public CompareReplace  // Note not SearchReplace
{
public:
    DetectExit()
    {
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls, l_decls;
        MakeTreePtr< Static > s_instance;
        MakeTreePtr< Exit > lr_exit;
        MakeTreePtr< Expression > l_code;
        MakeTreePtr< Call > ls_call;
        MakeTreePtr< MapOperand > ls_arg;            
        MakeTreePtr< InstanceIdentifierByName > s_token( "exit" ); 
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "code" ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_call, lr_exit );    
        
        // Eliminate the declaration that came from isystemc.h
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_token;        
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
        r_scope->members = (decls);   
        
        ls_call->callee = s_token;       
        ls_call->operands = (ls_arg);
        ls_arg->identifier = s_arg_id;
        ls_arg->value = l_code;
        lr_exit->code = l_code;       
           
        Configure( over, r_slave );
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
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
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
        l_overtype->through = MakeTreePtr<Subroutine>();
        l_overtype->overlay = lr_scprocess;
        
        Configure( over, r_slave );
    }
};

/// Remove constructors in SC modules that are now empty thanks to earlier steps
class RemoveEmptyModuleConstructors : public SearchReplace
{
public:
    RemoveEmptyModuleConstructors()
    {
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< Compound > s_comp;
        MakeTreePtr< Module > s_module, r_module;
        MakeTreePtr< Instance > s_cons;
        MakeTreePtr< Star<Automatic> > s_params;
        MakeTreePtr<Constructor> s_ctype;
        MakeTreePtr< Star<Base> > bases;        
                        
        s_module->members = (s_cons, decls);
        s_module->bases = (bases);
        s_cons->type = MakeTreePtr<Constructor>();        
        s_cons->initialiser = s_comp;
        // s_comp's members and statements left empty to signify empty constructor
        s_cons->type = s_ctype;
        s_ctype->members = (s_params); // any parameters
        r_module->members = (decls);
        r_module->bases = (bases);
                
        Configure( s_module, r_module );
    }
};





DetectAllSCTypes::DetectAllSCTypes()
{
    push_back( shared_ptr<Transformation>( new DetectSCType<Event> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Module> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Interface> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCWaitParm ) );        
    push_back( shared_ptr<Transformation>( new DetectSCWaitNoParm ) );    
    push_back( shared_ptr<Transformation>( new DetectExit ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<Thread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<ClockedThread>() ) ) );    
    push_back( shared_ptr<Transformation>( new DetectSCProcess( MakeTreePtr<Method>() ) ) );    
    push_back( shared_ptr<Transformation>( new RemoveEmptyModuleConstructors ) );    
}

