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
	shared_ptr<Compound> s_comp( new Compound );
	shared_ptr< SearchReplace::Star<Declaration> > s_decls( new SearchReplace::Star<Declaration> );
	s_comp->members.insert( s_decls );	
	shared_ptr< SearchReplace::Star<Statement> > s_pre( new SearchReplace::Star<Statement> );
	s_comp->statements.push_back( s_pre );
	shared_ptr<Return> s_return( new Return );
	s_comp->statements.push_back( s_return );	
	shared_ptr< SoftAnd<Expression> > s_and( new SoftAnd<Expression> );
	s_return->return_value = s_and;
	shared_ptr<SoftExpressonOfType> s_retval( new SoftExpressonOfType );
	s_and->patterns.insert( s_retval );
	s_retval->pattern = shared_new<Type>();
	shared_ptr< SearchReplace::Star<Statement> > s_post( new SearchReplace::Star<Statement> );
	s_comp->statements.push_back( s_post );
    
    // Restrict the search to returns that have an automaitc variable under them
    shared_ptr< SearchReplace::Stuff<Expression> > cs_stuff( new SearchReplace::Stuff<Expression> );
	s_and->patterns.insert( cs_stuff );
	shared_ptr< SoftIdentifierOfInstance > cs_id( new SoftIdentifierOfInstance );	
    cs_stuff->terminus = cs_id;
    shared_ptr<Instance> cs_instance( new Instance );
    cs_id->pattern = cs_instance;
    cs_instance->storage = shared_new<Auto>();
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
	shared_ptr<Compound> r_comp( new Compound );
	shared_ptr< SearchReplace::Star<Declaration> > r_decls( new SearchReplace::Star<Declaration> );
	r_comp->members.insert( r_decls );	
	shared_ptr< SearchReplace::Star<Statement> > r_pre( new SearchReplace::Star<Statement> );
	r_comp->statements.push_back( r_pre );
	shared_ptr<Compound> r_sub_comp( new Compound );
	r_comp->statements.push_back( r_sub_comp );
	shared_ptr< Instance > r_newvar( new Instance );
	r_newvar->storage = shared_new<Temp>();
	r_newvar->type = shared_new<Type>();
	r_newvar->constancy = shared_new<NonConst>();
	r_newvar->access = shared_new<Private>();
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
	shared_ptr< SearchReplace::Star<Statement> > r_post( new SearchReplace::Star<Statement> );
	r_comp->statements.push_back( r_post );

    SearchReplace::Coupling c1;
    c1.insert( s_decls );
    c1.insert( r_decls );
    sms.insert( &c1 ); 
       
    SearchReplace::Coupling c2;
    c2.insert( s_pre );
    c2.insert( r_pre );
    sms.insert( &c2 ); 
       
    SearchReplace::Coupling c3;
    c3.insert( s_post );
    c3.insert( r_post );
    sms.insert( &c3 ); 
       
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
             
	SearchReplace( s_comp, r_comp, sms )( context, proot );
}
