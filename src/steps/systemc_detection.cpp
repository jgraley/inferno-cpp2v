
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
class DetectSCType : public CompareReplace
{
public:
    DetectSCType()
    {
        MakeTreePtr< Overlay<Node> > over;
        MakeTreePtr< Scope > s_scope, r_scope;
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< UserType > s_usertype;
        MakeTreePtr< SCNODE > lr_scnode;
        MakeTreePtr< TypeIdentifierByName > s_id( lr_scnode->GetName() );                
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, s_id, lr_scnode );    
        
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_usertype);
        s_usertype->identifier = s_id;
        
        r_scope->members = (decls);          
           
        CompareReplace::Configure( over, r_slave );
    }
};


/// spot SystemC base class by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all class nodes
 that inherit from the suppleid base with the new inferno node and 
 remove the base */
template< class SCCLASS>
class DetectSCBase : public CompareReplace
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
        MakeTreePtr< TypeIdentifierByName > s_id( lr_scclass->GetName() ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_class, lr_scclass );    
        
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_usertype);
        s_usertype->identifier = s_id;        
        r_scope->members = (decls);   
        
        ls_class->identifier = l_tid;       
        ls_class->members = (l_decls);
        ls_class->bases = (l_bases, ls_base);       
        ls_base->record = s_id;
        lr_scclass->identifier = l_tid;       
        lr_scclass->members = (l_decls);
        lr_scclass->bases = (l_bases);
           
        CompareReplace::Configure( over, r_slave );
    }
};


/// spot SystemC wait() function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's MapOperator style. */
class DetectSCWaitParm : public CompareReplace
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
        MakeTreePtr< InstanceIdentifierByName > s_id( "wait" ); 
        MakeTreePtr< InstanceIdentifierByName > s_arg_id( "e" ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_call, lr_wait );    
        
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_id;        
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
        r_scope->members = (decls);   
        
        ls_call->callee = s_id;       
        ls_call->operands = (ls_arg);
        ls_arg->identifier = s_arg_id;
        ls_arg->value = l_event;
        lr_wait->event = l_event;       
           
        CompareReplace::Configure( over, r_slave );
    }
};


class DetectSCWaitNoParm : public CompareReplace
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
        MakeTreePtr< InstanceIdentifierByName > s_id( "wait" ); 
        MakeTreePtr< SlaveSearchReplace<Node> > r_slave( over, ls_call, lr_wait );    
        
        over->through = s_scope;
        over->overlay = r_scope;
        s_scope->members = (decls, s_instance);
        s_instance->identifier = s_id;        
        s_instance->type = MakeTreePtr<Subroutine>(); // just narrow things a little        
        r_scope->members = (decls);   
        
        ls_call->callee = s_id;       
        lr_wait->event = MakeTreePtr<Uninitialised>();       
           
        CompareReplace::Configure( over, r_slave );
    }
};


DetectAllSCTypes::DetectAllSCTypes()
{
    push_back( shared_ptr<Transformation>( new DetectSCType<Event> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Module> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCBase<Interface> ) );    
    push_back( shared_ptr<Transformation>( new DetectSCWaitParm ) );        
    push_back( shared_ptr<Transformation>( new DetectSCWaitNoParm ) );    
}

