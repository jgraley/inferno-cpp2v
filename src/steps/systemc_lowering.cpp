
#include "systemc_lowering.hpp"
#include "clean_up.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "vn/agents/all.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


LowerSCType::LowerSCType( TreePtr< Type > s_sctype )
{
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_sctype->GetToken() );                

    Configure( SEARCH_REPLACE, s_sctype, r_token );
}


EnsureConstructorsInSCRecordUsers::EnsureConstructorsInSCRecordUsers()
{
	auto s_scclass = MakePatternNode< SCRecord >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto s_decls_negation = MakePatternNode< Negation<Declaration> >();
    auto sn_cons = MakePatternNode< Field >();
    auto sn_cons_type = MakePatternNode< Constructor >();
    auto sn_params = MakePatternNode< Star<Parameter> >();
    
	auto r_scclass = MakePatternNode< SCRecord >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scclass->GetToken() ); 
    auto r_cons_macro = MakePatternNode< MacroField >(); 
    auto r_comp = MakePatternNode< Compound >();
    auto r_id = MakePatternNode<BuildInstanceIdentifierAgent>(""); // constructor id 
    auto r_ctype = MakePatternNode<Constructor>();
    auto r_params = MakePatternNode< Star<Parameter> >();
    auto sc_ctor_macro_name = MakePatternNode< SpecificPreprocessorIdentifier >( "SC_CTOR" ); // #819 style

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
    r_scclass->members = (decls, r_cons_macro); 
    r_scclass->bases = (bases);    
    r_cons_macro->type = MakePatternNode<Constructor>();        
    r_cons_macro->constancy = MakePatternNode<NonConst>();
    r_cons_macro->access = MakePatternNode< Public >();
    r_cons_macro->virt = MakePatternNode< NonVirtual >();
    r_cons_macro->initialiser = r_comp;
    r_cons_macro->identifier = r_id;
    r_cons_macro->type = r_ctype;
    r_cons_macro->arguments = (tid);
    r_cons_macro->name = sc_ctor_macro_name;
    //r_ctype->params = ();

    Configure( SEARCH_REPLACE, s_scclass, r_scclass );
}

LowerSCHierarchicalClass::LowerSCHierarchicalClass( TreePtr< SCRecord > s_scclass )
{
	auto stuff = MakePatternNode<Stuff<Scope>>();
	auto delta = MakePatternNode<Delta<Scope>>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto r_class = MakePatternNode< Class >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scclass->GetToken() ); 
    auto r_cons = MakePatternNode< Field >(); // TODO for SC_CTOR(ClassName) add MacroField
    auto r_id = MakePatternNode<BuildInstanceIdentifierAgent>(""); // constructor id 
    auto r_ctype = MakePatternNode<Constructor>();
    auto r_params = MakePatternNode< Star<Parameter> >();
    
    auto l1_class = MakePatternNode< InheritanceRecord >();
    auto l1_fields = MakePatternNode< Star<Declaration> >();
    auto l1_bases = MakePatternNode< Star<Base> >();
    auto l1_statements = MakePatternNode< Star<Statement> >();
    auto l1_statements_negation = MakePatternNode< Negation<Statement> >();
    auto l1n_call = MakePatternNode<ExteriorCall>();
    auto l1n_lookup = MakePatternNode<Lookup>();
    auto l1_decls = MakePatternNode< Star<Declaration> >();
    auto l1_cons = MakePatternNode< Field >(); // TODO for SC_CTOR(ClassName) add MacroField
    auto l1_cons_type = MakePatternNode< Constructor >();
    auto l1_params = MakePatternNode< Star<Parameter> >();
    auto l1s_comp = MakePatternNode< Compound >();
    auto l1r_comp = MakePatternNode< Compound >();
    auto l1_field = MakePatternNode< Field >();
    auto l1_field_id = MakePatternNode< InstanceIdentifier >();
    auto l1_delta = MakePatternNode<Delta<Initialiser>>();  
    auto l1r_call = MakePatternNode<ExteriorCall>();
    auto l1r_lookup = MakePatternNode<Lookup>();
	auto l1r_arg = MakePatternNode< StringizeAgent >();
    
    auto l2_conjunction = MakePatternNode<Conjunction<Instance>>();  
    auto l2_negation = MakePatternNode<Negation<Instance>>();  
    auto l2_instance = MakePatternNode<Instance>();  
    auto l2_inst_id = MakePatternNode<InstanceIdentifier>();  
    auto l2_delta = MakePatternNode<Delta<Initialiser>>();  
    auto l2r_call = MakePatternNode<ExteriorCall>();
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
       
    // Field decl of our module in some OTHER class: add ExteriorCall to all constructors
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
	l1n_call->arguments = MakePatternNode<Star<Expression>>();
	l1n_lookup->object = l1_field_id; // this should be enough to prevent spin
	// l1_field_id is instance of a SC class and is not constructed in SC language
	
    l1r_call->callee = l1r_lookup;
    l1r_call->arguments = (l1r_arg);
    l1r_arg->source =  tid;
    l1r_lookup->object = l1_field_id;
    l1r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor ExteriorCall
                    
    // Static decl of our module: init to ExteriorCall
    // Not fields: they can't have constructor calls as intiialisers
    l2_conjunction->conjuncts = (l2_instance, l2_negation);
    l2_negation->negand = MakePatternNode<Field>();
    l2_instance->type = tid;
    l2_instance->initialiser = l2_delta;
    l2_instance->identifier = l2_inst_id;
    l2_delta->through = MakePatternNode< Uninitialised >();
    l2_delta->overlay = l2r_call;
    l2r_call->callee = l2r_lookup;
    l2r_call->arguments = (l2r_arg);
    l2r_arg->source =  tid;
    l2r_lookup->object = l2_instance->identifier;
    l2r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor ExteriorCall
                                                           
    auto embedded_2 = MakePatternNode< EmbeddedSearchReplace<Node> >( stuff, l2_conjunction, l2_instance );                         
    auto embedded_1 = MakePatternNode< EmbeddedSearchReplace<Node> >( embedded_2, l1_class, l1_class );                         
    Configure( COMPARE_REPLACE, stuff, embedded_1 );
}


LowerSCDynamic::LowerSCDynamic( TreePtr<SCDynamicFunction> s_dynamic,
                                TreePtr<InstanceIdentifier> r_dest )                              
{
    auto r_call = MakePatternNode< ExteriorCall >();
    // TODO IdValuePair args can't render without a function decl. Maybe add OperandSequence as an alternative? 
    auto event_expr = MakePatternNode< Expression >(); 
                    
    s_dynamic->event = event_expr;       
    r_call->callee = r_dest;       
    r_call->arguments = (event_expr);
      
    Configure( SEARCH_REPLACE, s_dynamic, r_call );
}


LowerSCStatic::LowerSCStatic( TreePtr<SCFunction> s_static,
                              TreePtr<InstanceIdentifier> r_dest )
{
    auto r_call = MakePatternNode< ExteriorCall >();
                        
    r_call->callee = r_dest;       
      
    Configure( SEARCH_REPLACE, s_static, r_call );
}


LowerSCDelta::LowerSCDelta( TreePtr<SCFunction> s_delta,
                            TreePtr<InstanceIdentifier> r_dest,
                            TreePtr<CPPTree::InstanceIdentifier> zero_time_id )
{
    auto r_call = MakePatternNode< ExteriorCall >();
                    
    r_call->callee = r_dest;       
    r_call->arguments = (zero_time_id);
          
    Configure( SEARCH_REPLACE, s_delta, r_call );
}


LowerTerminationFunction::LowerTerminationFunction( TreePtr<SCTree::TerminationFunction> s_tf )
{
    auto r_call = MakePatternNode< ExteriorCall >();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_tf->GetToken() ); 
    // TODO IdValuePair args can't render without a function decl. Maybe add OperandSequence as an alternative? 
    auto exit_expr = MakePatternNode< Expression >(); 
                    
    s_tf->code = exit_expr;       
    r_call->callee = r_token;       
    r_call->arguments = (exit_expr);
      
    Configure( SEARCH_REPLACE, s_tf, r_call );
}


LowerSCProcess::LowerSCProcess( TreePtr< SCTree::Process > s_scprocess )
{
    auto decls = MakePatternNode< Star<Declaration> >();
    auto cdecls = MakePatternNode< Star<Declaration> >();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto module = MakePatternNode< Module >();
    auto r_process_macro = MakePatternNode< MacroStatement >();
    auto overcons = MakePatternNode< Delta<Instance> >();
    auto overtype = MakePatternNode< Delta<Type> >();
    auto s_cons = MakePatternNode< Instance >();
    auto r_cons = MakePatternNode< Instance >();
    auto process = MakePatternNode< Instance >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto statements_negation = MakePatternNode< Negation<Statement> >();    
    auto sn_process_macro = MakePatternNode< MacroStatement >();
    auto id = MakePatternNode< InstanceIdentifier >(); 
    auto bases = MakePatternNode< Star<Base> >();
    auto ctype = MakePatternNode<Constructor>();
    auto ident = MakePatternNode<InstanceIdentifier>();
    auto token = MakePatternNode< SpecificPreprocessorIdentifier >( s_scprocess->GetToken() ); // #819 style
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
    statements_negation->negand = sn_process_macro;
    sn_process_macro->arguments = (id);
    
    // ctype->params = (); // no parameters
    overcons->overlay = r_cons;
    r_cons->identifier = ident;
    r_cons->type = ctype;
    r_cons->initialiser = r_comp;
    r_comp->members = cdecls;
    r_comp->statements = (pre, r_process_macro);
    r_process_macro->name = token;
    r_process_macro->arguments = (id);
    
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
    auto r_call = MakePatternNode<ExteriorCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_event = MakePatternNode<Event>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetToken() );                
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    //MakePatternNode< Expression > eexpr; 
            
    s_notify->event = eexpr;
            
    r_call->callee = r_lookup;
    //s_call->arguments = ();
    r_lookup->object = eexpr;          
    eexpr->pattern = r_event;     // ensure base really evaluates to an event 
    r_lookup->member = r_token;        
       
    Configure( SEARCH_REPLACE, s_notify, r_call );
}


LowerSCNotifyDelta::LowerSCNotifyDelta(TreePtr<CPPTree::InstanceIdentifier> zero_time_id)
{
    auto s_notify = MakePatternNode<NotifyDelta>();
    auto eexpr = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance ); 
    auto event = MakePatternNode<Event>();
    
    auto r_call = MakePatternNode<ExteriorCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetToken() );                
    //MakePatternNode< Expression > eexpr; 
            
    s_notify->event = eexpr;
    eexpr->pattern = event;     // ensure base really evaluates to an event 

    r_call->callee = r_lookup;
    r_call->arguments = (zero_time_id);
    r_lookup->object = eexpr;          
    r_lookup->member = r_token;        
       
    Configure( SEARCH_REPLACE, s_notify, r_call );
}


LowerSCDeltaCount::LowerSCDeltaCount()
{
    auto s_delta_count = MakePatternNode<DeltaCount>();
 
    auto r_call = MakePatternNode<ExteriorCall>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_delta_count->GetToken() );                
    //MakePatternNode< Expression > eexpr; 
            
    r_call->callee = r_token;
    //r_call->arguments = (); // no operands
       
    Configure( SEARCH_REPLACE, s_delta_count, r_call );
}


AddIncludeSystemC::AddIncludeSystemC()
{
	string filename = "systemc.h";
	
    auto s_program = MakePatternNode<Program>();
    auto r_program = MakePatternNode<Program>();
    auto decls = MakePatternNode<Star<Declaration>>();
    auto declstuff = MakePatternNode<Stuff<Declaration>>();
    auto r_include = MakePatternNode<SystemInclude>();
    auto s_negation = MakePatternNode<Negation<Declaration>>();
    auto sn_include = MakePatternNode<SystemInclude>();

    s_program->members = (decls, declstuff);
    decls->restriction = s_negation;
    declstuff->terminus = MakePatternNode<SCNode>(); 
    s_negation->negand = sn_include;
    sn_include->filename = MakePatternNode<SpecificString>(filename);   
    r_program->members = (decls, declstuff, r_include);
    r_include->filename = MakePatternNode<SpecificString>(filename);   

    Configure( COMPARE_REPLACE, s_program, r_program );
}


AddIncludeSCExtensions::AddIncludeSCExtensions()
{
	string filename = "systemc_extensions.h";

    auto s_program = MakePatternNode<Program>();
    auto r_program = MakePatternNode<Program>();
    auto decls = MakePatternNode<Star<Declaration>>();
    auto declstuff = MakePatternNode<Stuff<Declaration>>();
    auto r_include = MakePatternNode<LocalInclude>();
    auto s_negation = MakePatternNode<Negation<Declaration>>();
    auto sn_include = MakePatternNode<LocalInclude>();

    s_program->members = (decls, declstuff);
    decls->restriction = s_negation;
    declstuff->terminus = MakePatternNode<SCExtension>(); 
    s_negation->negand = sn_include;
    sn_include->filename = MakePatternNode<SpecificString>(filename);   
    r_program->members = (decls, declstuff, r_include);
    r_include->filename = MakePatternNode<SpecificString>(filename);   

    Configure( COMPARE_REPLACE, s_program, r_program );
}


void SystemCLowering::Build( vector< shared_ptr<VNStep> > *sequence )
{
    sequence->push_back( make_shared<AddIncludeSystemC>() );
    sequence->push_back( make_shared<AddIncludeSCExtensions>() );

	auto zero_time_id = MakePatternNode< SpecificInstanceIdentifier >( "SC_ZERO_TIME" );
	
	// The reverse ordering of SystemCRaising::Build()
	sequence->push_back( make_shared<LowerSCNotifyDelta>(zero_time_id) );
	sequence->push_back( make_shared<LowerSCNotifyImmediate>() );
	sequence->push_back( make_shared<LowerSCDeltaCount>() );
	
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
    
    // TODO renderer don't use try/catch, use IsDeclared()    
}
