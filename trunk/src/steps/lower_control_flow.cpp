
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
    MakeTreePtr<Statement> body;
    MakeTreePtr<Type> cond_type;
    MakeTreePtr<Automatic> r_decl;
    MakeTreePtr<BuildInstanceIdentifier> id("switch_value");
    TreePtr<TypeOf> s_cond( new TypeOf ); // TODO use MakeTreePtr, confirm this works
    
    // Slave for default
    MakeTreePtr<Compound> l1_s_body, l1_r_body;
    MakeTreePtr< Star<Declaration> > l1_decls;
    MakeTreePtr< Star<Statement> > l1_pre, l1_post;
    MakeTreePtr< Default > l1_s_default;
    MakeTreePtr< Label > l1_r_label;
    MakeTreePtr<BuildLabelIdentifier> l1_r_labelid("DEFAULT");
    MakeTreePtr<Goto> l1_r_goto;
    
    l1_s_body->members = l1_decls;
    l1_s_body->statements = (l1_pre, l1_s_default, l1_post);
    
    l1_r_body->members = l1_decls;
    l1_r_body->statements = (l1_r_goto, l1_pre, l1_r_label, l1_post);
    l1_r_goto->destination = l1_r_labelid;
    l1_r_label->identifier = l1_r_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave1( body, l1_s_body, l1_r_body );

    // slave for normal case statements (single value)
    MakeTreePtr<Compound> l2_s_body, l2_r_body;
    MakeTreePtr< Star<Declaration> > l2_decls;
    MakeTreePtr< Star<Statement> > l2_pre, l2_post;
    MakeTreePtr< Case > l2_s_case;
    MakeTreePtr< Label > l2_r_label;
    MakeTreePtr<BuildLabelIdentifier> l2_r_labelid("CASE");
    MakeTreePtr<If> l2_r_if;
    MakeTreePtr<Nop> l2_r_nop;
    MakeTreePtr<Goto> l2_r_goto;
    MakeTreePtr<Equal> l2_r_equal;
    MakeTreePtr<Expression> l2_exp;
    
    l2_s_body->members = l2_decls;
    l2_s_body->statements = (l2_pre, l2_s_case, l2_post);
    l2_s_case->value = l2_exp;
    
    l2_r_body->members = l2_decls;
    l2_r_body->statements = (l2_r_if, l2_pre, l2_r_label, l2_post);
    l2_r_if->condition = l2_r_equal;
    l2_r_if->body = l2_r_goto;
    l2_r_if->else_body = l2_r_nop;
    l2_r_equal->operands = (id, l2_exp);
    l2_r_goto->destination = l2_r_labelid;
    l2_r_label->identifier = l2_r_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave2( r_slave1, l2_s_body, l2_r_body );
    
    // Slave for range cases (GCC extension) eg case 5..7:    
    MakeTreePtr<Compound> l3_s_body, l3_r_body;
    MakeTreePtr< Star<Declaration> > l3_decls;
    MakeTreePtr< Star<Statement> > l3_pre, l3_post;
    MakeTreePtr< RangeCase > l3_s_case;
    MakeTreePtr< Label > l3_r_label;
    MakeTreePtr<BuildLabelIdentifier> l3_r_labelid("CASE");
    MakeTreePtr<If> l3_r_if;
    MakeTreePtr<Nop> l3_r_nop;
    MakeTreePtr<Goto> l3_r_goto;
    MakeTreePtr<LogicalAnd> l3_r_and;
    MakeTreePtr<GreaterOrEqual> l3_r_ge;
    MakeTreePtr<LessOrEqual> l3_r_le;
    MakeTreePtr<Expression> l3_exp_lo, l3_exp_hi;
    
    l3_s_body->members = l3_decls;
    l3_s_body->statements = (l3_pre, l3_s_case, l3_post);
    l3_s_case->value_lo = l3_exp_lo;
    l3_s_case->value_hi = l3_exp_hi;
    
    l3_r_body->members = l3_decls;
    l3_r_body->statements = (l3_r_if, l3_pre, l3_r_label, l3_post);
    l3_r_if->condition = l3_r_and;
    l3_r_if->body = l3_r_goto;
    l3_r_if->else_body = l3_r_nop;
    l3_r_and->operands = (l3_r_ge, l3_r_le);
    l3_r_ge->operands = (id, l3_exp_lo);
    l3_r_le->operands = (id, l3_exp_hi);
    l3_r_goto->destination = l3_r_labelid;
    l3_r_label->identifier = l3_r_labelid;
    
    MakeTreePtr< RootedSlave<Statement> > r_slave3( r_slave2, l3_s_body, l3_r_body );

    // Finish up master
    s_cond->pattern = cond_type;
    s_switch->body = body; // will only match when body is a compound
    s_switch->condition = s_cond;
    
    r_decl->identifier = id;
    r_decl->type = cond_type;
    r_decl->initialiser = MakeTreePtr<Expression>();
    r_comp->statements = (r_decl, r_slave3);
    
    CouplingSet couplings;
	couplings.insert(  
        Coupling(( s_cond, r_decl->initialiser )) );

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
    MakeTreePtr<Label> r_label;
    MakeTreePtr<Compound> r_comp;
    
    sx_not->pattern = sx_breakable;
    s_stuff->terminus = s_break;
    s_stuff->recurse_restriction = sx_not;
    r_stuff->terminus = r_goto;
    r_goto->destination = r_labelid;
    s_breakable->body = s_stuff;   
   
    r_comp->statements = (r_breakable, r_label);
    r_breakable->body = r_stuff;
    r_label->identifier = r_labelid;
    
    CouplingSet couplings((
        Coupling(( s_breakable, r_breakable )),
        Coupling(( s_stuff, r_stuff )) ));

    SearchReplace( s_breakable, r_comp, couplings )( context, proot );
}
