
#include "systemc_from_c_simple.hpp"
#include "tree/cpptree.hpp"
#include "tree/typeof.hpp"
#include "inferno_agents.hpp"
#include "vn/agents/all.hpp"

// Uncomment to reproduce #807
#define ALSO_MOVE_VARS

using namespace CPPTree;
using namespace SCTree;
using namespace Steps;

struct GlobalsModule : Module { NODE_FUNCTIONS_FINAL };

GlobalScopeToModule::GlobalScopeToModule()
{
    auto delta = MakePatternNode< Delta<Scope> >();
	auto s_all = MakePatternNode< Conjunction<Scope> >();
	auto s_scope = MakePatternNode< Program >();
	auto s_decls = MakePatternNode< Star<Declaration> >();
	auto s_despin = MakePatternNode< Negation<Scope> >();
	auto sn_scope = MakePatternNode< Program >();
	auto sn_module = MakePatternNode< Module >();
	auto r_scope = MakePatternNode< Program >();
	auto r_gmodule = MakePatternNode< GlobalsModule >();
	auto r_gmodule_tid = MakePatternNode<BuildTypeIdentifierAgent>("GlobalScope");
	auto r_gmodule_inst = MakePatternNode< Static >();
	auto es_scope = MakePatternNode< Program >();
	auto es_decls = MakePatternNode< Star<Declaration> >();
	auto es_instance = MakePatternNode<Instance>();
	auto es_gmodule = MakePatternNode< GlobalsModule >();
	auto es_gmodule_name = MakePatternNode< TypeIdentifier >();
	auto es_gmodule_decls = MakePatternNode< Star<Declaration> >();
	auto er_gmodule = MakePatternNode< GlobalsModule >();
	auto er_scope = MakePatternNode< Program >();
	auto er_field = MakePatternNode<Field>();
	auto es_id = MakePatternNode<InstanceIdentifier>();
#ifdef ALSO_MOVE_VARS
	auto es_id_all = MakePatternNode<Conjunction<InstanceIdentifier>>();
	auto es_id_not = MakePatternNode<Negation<InstanceIdentifier>>();
#endif		
	delta->through = s_all;
	s_all->conjuncts = (s_scope, s_despin);
	s_scope->members = ( s_decls );
	
	s_despin->negand = sn_scope;
	sn_scope->members = (sn_module, MakePatternNode< Star<Declaration> >() );
	sn_module->bases = ( MakePatternNode< Star<Base> >() );
	sn_module->members = ( MakePatternNode< Star<Declaration> >() );
	
	r_scope->members = ( r_gmodule, r_gmodule_inst, s_decls );
	r_gmodule->identifier = r_gmodule_tid;
	r_gmodule_inst->type = r_gmodule_tid;
	r_gmodule_inst->identifier = MakePatternNode<BuildInstanceIdentifierAgent>("globals");
	r_gmodule_inst->initialiser = MakePatternNode<Uninitialised>();
	r_gmodule_inst->constancy = MakePatternNode<NonConst>();
	
	es_scope->members = ( es_gmodule, es_instance, es_decls );
	// Act on all Instance types, which includes functions, variable and 
	// constants but excludes user types (incl classes) and labels
#ifdef ALSO_MOVE_VARS
	es_instance->type = MakePatternNode<Type>(); 
#else
	es_instance->type = MakePatternNode<Callable>(); 
#endif
#ifdef ALSO_MOVE_VARS
	es_instance->identifier = es_id_all;
	es_id_all->conjuncts = ( es_id, es_id_not );
	es_id_not->negand = r_gmodule_inst->identifier;
#else
	es_instance->identifier = es_id;	
#endif
	es_instance->initialiser = MakePatternNode<Initialiser>();
	es_gmodule->identifier = es_gmodule_name;
	es_gmodule->members = ( es_gmodule_decls );
	er_scope->members = ( er_gmodule, es_decls );
	er_gmodule->identifier = es_gmodule_name;
	er_gmodule->members = ( es_gmodule_decls, er_field );
	er_field->type = es_instance->type;
	er_field->identifier = es_id;
	er_field->initialiser = es_instance->initialiser;
	er_field->virt = MakePatternNode<NonVirtual>();
	er_field->access = MakePatternNode<Public>();
	er_field->constancy = MakePatternNode<NonConst>();
	
	// Through, search, replace
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Scope> >( delta, es_scope, er_scope );
    delta->overlay = r_scope;
    Configure( COMPARE_REPLACE, delta, r_embedded );
}


MainToThread::MainToThread()
{
	auto e_stuff = MakePatternNode< Stuff<Initialiser> >();
	auto e_delta = MakePatternNode< Delta<Statement> >();
	auto es_return = MakePatternNode<Return>();
	auto er_cease = MakePatternNode<Cease>();
	
	e_stuff->terminus = e_delta;
	e_delta->through = es_return;
	es_return->return_value = MakePatternNode< Expression >();
	e_delta->overlay = er_cease;
	er_cease->code = es_return->return_value;
	
	auto gmodule = MakePatternNode< GlobalsModule >();
    auto delta = MakePatternNode< Delta<Instance> >();
	auto s_field = MakePatternNode< Field >();
    auto s_func = MakePatternNode<Function>();
    auto s_identifier = MakePatternNode<InstanceIdentifierByNameAgent>("main");
	auto r_field = MakePatternNode< Field >();
	
	gmodule->bases = ( MakePatternNode< Star<Base> >() );
	gmodule->members = ( delta, MakePatternNode< Star<Declaration> >() );	
	delta->through = s_field;
	s_field->identifier = s_identifier;
	s_field->type = s_func;
	s_field->initialiser = MakePatternNode< Initialiser >();
	//s_func->members = ()        require no parameters
	s_func->return_type = MakePatternNode<Integral>();
	delta->overlay = r_field;
	r_field->identifier = s_identifier;
	r_field->type = MakePatternNode<Thread>();
	r_field->virt = MakePatternNode<NonVirtual>();
	r_field->access = MakePatternNode<Public>();
	r_field->constancy = MakePatternNode<NonConst>();
	auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Initialiser> >( s_field->initialiser, e_stuff );
	r_field->initialiser = r_embedded;

    Configure( SEARCH_REPLACE, gmodule );
}


void SystemCFromCSimple::Build( vector< shared_ptr<VNStep> > *sequence )
{
    sequence->push_back( make_shared<GlobalScopeToModule>() );
    sequence->push_back( make_shared<MainToThread>() );
}
