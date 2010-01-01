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

	SharedNew<Call> s_call;
	SharedNew<TypeOf> s_callee;
	s_call->callee = s_callee;
	SharedNew<Procedure> s_proc;
	s_callee->pattern = s_proc;
	SharedNew< Instance > s_param;
	s_param->identifier = SharedNew< InstanceIdentifier >();
	s_param->type = SharedNew< Type >();
	SharedNew< Star<Instance> > s_other_params;
	s_proc->members = (s_param, s_other_params);
	SharedNew< MapOperand > s_arg;
	s_arg->identifier = SharedNew< InstanceIdentifier >();
	SharedNew<TypeOf> s_arg_value;
	s_arg->value = s_arg_value;
	//s_arg_value->pattern = SharedNew< Type >();
	SharedNew< NotMatch<Type> > s_arg_type;
	s_arg_value->pattern = s_arg_type;
	s_arg_type->pattern = SharedNew< Type >();
	SharedNew< Star<MapOperand> > s_other_args;
	s_call->operands = ( s_arg, s_other_args );

	SharedNew<Call> r_call;
	SharedNew< MapOperand > r_arg;
	r_arg->identifier = SharedNew< InstanceIdentifier >();
	SharedNew<Cast> r_cast;
	r_arg->value = r_cast;
	r_cast->operand = SharedNew< Expression >();
	r_cast->type = SharedNew< Type >();
	SharedNew< Star<MapOperand> > r_other_args;
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
