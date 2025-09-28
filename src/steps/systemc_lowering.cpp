
#include "systemc_lowering.hpp"
#include "clean_up.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "vn/agents/all.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


LowerSCType::LowerSCType( TreePtr< SCNamedConstruct > s_scnode )
{
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scnode->GetToken() );                

    Configure( SEARCH_REPLACE, s_scnode, r_token );
}

EnsureConstructorsInSCRecordUsers::EnsureConstructorsInSCRecordUsers()
{
	auto s_scclass = MakePatternNode< SCNamedRecord >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto s_decls_negation = MakePatternNode< Negation<Declaration> >();
    auto sn_cons = MakePatternNode< Field >();
    auto sn_cons_type = MakePatternNode< Constructor >();
    auto sn_params = MakePatternNode< Star<Parameter> >();
    
	auto r_scclass = MakePatternNode< SCNamedRecord >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scclass->GetToken() ); 
    auto r_cons = MakePatternNode< SysMacroField >(); 
    auto r_comp = MakePatternNode< Compound >();
    auto r_id = MakePatternNode<BuildInstanceIdentifierAgent>(""); // constructor id 
    auto r_ctype = MakePatternNode<Constructor>();
    auto r_params = MakePatternNode< Star<Parameter> >();

	// TODO not restricting properly: as per our name EnsureConstructorsInSCRecordUsers
	// we should add constructors to the USERS of SC classes (i.e. those that declare
	// one as a field). This will be the same set if every class is now an SC class.
	// But one day they may not be, as with small pod-resembling untility classes.
	// Actually we want the union of *is* SC and *uses* SC
	
    // Module to a class and add a constructor such as SC_CTOR(ClassName)    
    s_scclass->identifier = tid;       
    s_scclass->members = (decls);
    decls->restriction = s_decls_negation;
    s_decls_negation->negand = sn_cons;
    sn_cons->type = sn_cons_type;
    sn_cons_type->params = sn_params; // any constructor is enough to stop us
    s_scclass->bases = (bases);    
    r_scclass->identifier = tid;       
    r_scclass->members = (decls, r_cons); // TODO for SC_CTOR(ClassName) add SysMacroField;
    r_scclass->bases = (bases);    
    r_cons->type = MakePatternNode<Constructor>();        
    r_cons->constancy = MakePatternNode<NonConst>();
    r_cons->access = MakePatternNode< Public >();
    r_cons->virt = MakePatternNode< NonVirtual >();
    r_cons->initialiser = r_comp;
    r_cons->identifier = r_id;
    r_cons->type = r_ctype;
    r_cons->macro_operands = (tid);
    //r_ctype->params = ();

    Configure( SEARCH_REPLACE, s_scclass, r_scclass );
}

LowerSCHierarchicalClass::LowerSCHierarchicalClass( TreePtr< SCNamedRecord > s_scclass )
{
	auto stuff = MakePatternNode<Stuff<Scope>>();
	auto delta = MakePatternNode<Delta<Scope>>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto r_class = MakePatternNode< Class >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scclass->GetToken() ); 
    auto r_cons = MakePatternNode< Field >(); // TODO for SC_CTOR(ClassName) add SysMacroField
    auto r_id = MakePatternNode<BuildInstanceIdentifierAgent>(""); // constructor id 
    auto r_ctype = MakePatternNode<Constructor>();
    auto r_params = MakePatternNode< Star<Parameter> >();
    
    auto l1_class = MakePatternNode< InheritanceRecord >();
    auto l1_fields = MakePatternNode< Star<Declaration> >();
    auto l1_bases = MakePatternNode< Star<Base> >();
    auto l1_statements = MakePatternNode< Star<Statement> >();
    auto l1_statements_negation = MakePatternNode< Negation<Statement> >();
    auto l1n_call = MakePatternNode<SysCall>();
    auto l1n_lookup = MakePatternNode<Lookup>();
    auto l1_decls = MakePatternNode< Star<Declaration> >();
    auto l1_cons = MakePatternNode< Field >(); // TODO for SC_CTOR(ClassName) add SysMacroField
    auto l1_cons_type = MakePatternNode< Constructor >();
    auto l1_params = MakePatternNode< Star<Parameter> >();
    auto l1s_comp = MakePatternNode< Compound >();
    auto l1r_comp = MakePatternNode< Compound >();
    auto l1_field = MakePatternNode< Field >();
    auto l1_field_id = MakePatternNode< InstanceIdentifier >();
    auto l1_delta = MakePatternNode<Delta<Initialiser>>();  
    auto l1r_call = MakePatternNode<SysCall>();
    auto l1r_lookup = MakePatternNode<Lookup>();
	auto l1r_arg = MakePatternNode< StringizeAgent >();
    
    auto l2_conjunction = MakePatternNode<Conjunction<Instance>>();  
    auto l2_negation = MakePatternNode<Negation<Instance>>();  
    auto l2_instance = MakePatternNode<Instance>();  
    auto l2_inst_id = MakePatternNode<InstanceIdentifier>();  
    auto l2_delta = MakePatternNode<Delta<Initialiser>>();  
    auto l2r_call = MakePatternNode<SysCall>();
    auto l2r_lookup = MakePatternNode<Lookup>();
	auto l2r_arg = MakePatternNode< StringizeAgent >();
    
    // Module to a class 
    stuff->terminus = delta;
    delta->through = s_scclass;
    delta->overlay = r_class;
    s_scclass->identifier = tid;       
    s_scclass->members = (decls);
    s_scclass->bases = (bases);    
    r_class->identifier = tid;       
    r_class->members = (decls);
    r_class->bases = (bases, r_base);       
    r_base->record = r_token;
    r_base->access = MakePatternNode< Public >();
       
    // Field decl of our module in some OTHER class: add SysCall to all constructors
    l1_class->identifier = MakePatternNode< TypeIdentifier >(); // not tid, the OTHER class
    l1_class->members = (l1_fields, l1_cons, l1_field);
    l1_class->bases = (l1_bases);
    l1_cons->identifier = MakePatternNode< InstanceIdentifier >();
    l1_cons->initialiser = l1_delta;
    l1_cons->type = l1_cons_type;        
    l1_cons_type->params = l1_params;
    l1_delta->through = l1s_comp;
    l1_delta->overlay = l1r_comp;
    l1s_comp->members = (l1_decls);
    l1s_comp->statements = (l1_statements);
    l1r_comp->members = (l1_decls);
    l1r_comp->statements = (l1_statements, l1r_call);
    
    l1_field->type = tid;
    l1_field->identifier = l1_field_id;
    
	l1_statements->restriction = l1_statements_negation;
	l1_statements_negation->negand = l1n_call;
	l1n_call->callee = l1n_lookup;
	l1n_call->operands = MakePatternNode<Star<Expression>>();
	l1n_lookup->base = l1_field_id; // this should be enough to prevent spin
	// l1_field_id is instance of a SC class and is not constructed in SC language
	
    l1r_call->callee = l1r_lookup;
    l1r_call->operands = (l1r_arg);
    l1r_arg->source =  tid;
    l1r_lookup->base = l1_field_id;
    l1r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor SysCall
                    
    // Static decl of our module: init to SysCall
    // Not fields: they can't have constructor calls as intiialisers
    l2_conjunction->conjuncts = (l2_instance, l2_negation);
    l2_negation->negand = MakePatternNode<Field>();
    l2_instance->type = tid;
    l2_instance->initialiser = l2_delta;
    l2_instance->identifier = l2_inst_id;
    l2_delta->through = MakePatternNode< Uninitialised >();
    l2_delta->overlay = l2r_call;
    l2r_call->callee = l2r_lookup;
    l2r_call->operands = (l2r_arg);
    l2r_arg->source =  tid;
    l2r_lookup->base = l2_instance->identifier;
    l2r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor SysCall
                                                           
    auto embedded_2 = MakePatternNode< EmbeddedSearchReplace<Node> >( stuff, l2_conjunction, l2_instance );                         
    auto embedded_1 = MakePatternNode< EmbeddedSearchReplace<Node> >( embedded_2, l1_class, l1_class );                         
    Configure( COMPARE_REPLACE, stuff, embedded_1 );
}

 /*  auto r_construct = MakePatternNode<Field>();
    auto r_construct_type = MakePatternNode<SysCall>();
    auto r_construct_lu = MakePatternNode<Lookup>();
    auto r_param = MakePatternNode<Parameter>();
    auto r_param_ptr = MakePatternNode<Pointer>();
    
    r_construct->type = r_construct_type;
    r_construct->identifier = MakePatternNode< SpecificInstanceIdentifier >( "" );
    r_construct->initialsier = MakePatternNode< Uninitialised >();
    r_construct->virtuality = MakePatternNode< NonVirtual >();
    r_construct->access = MakePatternNode< Public >();
    r_construct->constancy = MakePatternNode< NonConst >();
    r_construct_type->callee = r_construct_lu;
    r_construct_type->operands = (r_param);
    r_param->type = r_param_ptr;
    r_param->identifier = MakePatternNode< SpecificInstanceIdentifier >( "" );;
    r_param_ptr->constancy = MakePatternNode< Const >
*/

LowerSCDynamic::LowerSCDynamic( TreePtr<SCDynamicNamedFunction> s_dynamic,
                                TreePtr<InstanceIdentifier> r_dest )                              
{
    auto r_call = MakePatternNode< SysCall >();
    // TODO MapOperand args can't render without a function decl. Maybe add OperandSequence as an alternative? 
    auto event_expr = MakePatternNode< Expression >(); 
                    
    s_dynamic->event = event_expr;       
    r_call->callee = r_dest;       
    r_call->operands = (event_expr);
      
    Configure( SEARCH_REPLACE, s_dynamic, r_call );
}


LowerSCStatic::LowerSCStatic( TreePtr<SCNamedFunction> s_static,
                              TreePtr<InstanceIdentifier> r_dest )
{
    auto r_call = MakePatternNode< SysCall >();
                        
    r_call->callee = r_dest;       
      
    Configure( SEARCH_REPLACE, s_static, r_call );
}


LowerSCDelta::LowerSCDelta( TreePtr<SCNamedFunction> s_delta,
                            TreePtr<InstanceIdentifier> r_dest,
                            TreePtr<CPPTree::InstanceIdentifier> zero_time_id )
{
    auto r_call = MakePatternNode< SysCall >();
                    
    r_call->callee = r_dest;       
    r_call->operands = (zero_time_id);
          
    Configure( SEARCH_REPLACE, s_delta, r_call );
}


LowerTerminationFunction::LowerTerminationFunction( TreePtr<SCTree::TerminationFunction> s_tf )
{
    auto r_call = MakePatternNode< SysCall >();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_tf->GetToken() ); 
    // TODO MapOperand args can't render without a function decl. Maybe add OperandSequence as an alternative? 
    auto exit_expr = MakePatternNode< Expression >(); 
                    
    s_tf->code = exit_expr;       
    r_call->callee = r_token;       
    r_call->operands = (exit_expr);
      
    Configure( SEARCH_REPLACE, s_tf, r_call );
}


LowerSCProcess::LowerSCProcess( TreePtr< SCTree::Process > s_scprocess )
{
    auto decls = MakePatternNode< Star<Declaration> >();
    auto cdecls = MakePatternNode< Star<Declaration> >();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto module = MakePatternNode< Module >();
    auto r_pcall = MakePatternNode< SysMacroCall >();
    auto overcons = MakePatternNode< Delta<Instance> >();
    auto overtype = MakePatternNode< Delta<Type> >();
    auto s_cons = MakePatternNode< Instance >();
    auto r_cons = MakePatternNode< Instance >();
    auto process = MakePatternNode< Instance >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto statements_negation = MakePatternNode< Negation<Statement> >();    
    auto sn_pcall = MakePatternNode< SysMacroCall >();
    auto id = MakePatternNode< InstanceIdentifier >(); 
    auto bases = MakePatternNode< Star<Base> >();
    auto ctype = MakePatternNode<Constructor>();
    auto ident = MakePatternNode<InstanceIdentifier>();
    auto token = MakePatternNode< SpecificInstanceIdentifier >( s_scprocess->GetToken() ); // #819 style
    auto r_func = MakePatternNode<Function>();
                
    module->members = (overcons, process, decls);
    module->bases = (bases);
    overcons->through = s_cons;       
    s_cons->identifier = ident;
    s_cons->type = ctype;
    s_cons->initialiser = s_comp;
    s_comp->members = cdecls;
    s_comp->statements = (pre);
    pre->restriction = statements_negation;
    statements_negation->negand = sn_pcall;
    sn_pcall->macro_operands = (id);
    
    // ctype->params = (); // no parameters
    overcons->overlay = r_cons;
    r_cons->identifier = ident;
    r_cons->type = ctype;
    r_cons->initialiser = r_comp;
    r_comp->members = cdecls;
    r_comp->statements = (pre, r_pcall);
    r_pcall->callee = token;
    r_pcall->macro_operands = (id);
    
    process->identifier = id;
    process->type = overtype;
    overtype->through = s_scprocess;
    overtype->overlay = r_func;
    r_func->return_type = MakePatternNode<Void>();
    
    Configure( SEARCH_REPLACE, module, module );
}


LowerSCNotifyImmediate::LowerSCNotifyImmediate()
{
    auto s_notify = MakePatternNode<NotifyImmediate>();
    auto r_call = MakePatternNode<SysCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_event = MakePatternNode<Event>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetToken() );                
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternNode< Expression > eexpr; 
            
    s_notify->event = eexpr;
            
    r_call->callee = r_lookup;
    //s_call->operands = ();
    r_lookup->base = eexpr;          
    eexpr->pattern = r_event;     // ensure base really evaluates to an event 
    r_lookup->member = r_token;        
       
    Configure( SEARCH_REPLACE, s_notify, r_call );
}


LowerSCNotifyDelta::LowerSCNotifyDelta(TreePtr<CPPTree::InstanceIdentifier> zero_time_id)
{
    auto s_notify = MakePatternNode<NotifyDelta>();
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    auto event = MakePatternNode<Event>();
    
    auto r_call = MakePatternNode<SysCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetToken() );                
    //MakePatternNode< Expression > eexpr; 
            
    s_notify->event = eexpr;
    eexpr->pattern = event;     // ensure base really evaluates to an event 

    r_call->callee = r_lookup;
    r_call->operands = (zero_time_id);
    r_lookup->base = eexpr;          
    r_lookup->member = r_token;        

       
    Configure( SEARCH_REPLACE, s_notify, r_call );
}

void LowerAllSystemC::Build( vector< shared_ptr<VNStep> > *sequence )
{
	auto zero_time_id = MakePatternNode< SpecificInstanceIdentifier >( "SC_ZERO_TIME" );
	// TODO do we need a way of calling constructors (for init lists) that is
	// determinable without needing TypeOf?
	
	// The reverse ordering of DetectAllSystemC::Build()
	// TODO renaming: sc_det -> sc_raising, sc_gen -> simple_sc_from_c
	sequence->push_back( make_shared<LowerSCNotifyDelta>(zero_time_id) );
	sequence->push_back( make_shared<LowerSCNotifyImmediate>() );
	
    sequence->push_back( make_shared<LowerTerminationFunction>( MakePatternNode<Cease>() ) );
    sequence->push_back( make_shared<LowerTerminationFunction>( MakePatternNode<Exit>() ) );

	// #819-style identifiers must be fully unique. In a sense, this line is the one and only
	// declaration of this system function, for the purposes of rendering.
	auto next_trigger_id = MakePatternNode< SpecificInstanceIdentifier >( "next_trigger" ); 
    sequence->push_back( make_shared<LowerSCDelta>( MakePatternNode<NextTriggerDelta>(), next_trigger_id, zero_time_id ) );
    sequence->push_back( make_shared<LowerSCStatic>( MakePatternNode<NextTriggerStatic>(), next_trigger_id ) );		
    sequence->push_back( make_shared<LowerSCDynamic>( MakePatternNode<NextTriggerDynamic>(), next_trigger_id ) );

	auto wait_id = MakePatternNode< SpecificInstanceIdentifier >( "wait" ); 
    sequence->push_back( make_shared<LowerSCDelta>( MakePatternNode<WaitDelta>(), wait_id, zero_time_id ) );
    sequence->push_back( make_shared<LowerSCStatic>( MakePatternNode<WaitStatic>(), wait_id ) );		
    sequence->push_back( make_shared<LowerSCDynamic>( MakePatternNode<WaitDynamic>(), wait_id ) );	

	// Precondition for LowerSCHierarchicalClass
    sequence->push_back( make_shared<EnsureConstructorsInSCRecordUsers>() );
    
    sequence->push_back( make_shared<LowerSCProcess>( MakePatternNode<Thread>() ) );
    sequence->push_back( make_shared<LowerSCProcess>( MakePatternNode<ClockedThread>() ) );
    sequence->push_back( make_shared<LowerSCProcess>( MakePatternNode<Method>() ) );
        
    sequence->push_back( make_shared<LowerSCHierarchicalClass>( MakePatternNode<Interface>() ) );
    sequence->push_back( make_shared<LowerSCHierarchicalClass>( MakePatternNode<Module>() ) );
    sequence->push_back( make_shared<LowerSCType>( MakePatternNode<Event>() ) );
    // TODO add a Sys node to cause the #include "isystemc.h"
    // TODO and why can't it be the real header?
    
    // TODO renderer don't use try/catch, use IsDeclared()
    
    // Lookup dont say base; MapOperand do say key
    
    // Get -ul out of gen_graphs
    
    // Route replace path around conjuntion to stop warning
}
