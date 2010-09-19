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

void ForToWhile::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	MakeTreePtr<For> s_for;
    MakeTreePtr<Statement> s_body, s_init, s_inc, r_forbody, r_init, r_inc, sr_inc;
    MakeTreePtr<Expression> s_cond, r_cond;
    MakeTreePtr<While> r_while;
    MakeTreePtr<Compound> r_outer, r_body, sr_block;
    MakeTreePtr< GreenGrass<Statement> > ss_gg;
    
    MakeTreePtr<Continue> ss_cont, sr_cont;
    MakeTreePtr<Nop> sr_nop;

    sr_block->statements = (sr_inc, sr_cont);
    ss_gg->through = ss_cont;
    MakeTreePtr< Slave<Statement> > r_slave( r_forbody, ss_gg, sr_block );

    s_for->body = s_body;
    s_for->initialisation = s_init;
    s_for->condition = s_cond;
    s_for->increment = s_inc;

    r_outer->statements = (r_init, r_while);
    r_while->body = r_body;
    r_while->condition = r_cond;
    r_body->statements = (r_slave, r_inc);


    CouplingSet couplings((
		Coupling(( s_body, r_forbody )),
		Coupling(( s_init, r_init )),
		Coupling(( s_cond, r_cond )),
		Coupling(( s_inc, r_inc, sr_inc )) ));

   	SearchReplace( s_for, r_outer, couplings )( context, proot );
}
