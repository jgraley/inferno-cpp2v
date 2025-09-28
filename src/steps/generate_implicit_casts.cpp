/*
 * generate_implicit_casts.cpp
 *
 *  Created on: 20 Oct 2009
 *      Author: jgraley
 */

#include "generate_implicit_casts.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "vn/agents/all.hpp"
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
    auto s_call = MakePatternNode<Call>();
    auto callee = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    s_call->callee = callee;
    auto s_proc = MakePatternNode<CallableParams>();
    callee->pattern = s_proc;
    auto s_param = MakePatternNode< Parameter >();
    auto param_id = MakePatternNode< InstanceIdentifier >();
    s_param->identifier = param_id;
    auto type = MakePatternNode< Type >();
    s_param->type = type;
    auto s_other_params = MakePatternNode< Star<Parameter> >();
    s_proc->params = (s_param, s_other_params);
    auto s_arg = MakePatternNode< MapOperand >();
    s_arg->key = param_id;
    auto s_arg_value = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    s_arg->value = s_arg_value;
    //s_arg_value->pattern = MakePatternNode< Type >();
    auto s_arg_type = MakePatternNode< Negation<Type> >();
    s_arg_value->pattern = s_arg_type;
    s_arg_type->negand = type;
    auto other_args = MakePatternNode< Star<MapOperand> >();
    s_call->operands = ( s_arg, other_args );

    auto r_call = MakePatternNode<Call>();
    auto r_arg = MakePatternNode< MapOperand >();
    r_arg->key = param_id;
    auto r_cast = MakePatternNode<Cast>();
    r_arg->value = r_cast;
    r_cast->operand = s_arg->value;
    r_cast->type = type;
    r_call->operands = ( r_arg, other_args );
    r_call->callee = callee;
    
    Configure(SEARCH_REPLACE, s_call, r_call);
}
