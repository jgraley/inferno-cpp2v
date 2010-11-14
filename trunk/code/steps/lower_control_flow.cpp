
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
