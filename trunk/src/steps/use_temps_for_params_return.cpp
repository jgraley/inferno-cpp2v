/*
 * use_temps_for_params_return.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/use_temps_for_params_return.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"
#include "helpers/misc.hpp"


UseTempsForParamsReturn::UseTempsForParamsReturn()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
	TreePtr<Return> s_return( new Return );
	TreePtr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
	s_return->return_value = s_and;
    MakeTreePtr< TransformOf<Expression> > retval( &TypeOf::instance );
	MakeTreePtr<Type> type;
	retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    TreePtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> );
	s_and->patterns = ( retval, cs_stuff );
    MakeTreePtr< TransformOf<InstanceIdentifier> > cs_id( &GetDeclaration::instance );
    cs_stuff->terminus = cs_id;
    TreePtr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
	TreePtr<Compound> r_sub_comp( new Compound );
	TreePtr< Temporary > r_newvar( new Temporary );
	r_newvar->type = type;
	MakeTreePtr<BuildInstanceIdentifier> id("temp_retval");
	r_newvar->identifier = id;
	r_newvar->initialiser = MakeTreePtr<Uninitialised>();
	r_sub_comp->members = ( r_newvar );
	TreePtr<Assign> r_assign( new Assign );
	r_assign->operands.push_back( id );
	r_assign->operands.push_back( retval );
	r_sub_comp->statements.push_back( r_assign );
	TreePtr<Return> r_return( new Return );
	r_sub_comp->statements.push_back( r_return );
	r_return->return_value = id;
       
	SearchReplace::Configure( s_return, r_sub_comp );
}
