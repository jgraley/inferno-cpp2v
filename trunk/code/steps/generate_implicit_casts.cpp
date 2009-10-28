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

GenerateImplicitCasts::GenerateImplicitCasts()
{
}

void GenerateImplicitCasts::operator()( shared_ptr<Program> program )
{
	set<SearchReplace::MatchSet *> sms0;
	SearchReplace sr0;

	shared_ptr<Call> s_call( new Call );
      shared_ptr<SoftExpressonOfType> s_callee( new SoftExpressonOfType );
	  s_call->callee = s_callee;
	    shared_ptr<Procedure> s_proc( new Procedure );
	    s_callee->type_pattern = s_proc;
	      shared_ptr< Instance > s_param( new Instance );
	      s_proc->members.insert(s_param);
	        s_param->identifier = shared_new< InstanceIdentifier >();
	        s_param->type = shared_new< Type >();
	      shared_ptr< SearchReplace::Star<Instance> > s_other_params( new SearchReplace::Star<Instance> );
	      s_proc->members.insert(s_other_params);
	  shared_ptr< MapOperand > s_arg( new MapOperand );
	  s_call->operands.insert( s_arg );
	    s_arg->identifier = shared_new< InstanceIdentifier >();
	    shared_ptr<SoftExpressonOfType> s_arg_value( new SoftExpressonOfType );
	    s_arg->value = s_arg_value;
	      //s_arg_value->type_pattern = shared_new< Type >();
	      shared_ptr< SoftNot<Type> > s_arg_type( new SoftNot<Type> );
	      s_arg_value->type_pattern = s_arg_type;
	        s_arg_type->pattern = shared_new< Type >();
	  shared_ptr< SearchReplace::Star<MapOperand> > s_other_args( new SearchReplace::Star<MapOperand> );
	  s_call->operands.insert( s_other_args );

	shared_ptr<Call> r_call( new Call );
      shared_ptr< MapOperand > r_arg( new MapOperand );
	  r_call->operands.insert( r_arg );
		r_arg->identifier = shared_new< InstanceIdentifier >();
		shared_ptr<Cast> r_cast( new Cast );
		r_arg->value = r_cast;
		  r_cast->operand = shared_new< Expression >();
		  r_cast->type = shared_new< Type >();
	  shared_ptr< SearchReplace::Star<MapOperand> > r_other_args( new SearchReplace::Star<MapOperand> );
	  r_call->operands.insert( r_other_args );

	SearchReplace::MatchSet ms_call;
	ms_call.insert( s_call );
	ms_call.insert( r_call );
	sms0.insert( &ms_call ); // note: alternatively we could just match the <x>_other_args

	SearchReplace::MatchSet ms_ident;
	ms_ident.insert( s_param->identifier );
	ms_ident.insert( s_arg->identifier );
	ms_ident.insert( r_arg->identifier );
	sms0.insert( &ms_ident );

	SearchReplace::MatchSet ms_type;
	ms_type.insert( s_param->type );
	ms_type.insert( s_arg_type->pattern );
	ms_type.insert( r_cast->type );
	sms0.insert( &ms_type );

	SearchReplace::MatchSet ms_value;
	ms_value.insert( s_arg->value );
	ms_value.insert( r_cast->operand );
	sms0.insert( &ms_value );

	SearchReplace::MatchSet ms_other_args;
	ms_other_args.insert( s_other_args );
	ms_other_args.insert( r_other_args );
	sms0.insert( &ms_other_args );

	sr0.Configure(s_call, r_call, sms0);
	sr0( program );
}
