
#include "steps/lower_control_flow.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/typeof.hpp"

// TOOD go through step impls and use inline decls of leaf nodes, to reduce wordiness

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
    MakeTreePtr<BuildLabelIdentifier> r_labelid1("BYPASS_THEN"), r_labelid2("BYPASS_ELSE");
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


void SwitchToIfGoto::operator()( TreePtr<Node> context, TreePtr<Node> *proot )
{
    MakeTreePtr<Switch> s_switch;
    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<Statement> s_body, r_body;
    MakeTreePtr<Type> s_cond_type, r_cond_type;
    MakeTreePtr<Automatic> r_decl;
    MakeTreePtr<BuildInstanceIdentifier> r_id("switch_value");
    TreePtr<TypeOf> s_cond( new TypeOf ); // TODO use MakeTreePtr, confirm this works
    
    // Slave for default
    MakeTreePtr<Compound> ss1_body, sr1_body;
    MakeTreePtr< Star<Declaration> > ss1_decls, sr1_decls;
    MakeTreePtr< Star<Statement> > ss1_pre, sr1_pre;
    MakeTreePtr< Star<Statement> > ss1_post, sr1_post;
    MakeTreePtr< Default > ss1_default;
    MakeTreePtr< Label > sr1_label;
    MakeTreePtr<BuildLabelIdentifier> sr1_labelid("DEFAULT");
    MakeTreePtr<LabelIdentifier> sr1_labelid_a;
    MakeTreePtr<Goto> sr1_goto;
    
    ss1_body->members = ss1_decls;
    ss1_body->statements = (ss1_pre, ss1_default, ss1_post);
    
    sr1_body->members = sr1_decls;
    sr1_body->statements = (sr1_goto, sr1_pre, sr1_label, sr1_post);
    sr1_goto->destination = sr1_labelid_a;
    sr1_label->identifier = sr1_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave1( r_body, ss1_body, sr1_body );

    // slave for normal case statements (single value)
    MakeTreePtr<Compound> ss2_body, sr2_body;
    MakeTreePtr< Star<Declaration> > ss2_decls, sr2_decls;
    MakeTreePtr< Star<Statement> > ss2_pre, sr2_pre;
    MakeTreePtr< Star<Statement> > ss2_post, sr2_post;
    MakeTreePtr< Case > ss2_case;
    MakeTreePtr< Label > sr2_label;
    MakeTreePtr<BuildLabelIdentifier> sr2_labelid("CASE");
    MakeTreePtr<LabelIdentifier> sr2_labelid_a;
    MakeTreePtr<If> sr2_if;
    MakeTreePtr<Nop> sr2_nop;
    MakeTreePtr<Goto> sr2_goto;
    MakeTreePtr<Equal> sr2_equal;
    MakeTreePtr<InstanceIdentifier> sr2_id_a;
    MakeTreePtr<Expression> ss2_exp, sr2_exp;
    
    ss2_body->members = ss2_decls;
    ss2_body->statements = (ss2_pre, ss2_case, ss2_post);
    ss2_case->value = ss2_exp;
    
    sr2_body->members = sr2_decls;
    sr2_body->statements = (sr2_if, sr2_pre, sr2_label, sr2_post);
    sr2_if->condition = sr2_equal;
    sr2_if->body = sr2_goto;
    sr2_if->else_body = sr2_nop;
    sr2_equal->operands = (sr2_id_a, sr2_exp);
    sr2_goto->destination = sr2_labelid_a;
    sr2_label->identifier = sr2_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave2( r_slave1, ss2_body, sr2_body );
    
    // Slave for range cases (GCC extension) eg case 5..7:    
    MakeTreePtr<Compound> ss3_body, sr3_body;
    MakeTreePtr< Star<Declaration> > ss3_decls, sr3_decls;
    MakeTreePtr< Star<Statement> > ss3_pre, sr3_pre;
    MakeTreePtr< Star<Statement> > ss3_post, sr3_post;
    MakeTreePtr< RangeCase > ss3_case;
    MakeTreePtr< Label > sr3_label;
    MakeTreePtr<BuildLabelIdentifier> sr3_labelid("CASE");
    MakeTreePtr<LabelIdentifier> sr3_labelid_a;
    MakeTreePtr<If> sr3_if;
    MakeTreePtr<Nop> sr3_nop;
    MakeTreePtr<Goto> sr3_goto;
    MakeTreePtr<LogicalAnd> sr3_and;
    MakeTreePtr<GreaterOrEqual> sr3_ge;
    MakeTreePtr<LessOrEqual> sr3_le;
    MakeTreePtr<InstanceIdentifier> sr3_id_a, sr3_id_b;
    MakeTreePtr<Expression> ss3_exp_lo, sr3_exp_lo, ss3_exp_hi, sr3_exp_hi;
    
    ss3_body->members = ss3_decls;
    ss3_body->statements = (ss3_pre, ss3_case, ss3_post);
    ss3_case->value_lo = ss3_exp_lo;
    ss3_case->value_hi = ss3_exp_hi;
    
    sr3_body->members = sr3_decls;
    sr3_body->statements = (sr3_if, sr3_pre, sr3_label, sr3_post);
    sr3_if->condition = sr3_and;
    sr3_if->body = sr3_goto;
    sr3_if->else_body = sr3_nop;
    sr3_and->operands = (sr3_ge, sr3_le);
    sr3_ge->operands = (sr3_id_a, sr3_exp_lo);
    sr3_le->operands = (sr3_id_b, sr3_exp_hi);
    sr3_goto->destination = sr3_labelid_a;
    sr3_label->identifier = sr3_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave3( r_slave2, ss3_body, sr3_body );

    // Finish up master
    s_cond->pattern = s_cond_type;
    s_switch->body = s_body; // will only match when body is a compound
    s_switch->condition = s_cond;
    
    r_decl->identifier = r_id;
    r_decl->type = r_cond_type;
    r_decl->initialiser = MakeTreePtr<Expression>();
    r_comp->statements = (r_decl, r_slave3);
    
    CouplingSet couplings(( 
        Coupling(( s_cond_type, r_cond_type )),
        Coupling(( s_cond, r_decl->initialiser )),
        Coupling(( s_body, r_body )),
	Coupling(( ss1_decls, sr1_decls )),
	Coupling(( ss1_pre, sr1_pre )),
	Coupling(( ss1_post, sr1_post )),
	Coupling(( sr1_labelid, sr1_labelid_a )),
	Coupling(( ss2_decls, sr2_decls )),
	Coupling(( ss2_pre, sr2_pre )),
	Coupling(( ss2_post, sr2_post )),
	Coupling(( sr2_labelid, sr2_labelid_a )),
	Coupling(( r_id, sr2_id_a, sr3_id_a, sr3_id_b )),
	Coupling(( ss2_exp, sr2_exp )),
	Coupling(( ss3_decls, sr3_decls )),
	Coupling(( ss3_pre, sr3_pre )),
	Coupling(( ss3_post, sr3_post )),
	Coupling(( sr3_labelid, sr3_labelid_a )),
	Coupling(( ss3_exp_lo, sr3_exp_lo )),
	Coupling(( ss3_exp_hi, sr3_exp_hi )) ));

    SearchReplace( s_switch, r_comp, couplings )( context, proot );
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
    MakeTreePtr<BuildLabelIdentifier> r_labelid("NEXT"), sr_cont_labelid("CONTINUE");
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
    MakeTreePtr<BuildLabelIdentifier> r_labelid("BREAK");
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
