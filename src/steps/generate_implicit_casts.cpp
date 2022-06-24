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
	auto s_call = MakePatternPtr<Call>();
	auto callee = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
	s_call->callee = callee;
	auto s_proc = MakePatternPtr<CallableParams>();
	callee->pattern = s_proc;
	auto s_param = MakePatternPtr< Instance >();
	auto param_id = MakePatternPtr< InstanceIdentifier >();
	s_param->identifier = param_id;
	auto type = MakePatternPtr< Type >();
	s_param->type = type;
	auto s_other_params = MakePatternPtr< Star<Instance> >();
	s_proc->members = (s_param, s_other_params);
	auto s_arg = MakePatternPtr< MapOperand >();
	s_arg->identifier = param_id;
	auto s_arg_value = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
	s_arg->value = s_arg_value;
	//s_arg_value->pattern = MakePatternPtr< Type >();
	auto s_arg_type = MakePatternPtr< Negation<Type> >();
	s_arg_value->pattern = s_arg_type;
	s_arg_type->negand = type;
	auto other_args = MakePatternPtr< Star<MapOperand> >();
	s_call->operands = ( s_arg, other_args );

	auto r_call = MakePatternPtr<Call>();
	auto r_arg = MakePatternPtr< MapOperand >();
	r_arg->identifier = param_id;
	auto r_cast = MakePatternPtr<Cast>();
	r_arg->value = r_cast;
	r_cast->operand = s_arg->value;
	r_cast->type = type;
	r_call->operands = ( r_arg, other_args );
    r_call->callee = callee;
    
	Configure(SEARCH_REPLACE, s_call, r_call);
}
