
#include "steps/lower_control_flow.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"

void IfToIfGoto::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
    MakeTreePtr< MatchAll<Statement> > s_and;    
    MakeTreePtr<If> s_if, sr_if, r_if;
    MakeTreePtr<Statement> s_body, s_else_body, r_body, r_else_body;
    MakeTreePtr<Expression> s_cond, r_cond;
    MakeTreePtr< NotMatch<Statement> > sr_not;
    MakeTreePtr<Goto> sr_goto, r_goto, r_goto_else;
    MakeTreePtr<Nop> sr_nop, r_nop;
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<LogicalNot> r_not;
    MakeTreePtr<SoftMakeLabelIdentifier> r_labelid1("BYPASS_THEN"), r_labelid2("BYPASS_ELSE");
    MakeTreePtr<LabelIdentifier> r_labelid1a, r_labelid2a;
    MakeTreePtr<Label> r_label1, r_label2;
    
    s_and->patterns = (s_if, sr_not);
    s_if->condition = s_cond;
    s_if->body = s_body;
    s_if->else_body = s_else_body;
    
    // exclude if(x) goto y;
    sr_not->pattern = sr_if;
    sr_if->body = sr_goto;
    sr_if->else_body = sr_nop;
    
    r_comp->statements = (r_if, r_body, r_goto_else, r_label1, r_else_body, r_label2);
    r_if->condition = r_not;
    r_not->operands = (r_cond);
    r_if->body = r_goto;
    r_if->else_body = r_nop;
    r_goto->destination = r_labelid1;
    r_goto_else->destination = r_labelid2;
    r_label1->identifier = r_labelid1a;
    r_label2->identifier = r_labelid2a;
    
    CouplingSet couplings((
        Coupling(( s_cond, r_cond )),
        Coupling(( s_body, r_body )),
        Coupling(( s_else_body, r_else_body )),
        Coupling(( r_labelid1, r_labelid1a )),
        Coupling(( r_labelid2, r_labelid2a )) ));

    SearchReplace( s_and, r_comp, couplings )( context, proot );
}


void DoToIfGoto::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
    MakeTreePtr<Do> s_do;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Statement> s_body, r_body;
    MakeTreePtr<Expression> s_cond, r_cond;
    MakeTreePtr<Goto> r_goto, sr_goto;
    MakeTreePtr<Nop> r_nop;    
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<SoftMakeLabelIdentifier> r_labelid("NEXT"), sr_cont_labelid("CONTINUE");
    MakeTreePtr<LabelIdentifier> r_labelid_a, sr_cont_labelid_a;
    MakeTreePtr<Label> r_label, r_cont_label;
    MakeTreePtr< Stuff<Statement> > ss_stuff, sr_stuff;
    MakeTreePtr<Continue> ss_cont;
    MakeTreePtr< NotMatch<Statement> > ss_not;
    MakeTreePtr< Loop > ss_loop;

    ss_not->pattern = ss_loop;
    ss_stuff->terminus = ss_cont;
    ss_stuff->recurse_restriction = ss_not;
    sr_stuff->terminus = sr_goto;
    sr_goto->destination = sr_cont_labelid_a;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave( r_body, ss_stuff, sr_stuff );
    
    s_do->condition = s_cond;
    s_do->body = s_body;
    
    r_comp->statements = (r_label, r_slave, r_cont_label, r_if);
    r_label->identifier = r_labelid;
    r_cont_label->identifier = sr_cont_labelid;
    r_if->condition = r_cond;
    r_if->body = r_goto;
    r_if->else_body = r_nop;
    r_goto->destination = r_labelid_a;
        
    CouplingSet couplings((
        Coupling(( s_cond, r_cond )),
        Coupling(( s_body, r_body )),
        Coupling(( r_labelid, r_labelid_a )),
        Coupling(( sr_cont_labelid, sr_cont_labelid_a )),
        Coupling(( ss_stuff, sr_stuff )) ));

    SearchReplace( s_do, r_comp, couplings )( context, proot );
}

void BreakToGoto::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
    MakeTreePtr<Breakable> s_breakable, sx_breakable, r_breakable;
    MakeTreePtr< Stuff<Statement> > s_stuff, r_stuff;
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr<Break> s_break;
    MakeTreePtr<Goto> r_goto;
    MakeTreePtr<SoftMakeLabelIdentifier> r_labelid("BREAK");
    MakeTreePtr<LabelIdentifier> r_labelid_a;
    MakeTreePtr<Label> r_label;
    MakeTreePtr<Compound> r_comp;
    
    sx_not->pattern = sx_breakable;
    s_stuff->terminus = s_break;
    s_stuff->recurse_restriction = sx_not;
    r_stuff->terminus = r_goto;
    r_goto->destination = r_labelid_a;
    s_breakable->body = s_stuff;   
   
    r_comp->statements = (r_breakable, r_label);
    r_breakable->body = r_stuff;
    r_label->identifier = r_labelid;
    
    CouplingSet couplings((
        Coupling(( s_breakable, r_breakable )),
        Coupling(( s_stuff, r_stuff )),
        Coupling(( r_labelid, r_labelid_a )) ));

    SearchReplace( s_breakable, r_comp, couplings )( context, proot );
}
