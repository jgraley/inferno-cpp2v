
#include "systemc_lowering.hpp"
#include "clean_up.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "pattern_helpers.hpp"

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


LowerSCType::LowerSCType( TreePtr< Type > s_sctype )
{
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_sctype->GetLoweredIdName() );                

    Configure( SEARCH_REPLACE, s_sctype, r_token );
}


EnsureConstructorsInSCRecordUsers::EnsureConstructorsInSCRecordUsers()
{
	auto s_scclass = MakePatternNode< SCRecord >();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto bases = MakePatternNode<StarAgent, Base>();
    auto s_decls_negation = MakePatternNode<NegationAgent, Declaration>();
    auto sx_cons_macro = MakePatternNode< MacroDeclaration >(); 
    auto sx_params = MakePatternNode<StarAgent, Node>();
    
	auto r_scclass = MakePatternNode< SCRecord >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( ""/*s_scclass->GetLoweredIdName() TODO SCRecord is intermediate and has no lowered id name */ ); 
    auto r_cons_macro = MakePatternNode< MacroDeclaration >(); 
    auto r_comp = MakePatternNode< Compound >();
    auto ctor_macro_name = MakePatternNode< SpecificPreprocessorIdentifier >( "SC_CTOR" ); // #819 style
    auto ctor_macro_byname = MakePatternNode< PreprocessorIdentifierByNameAgent >( "SC_CTOR" );                
    auto ctor_macro_wildname = MakePatternNode< PreprocessorIdentifier >(); // TODO too broad               

	// TODO not restricting properly: as per our name EnsureConstructorsInSCRecordUsers
	// we should add constructors to the USERS of SC classes (i.e. those that declare
	// one as a field). This will be the same set if every class is now an SC class.
	// But one day they may not be, as with small pod-resembling utility classes.
	// Actually we want the union of *is* SC and *uses* SC
	
    // Module to a class and add a constructor such as SC_CTOR(ClassName)    
    s_scclass->identifier = tid;       
    s_scclass->members = (decls);
    decls->restriction = s_decls_negation;
    s_decls_negation->negand = sx_cons_macro;
#ifdef RECREATE_856
    sx_cons_macro->identifier = ctor_macro_name;
#else
#ifdef RECREATE_857
    sx_cons_macro->identifier = ctor_macro_byname;
#else
    sx_cons_macro->identifier = ctor_macro_wildname;
#endif        
#endif    
    sx_cons_macro->arguments = sx_params;
    s_scclass->bases = (bases);    
    r_scclass->identifier = tid;       
    r_scclass->members = (decls, r_cons_macro); 
    r_scclass->bases = (bases);    
    r_cons_macro->initialiser = r_comp;
    r_cons_macro->arguments = (tid);
    r_cons_macro->identifier = ctor_macro_name;

    Configure( SEARCH_REPLACE, s_scclass, r_scclass );
}

LowerSCHierarchicalClass::LowerSCHierarchicalClass( TreePtr< SCRecord > s_scclass )
{
	auto stuff = MakePatternNode<StuffAgent, Scope>();
	auto delta = MakePatternNode<DeltaAgent, Scope>();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto bases = MakePatternNode<StarAgent, Base>();
    auto r_class = MakePatternNode< Class >();
    auto r_base = MakePatternNode< Base >();
    auto tid = MakePatternNode< TypeIdentifier >();
    auto r_token = MakePatternNode< SpecificTypeIdentifier >( s_scclass->GetLoweredIdName() ); 
    
    auto l1_class = MakePatternNode< InheritanceRecord >();
    auto l1_fields = MakePatternNode<StarAgent, Declaration>();
    auto l1_bases = MakePatternNode<StarAgent, Base>();
    auto l1_statements = MakePatternNode<StarAgent, Statement>();
    auto l1_statements_negation = MakePatternNode<NegationAgent, Statement>();
    auto l1x_call = MakePatternNode<SeqArgsCall>();
    auto l1x_lookup = MakePatternNode<Lookup>();
    auto l1_decls = MakePatternNode<StarAgent, Declaration>();
    auto l1_cons_macro = MakePatternNode< MacroDeclaration >(); 
    auto l1_macro_args = MakePatternNode<StarAgent, Node>();    
    auto l1s_comp = MakePatternNode< Compound >();
    auto l1r_comp = MakePatternNode< Compound >();
    auto l1_field = MakePatternNode< Field >();
    auto l1_field_id = MakePatternNode< InstanceIdentifier >();
    auto l1_delta = MakePatternNode<DeltaAgent, Initialiser>();  
    auto l1r_call = MakePatternNode<SeqArgsCall>();
    auto l1r_lookup = MakePatternNode<Lookup>();
	auto l1r_arg = MakePatternNode< StringizeAgent >();
    
    auto l2_conjunction = MakePatternNode<ConjunctionAgent, Instance>();  
    auto l2_negation = MakePatternNode<NegationAgent, Instance>();  
    auto l2_instance = MakePatternNode<Instance>();  
    auto l2_inst_id = MakePatternNode<InstanceIdentifier>();  
    auto l2_delta = MakePatternNode<DeltaAgent, Initialiser>();  
    auto l2r_call = MakePatternNode<SeqArgsCall>();
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
       
    // Field decl of our module in some OTHER class: add SeqArgsCall to all constructors
    l1_class->identifier = MakePatternNode< TypeIdentifier >(); // not tid, the OTHER class
    l1_class->members = (l1_fields, l1_cons_macro, l1_field);
    l1_class->bases = (l1_bases);
    l1_cons_macro->identifier = MakePatternNode< PreprocessorIdentifier >();
    l1_cons_macro->arguments = l1_macro_args;
    l1_cons_macro->initialiser = l1_delta;
    l1_delta->through = l1s_comp;
    l1_delta->overlay = l1r_comp;
    l1s_comp->members = (l1_decls);
    l1s_comp->statements = (l1_statements);
    l1r_comp->members = (l1_decls);
    l1r_comp->statements = (l1_statements, l1r_call);
    
    l1_field->type = tid;
    l1_field->identifier = l1_field_id;
    
	l1_statements->restriction = l1_statements_negation;
	l1_statements_negation->negand = l1x_call;
	l1x_call->callee = l1x_lookup;
	l1x_call->arguments = MakePatternNode<StarAgent, Expression>();
	l1x_lookup->object = l1_field_id; // this should be enough to prevent spin
	// l1_field_id is instance of a SC class and is not constructed in SC language
	
    l1r_call->callee = l1r_lookup;
    l1r_call->arguments = (l1r_arg);
    l1r_arg->source =  tid;
    l1r_lookup->object = l1_field_id;
    l1r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor SeqArgsCall
                    
    // Static decl of our module: init to SeqArgsCall
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
    l2r_lookup->member = MakePatternNode< SpecificInstanceIdentifier >(""); // Empty indicates constructor SeqArgsCall
                                                           
    auto embedded_2 = MakePatternNode<EmbeddedSearchReplaceAgent, Node>( stuff, l2_conjunction, l2_instance );                         
    auto embedded_1 = MakePatternNode<EmbeddedSearchReplaceAgent, Node>( embedded_2, l1_class, l1_class );                         
    Configure( COMPARE_REPLACE, stuff, embedded_1 );
}


LowerSCDynamic::LowerSCDynamic( TreePtr<SCDynamicFunction> s_dynamic,
                                TreePtr<InstanceIdentifier> r_dest )                              
{
    auto r_call = MakePatternNode< SeqArgsCall >();
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
    auto r_call = MakePatternNode< SeqArgsCall >();
                        
    r_call->callee = r_dest;       
      
    Configure( SEARCH_REPLACE, s_static, r_call );
}


LowerSCDelta::LowerSCDelta( TreePtr<SCFunction> s_delta,
                            TreePtr<InstanceIdentifier> r_dest,
                            TreePtr<CPPTree::InstanceIdentifier> zero_time_id )
{
    auto r_call = MakePatternNode< SeqArgsCall >();
                    
    r_call->callee = r_dest;       
    r_call->arguments = (zero_time_id);
          
    Configure( SEARCH_REPLACE, s_delta, r_call );
}


LowerTerminationFunction::LowerTerminationFunction( TreePtr<SCTree::TerminationFunction> s_tf )
{
    auto r_call = MakePatternNode< SeqArgsCall >();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_tf->GetLoweredIdName() ); 
    // TODO IdValuePair args can't render without a function decl. Maybe add OperandSequence as an alternative? 
    auto exit_expr = MakePatternNode< Expression >(); 
                    
    s_tf->code = exit_expr;       
    r_call->callee = r_token;       
    r_call->arguments = (exit_expr);
      
    Configure( SEARCH_REPLACE, s_tf, r_call );
}


LowerSCProcess::LowerSCProcess( TreePtr< SCTree::Process > s_scprocess )
{
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto cdecls = MakePatternNode<StarAgent, Declaration>();
    auto s_comp = MakePatternNode< Compound >();
    auto r_comp = MakePatternNode< Compound >();
    auto module = MakePatternNode< Module >();
    auto r_process_macro = MakePatternNode< MacroStatement >();
    auto overcons = MakePatternNode<DeltaAgent, Declaration>();
    auto overtype = MakePatternNode<DeltaAgent, Type>();
    auto s_cons_macro = MakePatternNode< MacroDeclaration >();
    auto r_cons_macro = MakePatternNode< MacroDeclaration >();
    auto macro_args = MakePatternNode<StarAgent, Node>();
    auto process = MakePatternNode< Instance >();
    auto pre = MakePatternNode<StarAgent, Statement>();
    auto statements_negation = MakePatternNode<NegationAgent, Statement>();    
    auto sx_process_macro = MakePatternNode< MacroStatement >();
    auto id = MakePatternNode< InstanceIdentifier >(); 
    auto bases = MakePatternNode<StarAgent, Base>();
    auto ident = MakePatternNode<PreprocessorIdentifier>();
    auto token = MakePatternNode< SpecificPreprocessorIdentifier >( s_scprocess->GetLoweredIdName() ); // #819 style
    auto r_func = MakePatternNode<Function>();
                
    module->members = (overcons, process, decls);
    module->bases = (bases);
    overcons->through = s_cons_macro;       
    s_cons_macro->identifier = ident;
    s_cons_macro->arguments = macro_args;
    s_cons_macro->initialiser = s_comp;
    s_comp->members = cdecls;
    s_comp->statements = (pre);
    pre->restriction = statements_negation;
    statements_negation->negand = sx_process_macro;
    sx_process_macro->arguments = (id);
    
    // ctype->params = (); // no parameters
    overcons->overlay = r_cons_macro;
    r_cons_macro->identifier = ident;
    r_cons_macro->arguments = macro_args;
    r_cons_macro->initialiser = r_comp;
    r_comp->members = cdecls;
    r_comp->statements = (pre, r_process_macro);
    r_process_macro->identifier = token;
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
    auto r_call = MakePatternNode<SeqArgsCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_event = MakePatternNode<Event>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetLoweredIdName() );                
    auto eexpr = MakePatternNode<TransformOfAgent, Expression>( &TypeOf::instance ); 
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
    auto eexpr = MakePatternNode<TransformOfAgent, Expression>( &TypeOf::instance ); 
    auto event = MakePatternNode<Event>();
    
    auto r_call = MakePatternNode<SeqArgsCall>();
    auto r_lookup = MakePatternNode<Lookup>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_notify->GetLoweredIdName() );                
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
 
    auto r_call = MakePatternNode<SeqArgsCall>();
    auto r_token = MakePatternNode< SpecificInstanceIdentifier >( s_delta_count->GetLoweredIdName() );                
    //MakePatternNode< Expression > eexpr; 
            
    r_call->callee = r_token;
    //r_call->arguments = (); // no operands
       
    Configure( SEARCH_REPLACE, s_delta_count, r_call );
}


AddIncludeSystemC::AddIncludeSystemC()
{
	string filename = "systemc.h";
	
    auto s_conjunction = MakePatternNode<ConjunctionAgent, Program>();
    auto s_program = MakePatternNode<Program>();
    auto r_program = MakePatternNode<Program>();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto s_stuff = MakePatternNode<StuffAgent, Program>();
    auto r_include = MakePatternNode<SystemInclude>();
    auto s_negation = MakePatternNode<NegationAgent, Declaration>();
    auto sx_include = MakePatternNode<SystemInclude>();

	s_conjunction->conjuncts = (s_program, s_stuff);
    s_program->members = (decls);
    decls->restriction = s_negation;
    s_stuff->terminus = MakePatternNode<SCNode>(); 
    s_negation->negand = sx_include;
    sx_include->filename = MakePatternNode<SpecificString>(filename);   
    r_program->members = (decls, r_include);
    r_include->filename = MakePatternNode<SpecificString>(filename);   

    Configure( COMPARE_REPLACE, s_conjunction, r_program );
}


AddIncludeSCExtensions::AddIncludeSCExtensions()
{
	string filename = "systemc_extensions.h";

    auto s_conjunction = MakePatternNode<ConjunctionAgent, Program>();
    auto s_program = MakePatternNode<Program>();
    auto r_program = MakePatternNode<Program>();
    auto decls = MakePatternNode<StarAgent, Declaration>();
    auto s_stuff = MakePatternNode<StuffAgent, Program>();
    auto r_include = MakePatternNode<LocalInclude>();
    auto s_negation = MakePatternNode<NegationAgent, Declaration>();
    auto sx_include = MakePatternNode<LocalInclude>();

	s_conjunction->conjuncts = (s_program, s_stuff);
    s_program->members = (decls);
    decls->restriction = s_negation;
    s_stuff->terminus = MakePatternNode<SCExtension>(); 
    s_negation->negand = sx_include;
    sx_include->filename = MakePatternNode<SpecificString>(filename);   
    r_program->members = (decls, r_include);
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
