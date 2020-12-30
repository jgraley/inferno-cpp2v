/*
 * generate_implicit_casts.cpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#include "generate_implicit_casts.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/typeof.hpp"

using namespace CPPTree;
using namespace Steps;

GenerateImplicitCasts::GenerateImplicitCasts() 
{
    // Find a function call and use TypeOf to get the fucntion's declaration.
    // Select a single argument. Inforno does arguments via a map (as opposed
    // to ordering) and we couple the keys to an InstanceIdentifier widcard 
    // in order to ensure we are always talking about the same argument.
    //
    // Restrict the search to argument expressions whose type (TypeOf again)
    // does not match the type of the param in the declaration. Then we can 
    // simply insert a cast to the declaration param type at the root of the 
    // expression.
	MakePatternPtr<Call> s_call;
	MakePatternPtr< TransformOf<Expression> > callee( &TypeOf::instance );
	s_call->callee = callee;
	MakePatternPtr<CallableParams> s_proc;
	callee->pattern = s_proc;
	MakePatternPtr< Instance > s_param;
	MakePatternPtr< InstanceIdentifier > param_id;
	s_param->identifier = param_id;
	MakePatternPtr< Type > type;
	s_param->type = type;
	MakePatternPtr< Star<Instance> > s_other_params;
	s_proc->members = (s_param, s_other_params);
	MakePatternPtr< MapOperand > s_arg;
	s_arg->identifier = param_id;
	MakePatternPtr< TransformOf<Expression> > s_arg_value( &TypeOf::instance );
	s_arg->value = s_arg_value;
	//s_arg_value->pattern = MakePatternPtr< Type >();
	MakePatternPtr< Negation<Type> > s_arg_type;
	s_arg_value->pattern = s_arg_type;
	s_arg_type->pattern = type;
	MakePatternPtr< Star<MapOperand> > other_args;
	s_call->operands = ( s_arg, other_args );

	MakePatternPtr<Call> r_call;
	MakePatternPtr< MapOperand > r_arg;
	r_arg->identifier = param_id;
	MakePatternPtr<Cast> r_cast;
	r_arg->value = r_cast;
	r_cast->operand = s_arg->value;
	r_cast->type = type;
	r_call->operands = ( r_arg, other_args );
    r_call->callee = callee;
    
	Configure(s_call, r_call);
}
