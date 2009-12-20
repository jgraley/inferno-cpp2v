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
	set<SearchReplace::Coupling *> sms0;
	SearchReplace sr0;

	SharedPtr<Call> s_call( new Call );
      SharedPtr<TypeOf> s_callee( new TypeOf );
	  s_call->callee = s_callee;
	    SharedPtr<Procedure> s_proc( new Procedure );
	    s_callee->pattern = s_proc;
	      SharedPtr< Instance > s_param( new Instance );
	      s_proc->members.insert(s_param);
	        s_param->identifier = shared_new< InstanceIdentifier >();
	        s_param->type = shared_new< Type >();
	      SharedPtr< SearchReplace::Star<Instance> > s_other_params( new SearchReplace::Star<Instance> );
	      s_proc->members.insert(s_other_params);
	  SharedPtr< MapOperand > s_arg( new MapOperand );
	  s_call->operands.insert( s_arg );
	    s_arg->identifier = shared_new< InstanceIdentifier >();
	    SharedPtr<TypeOf> s_arg_value( new TypeOf );
	    s_arg->value = s_arg_value;
	      //s_arg_value->pattern = shared_new< Type >();
	      SharedPtr< NotMatch<Type> > s_arg_type( new NotMatch<Type> );
	      s_arg_value->pattern = s_arg_type;
	        s_arg_type->pattern = shared_new< Type >();
	  SharedPtr< SearchReplace::Star<MapOperand> > s_other_args( new SearchReplace::Star<MapOperand> );
	  s_call->operands.insert( s_other_args );

	SharedPtr<Call> r_call( new Call );
      SharedPtr< MapOperand > r_arg( new MapOperand );
	  r_call->operands.insert( r_arg );
		r_arg->identifier = shared_new< InstanceIdentifier >();
		SharedPtr<Cast> r_cast( new Cast );
		r_arg->value = r_cast;
		  r_cast->operand = shared_new< Expression >();
		  r_cast->type = shared_new< Type >();
	  SharedPtr< SearchReplace::Star<MapOperand> > r_other_args( new SearchReplace::Star<MapOperand> );
	  r_call->operands.insert( r_other_args );

	SearchReplace::Coupling ms_call((s_call, r_call));
	sms0.insert( &ms_call ); // note: alternatively we could just match the <x>_other_args

	SearchReplace::Coupling ms_ident((s_param->identifier, s_arg->identifier, r_arg->identifier));
	sms0.insert( &ms_ident );

	SearchReplace::Coupling ms_type((s_param->type, s_arg_type->pattern, r_cast->type));
	sms0.insert( &ms_type );

	SearchReplace::Coupling ms_value((s_arg->value, r_cast->operand));
	sms0.insert( &ms_value );

	SearchReplace::Coupling ms_other_args((s_other_args, r_other_args));
	sms0.insert( &ms_other_args );

	sr0.Configure(s_call, r_call, sms0);
	sr0( context, proot );
}
