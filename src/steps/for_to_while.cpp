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
    MakeTreePtr<Compound> r_outer, r_body, l_r_block;
    MakeTreePtr< GreenGrass<Statement> > l_s_gg;
    MakeTreePtr< Stuff<Statement> > l_stuff;
    MakeTreePtr< Overlay<Statement> > l_overlay;
    MakeTreePtr< NotMatch<Statement> > l_s_not;
    MakeTreePtr< Loop > l_s_loop;
    
    MakeTreePtr<Continue> l_s_cont, l_r_cont;
    MakeTreePtr<Nop> l_r_nop;

    l_r_block->statements = (inc, l_r_cont);
    l_s_gg->through = l_s_cont;
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave( forbody, l_stuff, l_stuff );
    l_stuff->terminus = l_overlay;
    l_overlay->through = l_s_gg;
    l_stuff->recurse_restriction = l_s_not;
    l_overlay->overlay = l_r_block;
    l_s_not->pattern = l_s_loop;

    s_for->body = forbody;
    s_for->initialisation = init;
    s_for->condition = cond;
    s_for->increment = inc;

    r_outer->statements = (init, r_while);
    r_while->body = r_body;
    r_while->condition = cond;
    r_body->statements = (r_slave, inc);

   	SearchReplace( s_for, r_outer )( context, proot );
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
