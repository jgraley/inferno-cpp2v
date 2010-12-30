/*
 * for_to_while.cpp
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
    MakeTreePtr<Statement> forbody, inc, init;
    MakeTreePtr<Expression> cond;
    MakeTreePtr<While> r_while;
    MakeTreePtr<Compound> r_outer, r_body, sr_block;
    MakeTreePtr< GreenGrass<Statement> > ss_gg;
    MakeTreePtr< Stuff<Statement> > ss_stuff, sr_stuff;
    MakeTreePtr< NotMatch<Statement> > ss_not;
    MakeTreePtr< Loop > ss_loop;
    
    MakeTreePtr<Continue> ss_cont, sr_cont;
    MakeTreePtr<Nop> sr_nop;

    sr_block->statements = (inc, sr_cont);
    ss_gg->through = ss_cont;
    MakeTreePtr< RootedSlave<Statement> > r_slave( forbody, ss_stuff, sr_stuff );
    ss_stuff->terminus = ss_gg;
    ss_stuff->recurse_restriction = ss_not;
    sr_stuff->terminus = sr_block;
    ss_not->pattern = ss_loop;

    s_for->body = forbody;
    s_for->initialisation = init;
    s_for->condition = cond;
    s_for->increment = inc;

    r_outer->statements = (init, r_while);
    r_while->body = r_body;
    r_while->condition = cond;
    r_body->statements = (r_slave, inc);

    CouplingSet couplings((
		Coupling(( ss_stuff, sr_stuff )) ));

   	SearchReplace( s_for, r_outer, couplings )( context, proot );
}


void WhileToDo::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	MakeTreePtr<While> s_while;
    MakeTreePtr<Statement> body;
    MakeTreePtr<Expression> cond;
    MakeTreePtr<Nop> r_nop;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Do> r_do;

    s_while->body = body;
    s_while->condition = cond;

    r_if->condition = cond;
    r_if->body = r_do;
    r_if->else_body = r_nop;
    r_do->condition = cond;
    r_do->body = body;
	
   	SearchReplace( s_while, r_if )( context, proot );
}
