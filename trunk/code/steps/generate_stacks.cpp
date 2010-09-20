/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"

#define HANDLE_EARLY_RETURNS 1


struct ThisOne : SoftMakeIdentifier
{
	NODE_FUNCTIONS
	ThisOne( string s ) :
		SoftMakeIdentifier(s){}
};

void GenerateStacks::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	TRACE();
	MakeTreePtr<Instance> s_fi, r_fi;
	MakeTreePtr<Subroutine> s_func;
	MakeTreePtr< MatchAll<Initialiser> > s_and;
	MakeTreePtr<Compound> s_top_comp, r_top_comp, r_ret_comp;
	MakeTreePtr< Star<Declaration> > s_top_decls, r_top_decls;
	MakeTreePtr< Star<Statement> > s_top_pre, r_top_pre;
	MakeTreePtr< Stuff<Statement> > cs_stuff, s_stuff, r_stuff;
	MakeTreePtr<Automatic> cs_instance, s_instance;
	MakeTreePtr<Static> r_index, r_instance; // TODO Field
	MakeTreePtr<Unsigned> r_index_type;
	MakeTreePtr<PostIncrement> r_inc;
	MakeTreePtr<PostDecrement> r_dec, r_ret_dec;
	MakeTreePtr<InstanceIdentifier> s_identifier;
	MakeTreePtr<Array> r_array;
	MakeTreePtr<Return> s_return, r_return;
	MakeTreePtr<Expression> ss_identifier;
	MakeTreePtr<Subscript> sr_sub;
	MakeTreePtr< MatchAll<Node> > s_and3;
	MakeTreePtr<SoftMakeIdentifier> r_index_identifier("%s_stack_index");
	MakeTreePtr<ThisOne> r_identifier("%s_stack");
    MakeTreePtr< GreenGrass<Statement> > s_gg;

    // Master search - look for functions satisfying the construct limitation and get
	s_fi->identifier = MakeTreePtr<InstanceIdentifier>();
	s_fi->type = s_func;
	s_fi->initialiser = s_and;
	s_top_comp->members = ( s_top_decls );
	s_top_comp->statements = ( s_top_pre );

	// Construct limitation - restrict master search to functions that contain an automatic variable
	cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

	// Master replace - insert index variable, inc and dec into function at top level
	MakeTreePtr< Slave<Statement> > r_slave( r_stuff, ss_identifier, sr_sub );
	MakeTreePtr< RootedSlave<Statement> > r_mid( r_top_comp, s_stuff, r_slave );

#if HANDLE_EARLY_RETURNS
	MakeTreePtr< Slave<Statement> > r_slave3( r_mid, s_gg, r_ret_comp );
	r_fi->initialiser = r_slave3;
#else
	r_fi->initialiser = r_mid;
#endif
	// top-level decls
	r_top_comp->members = ( r_top_decls, r_index );
	r_index->type = r_index_type;
	r_index_type->width = MakeTreePtr<SpecificInteger>(32);
	r_index_identifier->source = MakeTreePtr<Identifier>();
	r_index->identifier = r_index_identifier;
	r_index->constancy = MakeTreePtr<NonConst>();
	r_index->initialiser = MakeTreePtr<SpecificInteger>(0);
	// top-level statements
	r_inc->operands = ( MakeTreePtr<InstanceIdentifier>() );
	r_top_comp->statements = ( r_inc, r_top_pre, r_dec );
	r_dec->operands = ( MakeTreePtr<InstanceIdentifier>() );

    // Slave search to find automatic variables within the function
	s_stuff->terminus = s_instance;
	s_instance->identifier = s_identifier;
	s_instance->initialiser = MakeTreePtr<Uninitialised>(); // can't handle initialisers!
	s_instance->type = MakeTreePtr<Type>();

    // Slave replace to insert as a static array (TODO be a member of enclosing class)
	r_instance->constancy = MakeTreePtr<NonConst>();
	r_instance->initialiser = MakeTreePtr<Uninitialised>();
	r_stuff->terminus = r_instance;
	r_identifier->source = MakeTreePtr<Identifier>();
	r_instance->identifier = r_identifier;
	r_instance->type = r_array;
	r_array->element = MakeTreePtr<Type>();
	r_array->size = MakeTreePtr<SpecificInteger>(10);

	// Sub-slave replace with a subscript into the array
	sr_sub->operands = ( MakeTreePtr<InstanceIdentifier>(), MakeTreePtr<InstanceIdentifier>() );

#if HANDLE_EARLY_RETURNS
	// Slave to find early returns in the function
	s_gg->through = s_return;

	// Slave replace with a decrement of the stack index coming before the return
	//r_ret_comp->members = ( r_ret_decls );
	r_ret_dec->operands = ( MakeTreePtr<InstanceIdentifier>() );
	r_ret_comp->statements = ( r_ret_dec, r_return );
#endif

	CouplingSet sms((
		Coupling(( s_top_decls, r_top_decls )), // Couple pre-existing decls in the function's top level
		Coupling(( s_top_pre, r_top_pre )), // Couple pre-existing statements in the funciton's top level
		Coupling(( s_stuff, r_stuff )), // Couple stuff between the function and the variable to be changed
		Coupling(( s_fi, r_fi )), // Couple the original function
		Coupling(( s_instance->type, r_array->element )), // Couple the type of the auto variable into the element type of the array
		Coupling(( s_identifier, r_identifier->source, ss_identifier )), // Couple the identifier of the auto variable for sub-slave and as source for array's name
		Coupling(( r_identifier, sr_sub->operands[0] )), // Couple the name of the array into the base of the subscript

#if HANDLE_EARLY_RETURNS
		Coupling(( s_fi->identifier, r_index_identifier->source )),
		Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->operands[1], r_ret_dec->operands[0] )),
		Coupling(( s_return, r_return )) )); // Couple the return statement
                                                             // make sure the ns and s are talking about the same return if there's more than one
#else
    	Coupling(( s_fi->identifier, r_index_identifier->source, s_fi2->identifier )),
        Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->index )) )); // Couple the name of the index variable into the index of the subscript and all the incs and decs
#endif

	SearchReplace( s_fi, r_fi, sms )( context, proot );
}
