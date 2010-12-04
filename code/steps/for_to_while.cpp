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
    MakeTreePtr< Stuff<Statement> > ss_stuff, sr_stuff;
    MakeTreePtr< NotMatch<Statement> > ss_not;
    MakeTreePtr< Loop > ss_loop;
    
    MakeTreePtr<Continue> ss_cont, sr_cont;
    MakeTreePtr<Nop> sr_nop;

    sr_block->statements = (sr_inc, sr_cont);
    ss_gg->through = ss_cont;
    MakeTreePtr< RootedSlave<Statement> > r_slave( r_forbody, ss_stuff, sr_stuff );
    ss_stuff->terminus = ss_gg;
    ss_stuff->recurse_restriction = ss_not;
    sr_stuff->terminus = sr_block;
    ss_not->pattern = ss_loop;

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
		Coupling(( s_inc, r_inc, sr_inc )),
		Coupling(( ss_stuff, sr_stuff )) ));

   	SearchReplace( s_for, r_outer, couplings )( context, proot );
}


void WhileToDo::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
	MakeTreePtr<While> s_while;
    MakeTreePtr<Statement> s_body, r_body;
    MakeTreePtr<Expression> s_cond, r_if_cond, r_do_cond;
    MakeTreePtr<Nop> r_nop;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Do> r_do;

    s_while->body = s_body;
    s_while->condition = s_cond;

    r_if->condition = r_if_cond;
    r_if->body = r_do;
    r_if->else_body = r_nop;
    r_do->condition = r_do_cond;
    r_do->body = r_body;

    CouplingSet couplings((
		Coupling(( s_body, r_body )),
		Coupling(( s_cond, r_if_cond, r_do_cond )) ));

   	SearchReplace( s_while, r_if, couplings )( context, proot );
}

// Removing superfluous Compund blocks to clean up the code
void Cleanup::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) // LIMITAION: decls in body not allowed
{
	{ // {x;{a;b;c}y} -> {x;a;b;c;y}
		MakeTreePtr<Compound> s_inner, s_outer, r_comp;
		MakeTreePtr< Star<Statement> > s_pre, s_post, s_body, r_pre, r_post, r_body;
		MakeTreePtr< Star<Declaration> > s_inner_decls, s_outer_decls, r_inner_decls, r_outer_decls;

		s_inner->statements = s_body;
		s_inner->members = ( s_inner_decls );
		s_outer->statements = ( s_pre, s_inner, s_post );
		s_outer->members = ( s_outer_decls );
		r_comp->statements = ( s_pre, s_body, s_post );
		r_comp->members = ( s_inner_decls, s_outer_decls );

		CouplingSet couplings((
			Coupling(( s_body )),
			Coupling(( s_pre )),
			Coupling(( s_post )),
			Coupling(( s_inner_decls )),
			Coupling(( s_outer_decls )) ));

		SearchReplace( s_outer, r_comp, couplings )( context, proot );
	}
    { //{a} -> a TODO need to restrict parent node to Statement: For, If etc OK; Instance is NOT OK
        //         TODO OR maybe just fix renderer for that case
        MakeTreePtr<Compound> s_comp;
        MakeTreePtr< Statement > s_body, r_body;

        s_comp->statements = s_body;
        // Note: leaving s_comp empty meaning no decls allowed

        CouplingSet couplings;
        couplings.insert( Coupling(( s_body, r_body )) ); // TODO syntactic sugar for case of only one coupling

        SearchReplace( s_comp, r_body, couplings )( context, proot );
    }
    { // remove nop
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Nop> s_nop;
        MakeTreePtr< Star<Declaration> > s_decls, r_decls;
        MakeTreePtr< Star<Statement> > s_pre, r_pre;
        MakeTreePtr< Star<Statement> > s_post, r_post;

        s_comp->members = s_decls;
        s_comp->statements = (s_pre, s_nop, s_post);
        
        r_comp->members = r_decls;
        r_comp->statements = (r_pre, r_post);

        CouplingSet couplings(( Coupling(( s_comp, r_comp )),
                                Coupling(( s_decls, r_decls )),
                                Coupling(( s_pre, r_pre )),
                                Coupling(( s_post, r_post )) )); 

        SearchReplace( s_comp, r_comp, couplings )( context, proot );
    }
    { // remove duplicate (sucessive) labels 
        MakeTreePtr<Instance> s_instance, r_instance;
        MakeTreePtr< Stuff<Statement> > s_stuff, r_stuff, ss_stuff, sr_stuff;
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Label> s_label1, s_label2, r_label1; // keep l1 and elide l2
        MakeTreePtr< Star<Declaration> > s_decls, r_decls;
        MakeTreePtr< Star<Statement> > s_pre, r_pre;
        MakeTreePtr< Star<Statement> > s_post, r_post;
        MakeTreePtr<LabelIdentifier> s_labelid1, s_labelid2, ss_labelid1, ss_labelid2, sr_labelid;
        MakeTreePtr<BuildLabelIdentifier> r_labelid("MERGED");
        MakeTreePtr< MatchAny<LabelIdentifier> > ss_orrule;
        
        ss_stuff->terminus = ss_orrule;
        ss_orrule->patterns = (ss_labelid1, ss_labelid2);
        sr_stuff->terminus = sr_labelid;
        
        MakeTreePtr< RootedSlave<Statement> > r_slave( r_stuff, ss_stuff, sr_stuff );
        
        s_instance->initialiser = s_stuff;
        s_stuff->terminus = s_comp;
        s_comp->members = s_decls;
        s_comp->statements = (s_pre, s_label1, s_label2, s_post);
        s_label1->identifier = s_labelid1;
        s_label2->identifier = s_labelid2;
        
        r_instance->initialiser = r_slave;
        r_stuff->terminus = r_comp;
        r_comp->members = r_decls;
        r_comp->statements = (r_pre, r_label1, r_post);
        r_label1->identifier = r_labelid;
        
        CouplingSet couplings(( Coupling(( s_instance, r_instance )),
                                Coupling(( s_stuff, r_stuff )),
                                Coupling(( s_comp, r_comp )),
                                Coupling(( s_decls, r_decls )),
                                Coupling(( s_pre, r_pre )),
                                Coupling(( s_post, r_post )),
                                Coupling(( ss_stuff, sr_stuff )),
                                Coupling(( ss_labelid1, s_labelid1 )),
                                Coupling(( ss_labelid2, s_labelid2 )),
                                Coupling(( sr_labelid, r_labelid )) )); 

        SearchReplace( s_instance, r_instance, couplings )( context, proot );
    }
    { // remove goto X before X:
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Goto> s_goto;
        MakeTreePtr<Label> s_label, r_label;
        MakeTreePtr<LabelIdentifier> s_labelid, s_labelid_a, r_labelid;
        MakeTreePtr< Star<Declaration> > s_decls, r_decls;
        MakeTreePtr< Star<Statement> > s_pre, r_pre;
        MakeTreePtr< Star<Statement> > s_post, r_post;

        s_comp->members = s_decls;
        s_comp->statements = (s_pre, s_goto, s_label, s_post);
        s_goto->destination = s_labelid;
        s_label->identifier = s_labelid_a;
        
        r_comp->members = r_decls;
        r_comp->statements = (r_pre, r_label, r_post);
        r_label->identifier = r_labelid;

        CouplingSet couplings(( Coupling(( s_comp, r_comp )),
                                Coupling(( s_decls, r_decls )),
                                Coupling(( s_pre, r_pre )),
                                Coupling(( s_post, r_post )),
                                Coupling(( s_labelid, s_labelid_a, r_labelid)) )); 

        SearchReplace( s_comp, r_comp, couplings )( context, proot );
    }
    { // remove unused labels
        MakeTreePtr<Instance> s_instance, r_instance;
        MakeTreePtr< Stuff<Statement> > s_stuff, sx_stuff, r_stuff;
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Label> s_label; // keep l1 and elide l2
        MakeTreePtr< Star<Declaration> > s_decls, r_decls;
        MakeTreePtr< Star<Statement> > s_pre, r_pre;
        MakeTreePtr< Star<Statement> > s_post, r_post;
        MakeTreePtr<LabelIdentifier> s_labelid, sx_labelid;
        MakeTreePtr<Goto> sx_goto;
        MakeTreePtr< MatchAll<Statement> > s_andrule;
        MakeTreePtr< NotMatch<Statement> > sx_notrule;
        MakeTreePtr< NotMatch<Node> > sxx_notrule;        
        MakeTreePtr< Label > sxx_label;        
        
        s_instance->initialiser = s_andrule;
        s_andrule->patterns = (s_stuff, sx_notrule);
        s_stuff->terminus = s_comp;
        s_comp->members = s_decls;
        s_comp->statements = (s_pre, s_label, s_post);
        s_label->identifier = s_labelid;
                        
        sx_notrule->pattern = sx_stuff;
        sx_stuff->recurse_restriction = sxx_notrule;
        sxx_notrule->pattern = sxx_label;
        sx_stuff->terminus = sx_labelid;
        
        r_instance->initialiser = r_stuff;
        r_stuff->terminus = r_comp;
        r_comp->members = r_decls;
        r_comp->statements = (r_pre, r_post);

        CouplingSet couplings(( Coupling(( s_instance, r_instance )),
                                Coupling(( s_stuff, r_stuff )),
                                Coupling(( s_comp, r_comp )),
                                Coupling(( s_decls, r_decls )),
                                Coupling(( s_labelid, sx_labelid )),
                                Coupling(( s_pre, r_pre )),
                                Coupling(( s_post, r_post )) )); 

        SearchReplace( s_instance, r_instance, couplings )( context, proot );
    }
}

