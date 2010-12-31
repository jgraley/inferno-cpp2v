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

void GenerateImplicitCasts::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	MakeTreePtr<Call> s_call;
	MakeTreePtr<TypeOf> s_callee;
	s_call->callee = s_callee;
	MakeTreePtr<Procedure> s_proc;
	s_callee->pattern = s_proc;
	MakeTreePtr< Instance > s_param;
	MakeTreePtr< InstanceIdentifier > param_id;
	s_param->identifier = param_id;
	MakeTreePtr< Type > type;
	s_param->type = type;
	MakeTreePtr< Star<Instance> > s_other_params;
	s_proc->members = (s_param, s_other_params);
	MakeTreePtr< MapOperand > s_arg;
	s_arg->identifier = param_id;
	MakeTreePtr<TypeOf> s_arg_value;
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

	CouplingSet sms0((
	    Coupling(( s_call, r_call )) ));

	SearchReplace(s_call, r_call, sms0)( context, proot );
}
