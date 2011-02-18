/*
 * generate_implicit_casts.cpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#include "generate_implicit_casts.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"

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
	MakeTreePtr<Call> s_call;
	MakeTreePtr< TransformOf<Expression> > callee( &TypeOf::instance );
	s_call->callee = callee;
	MakeTreePtr<Procedure> s_proc;
	callee->pattern = s_proc;
	MakeTreePtr< Instance > s_param;
	MakeTreePtr< InstanceIdentifier > param_id;
	s_param->identifier = param_id;
	MakeTreePtr< Type > type;
	s_param->type = type;
	MakeTreePtr< Star<Instance> > s_other_params;
	s_proc->members = (s_param, s_other_params);
	MakeTreePtr< MapOperand > s_arg;
	s_arg->identifier = param_id;
	MakeTreePtr< TransformOf<Expression> > s_arg_value( &TypeOf::instance );
	s_arg->value = s_arg_value;
	//s_arg_value->pattern = MakeTreePtr< Type >();
	MakeTreePtr< NotMatch<Type> > s_arg_type;
	s_arg_value->pattern = s_arg_type;
	s_arg_type->pattern = type;
	MakeTreePtr< Star<MapOperand> > other_args;
	s_call->operands = ( s_arg, other_args );

	MakeTreePtr<Call> r_call;
	MakeTreePtr< MapOperand > r_arg;
	r_arg->identifier = param_id;
	MakeTreePtr<Cast> r_cast;
	r_arg->value = r_cast;
	r_cast->operand = s_arg->value;
	r_cast->type = type;
	r_call->operands = ( r_arg, other_args );
    r_call->callee = callee;
    
	SearchReplace::Configure(s_call, r_call);
}
