/*
 * use_temps_for_params_return.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/use_temps_for_params_return.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "common/refcount.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"
#include "helpers/misc.hpp"


void UseTempsForParamsReturn::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	TRACE();

    // search for return statement in a compound (TODO don't think we need the outer compound)
	SharedPtr<Return> s_return( new Return );
	SharedPtr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
	s_return->return_value = s_and;
	SharedPtr<TypeOf> s_retval( new TypeOf );
	s_retval->pattern = SharedNew<Type>();
    
    // Restrict the search to returns that have an automatic variable under them
    SharedPtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> );
	s_and->patterns = ( s_retval, cs_stuff );
	SharedPtr< GetDeclaration > cs_id( new GetDeclaration );	
    cs_stuff->terminus = cs_id;
    SharedPtr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
	SharedPtr<Compound> r_sub_comp( new Compound );
	SharedPtr< Temporary > r_newvar( new Temporary );
	r_newvar->type = SharedNew<Type>();
	r_newvar->identifier = SharedPtr<InstanceIdentifier>( new SoftMakeIdentifier( "temp_retval" ) );
	r_newvar->initialiser = SharedNew<Uninitialised>();
	r_sub_comp->members = ( r_newvar );
	SharedPtr<Assign> r_assign( new Assign );
	r_assign->operands.push_back( SharedNew<InstanceIdentifier>() );
	r_assign->operands.push_back( SharedNew<Expression>() );
	r_sub_comp->statements.push_back( r_assign );
	SharedPtr<Return> r_return( new Return );
	r_sub_comp->statements.push_back( r_return );
	r_return->return_value = SharedNew<InstanceIdentifier>();
       
    // Make the new variable be of the required type, ie whatever the expression evaluates to   
	CouplingSet sms((
		Coupling((s_retval->pattern, r_newvar->type)),
		Coupling((s_retval, r_assign->operands[1])),
		Coupling((r_newvar->identifier, r_assign->operands[0], r_return->return_value)) ));
             
	SearchReplace( s_return, r_sub_comp, sms )( context, proot );
}
