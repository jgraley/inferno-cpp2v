/*
 * generate_implicit_casts.cpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#include "generate_implicit_casts.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "common/refcount.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"

void GenerateImplicitCasts::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	SearchReplace sr0;

	MakeShared<Call> s_call;
	MakeShared<TypeOf> s_callee;
	s_call->callee = s_callee;
	MakeShared<Procedure> s_proc;
	s_callee->pattern = s_proc;
	MakeShared< Instance > s_param;
	s_param->identifier = MakeShared< InstanceIdentifier >();
	s_param->type = MakeShared< Type >();
	MakeShared< Star<Instance> > s_other_params;
	s_proc->members = (s_param, s_other_params);
	MakeShared< MapOperand > s_arg;
	s_arg->identifier = MakeShared< InstanceIdentifier >();
	MakeShared<TypeOf> s_arg_value;
	s_arg->value = s_arg_value;
	//s_arg_value->pattern = MakeShared< Type >();
	MakeShared< NotMatch<Type> > s_arg_type;
	s_arg_value->pattern = s_arg_type;
	s_arg_type->pattern = MakeShared< Type >();
	MakeShared< Star<MapOperand> > s_other_args;
	s_call->operands = ( s_arg, s_other_args );

	MakeShared<Call> r_call;
	MakeShared< MapOperand > r_arg;
	r_arg->identifier = MakeShared< InstanceIdentifier >();
	MakeShared<Cast> r_cast;
	r_arg->value = r_cast;
	r_cast->operand = MakeShared< Expression >();
	r_cast->type = MakeShared< Type >();
	MakeShared< Star<MapOperand> > r_other_args;
	r_call->operands = ( r_arg, r_other_args );

	CouplingSet sms0((
	    Coupling(( s_call, r_call )), // note: alternatively we could just match the <x>_other_args
        Coupling(( s_param->identifier, s_arg->identifier, r_arg->identifier )),
        Coupling(( s_param->type, s_arg_type->pattern, r_cast->type )),
        Coupling(( s_arg->value, r_cast->operand )),
        Coupling(( s_other_args, r_other_args )) ));

	sr0.Configure(s_call, r_call, sms0);
	sr0( context, proot );
}
