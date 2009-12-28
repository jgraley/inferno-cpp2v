/*
 * generate_stacks.cpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#include "steps/for_to_while.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "common/refcount.hpp"
#include "helpers/soft_patterns.hpp"

void ForToWhile::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
    SharedNew<For> s_for;
    SharedNew<Statement> s_body, s_init, s_inc, r_forbody, r_init, r_inc;
    SharedNew<Expression> s_cond, r_cond;
    SharedNew<While> r_while;
    SharedNew<Compound> r_outer, r_body;
    
    s_for->body = s_body;
    s_for->initialisation = s_init;
    s_for->condition = s_cond;
    s_for->increment = s_inc;
    
    r_outer->statements = (r_init, r_while);
    r_while->body = r_body;
    r_while->condition = r_cond;
    r_body->statements = (r_forbody, r_inc);
    
	SearchReplace::CouplingSet couplings((
		SearchReplace::Coupling(( s_body, r_forbody )), 
		SearchReplace::Coupling(( s_init, r_init )), 
		SearchReplace::Coupling(( s_cond, r_cond )), 
		SearchReplace::Coupling(( s_inc, r_inc )) ));		    
    
   	SearchReplace( s_for, r_outer, couplings )( context, proot );
}
