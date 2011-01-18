/*
 * clean_up.cpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#include "steps/clean_up.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"

// Removing superfluous Compund blocks to clean up the code
void CleanupCompoundMulti::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) // LIMITAION: decls in body not allowed
{
    { // {x;{a;b;c}y} -> {x;a;b;c;y}
        MakeTreePtr<Compound> s_inner, s_outer, r_comp;
        MakeTreePtr< Star<Statement> > s_pre, s_post, s_body;
        MakeTreePtr< Star<Declaration> > s_inner_decls, s_outer_decls;

        s_inner->statements = ( s_body );
        s_inner->members = ( s_inner_decls );
        s_outer->statements = ( s_pre, s_inner, s_post );
        s_outer->members = ( s_outer_decls );
        r_comp->statements = ( s_pre, s_body, s_post );
        r_comp->members = ( s_inner_decls, s_outer_decls );

        SearchReplace( s_outer, r_comp )( context, proot );
    }
}


void CleanupCompoundSingle::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) 
{
    { //{a} -> a TODO need to restrict parent node to Statement: For, If etc OK; Instance is NOT OK
      //         TODO OR maybe just fix renderer for that case
	  // Note: this hits eg If(x){a;} which the "Multi" version misses 
        MakeTreePtr<Compound> s_comp;
        MakeTreePtr< Statement > body;

        s_comp->statements = body;
        // Note: leaving s_comp empty meaning no decls allowed

        SearchReplace( s_comp, body )( context, proot );
    }
}


void CleanupNop::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) 
{
    { // remove nop
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Nop> s_nop;
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< Star<Statement> > pre, post;

        s_comp->members = decls;
        s_comp->statements = (pre, s_nop, post);
        
        r_comp->members = decls;
        r_comp->statements = (pre, post);

        SearchReplace( s_comp, r_comp )( context, proot );
    }
}

void CleanupDuplicateLabels::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) 
{
    { // remove duplicate (sucessive) labels 
        MakeTreePtr<Instance> s_instance, r_instance;
        MakeTreePtr< Stuff<Statement> > stuff;
        MakeTreePtr< Overlay<Statement> > overlay;
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Label> s_label1, s_label2, r_label1; // keep l1 and elide l2
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< Star<Statement> > pre, post;
        MakeTreePtr<LabelIdentifier> s_labelid1, s_labelid2;
        MakeTreePtr<BuildLabelIdentifier> r_labelid("MERGED");
        MakeTreePtr< MatchAny<LabelIdentifier> > l_s_orrule;
        MakeTreePtr<InstanceIdentifier> identifier;
        MakeTreePtr<Subroutine> type;
        
        l_s_orrule->patterns = (s_labelid1, s_labelid2);
        
        MakeTreePtr< SlaveSearchReplace<Statement> > r_slave( stuff, l_s_orrule, r_labelid );
        
        s_instance->initialiser = stuff;
        s_instance->identifier = identifier;
        s_instance->type = type;
        s_comp->members = decls;
        s_comp->statements = (pre, s_label1, s_label2, post);
        s_label1->identifier = s_labelid1;
        s_label2->identifier = s_labelid2;
        
        r_instance->initialiser = r_slave;
        r_instance->identifier = identifier;
        r_instance->type = type;
        stuff->terminus = overlay;           
        overlay->through = s_comp;
        overlay->overlay = r_comp;
        r_comp->members = decls;
        r_comp->statements = (pre, r_label1, post);
        r_label1->identifier = r_labelid;
        
        SearchReplace( s_instance, r_instance )( context, proot );
    }
}

void CleanupIneffectualGoto::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) 
{
    { // remove goto X before X:
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Goto> s_goto;
        MakeTreePtr<Label> s_label, r_label;
        MakeTreePtr<LabelIdentifier> labelid;
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< Star<Statement> > pre, post;

        s_comp->members = decls;
        s_comp->statements = (pre, s_goto, s_label, post);
        s_goto->destination = labelid;
        s_label->identifier = labelid;
        
        r_comp->members = decls;
        r_comp->statements = (pre, r_label, post);
        r_label->identifier = labelid;

        SearchReplace( s_comp, r_comp )( context, proot );
    }
}

void CleanupUnusedLabels::operator()( TreePtr<Node> context, TreePtr<Node> *proot ) 
{
    { // remove unused labels
        MakeTreePtr<Instance> s_instance, r_instance;
        MakeTreePtr< Stuff<Statement> > stuff, sx_stuff;
        MakeTreePtr< Overlay<Statement> > overlay;
        MakeTreePtr<Compound> s_comp, r_comp;
        MakeTreePtr<Label> s_label; // keep l1 and elide l2
        MakeTreePtr< Star<Declaration> > decls;
        MakeTreePtr< Star<Statement> > pre, post;
        MakeTreePtr<LabelIdentifier> labelid;
        MakeTreePtr<Goto> sx_goto;
        MakeTreePtr< MatchAll<Statement> > s_andrule;
        MakeTreePtr< NotMatch<Statement> > sx_notrule;
        MakeTreePtr< NotMatch<Node> > sxx_notrule;        
        MakeTreePtr< Label > sxx_label;        
        MakeTreePtr<InstanceIdentifier> identifier;
        MakeTreePtr<Subroutine> type;

        s_instance->initialiser = s_andrule;
        s_instance->identifier = identifier;
        s_instance->type = type;
        s_andrule->patterns = (stuff, sx_notrule);
        s_comp->members = decls;
        s_comp->statements = (pre, s_label, post);
        s_label->identifier = labelid;
                        
        sx_notrule->pattern = sx_stuff;
        sx_stuff->recurse_restriction = sxx_notrule;
        sxx_notrule->pattern = sxx_label;
        sx_stuff->terminus = labelid;
        
        r_instance->initialiser = stuff;
        r_instance->identifier = identifier;
        r_instance->type = type;
        stuff->terminus = overlay;
        overlay->through = s_comp;
        overlay->overlay = r_comp;
        r_comp->members = decls;
        r_comp->statements = (pre, post);
        
        SearchReplace( s_instance, r_instance )( context, proot );
    }
}

