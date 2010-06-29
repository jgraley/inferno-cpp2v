/*
 * generate_stacks.cpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#include "steps/for_to_while.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"

void ForToWhile::operator()( SharedPtr<Node> context, SharedPtr<Node> *proot )
{
    MakeShared<For> s_for;
    MakeShared<Statement> s_body, s_init, s_inc, r_forbody, r_init, r_inc;
    MakeShared<Expression> s_cond, r_cond;
    MakeShared<While> r_while;
    MakeShared<Compound> r_outer, r_body;
    
    s_for->body = s_body;
    s_for->initialisation = s_init;
    s_for->condition = s_cond;
    s_for->increment = s_inc;
    
    r_outer->statements = (r_init, r_while);
    r_while->body = r_body;
    r_while->condition = r_cond;
    r_body->statements = (r_forbody, r_inc);
    
	CouplingSet couplings((
		Coupling(( s_body, r_forbody )),
		Coupling(( s_init, r_init )),
		Coupling(( s_cond, r_cond )),
		Coupling(( s_inc, r_inc )) ));
    
   	SearchReplace( s_for, r_outer, couplings )( context, proot );
}
