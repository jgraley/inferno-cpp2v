/*
 * use_temps_for_params_return.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/use_temps_for_params_return.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "common/refcount.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"
#include "helpers/misc.hpp"


void UseTempsForParamsReturn::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	TRACE();
	set<SearchReplace::Coupling *> sms;

    // search for return statement in a compound (TODO don't think we need the outer compound)
	shared_ptr<Return> s_return( new Return );
	shared_ptr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
	s_return->return_value = s_and;
	shared_ptr<TypeOf> s_retval( new TypeOf );
	s_and->patterns.insert( s_retval );
	s_retval->pattern = shared_new<Type>();
    
    // Restrict the search to returns that have an automatic variable under them
    shared_ptr< SearchReplace::Stuff<Expression> > cs_stuff( new SearchReplace::Stuff<Expression> );
	s_and->patterns.insert( cs_stuff );
	shared_ptr< GetDeclaration > cs_id( new GetDeclaration );	
    cs_stuff->terminus = cs_id;
    shared_ptr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
	shared_ptr<Compound> r_sub_comp( new Compound );
	shared_ptr< Temporary > r_newvar( new Temporary );
	r_newvar->type = shared_new<Type>();
	r_newvar->identifier = shared_ptr<InstanceIdentifier>( new SoftMakeIdentifier( "temp_retval" ) );
	r_newvar->initialiser = shared_new<Uninitialised>();
	r_sub_comp->members.insert( r_newvar );
	shared_ptr<Assign> r_assign( new Assign );
	r_assign->operands.push_back( shared_new<InstanceIdentifier>() );
	r_assign->operands.push_back( shared_new<Expression>() );
	r_sub_comp->statements.push_back( r_assign );
	shared_ptr<Return> r_return( new Return );
	r_sub_comp->statements.push_back( r_return );
	r_return->return_value = shared_new<InstanceIdentifier>();
       
    SearchReplace::Coupling c4; // Make the new variable be of the required type, ie whatever the expression evaluates to
    c4.insert( s_retval->pattern );
    c4.insert( r_newvar->type );
    sms.insert( &c4 ); 

    SearchReplace::Coupling c5; // 
    c5.insert( s_retval );
    c5.insert( r_assign->operands[1] );
    sms.insert( &c5 ); 
       
    SearchReplace::Coupling c6;
    c6.insert( r_newvar->identifier );
    c6.insert( r_assign->operands[0] );
    c6.insert( r_return->return_value );
    sms.insert( &c6 ); 
             
	SearchReplace( s_return, r_sub_comp, sms )( context, proot );
}
