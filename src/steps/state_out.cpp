
#include "steps/state_out.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
 
CompactGotos::CompactGotos()
{
    MakeTreePtr< If > s_if;      
    MakeTreePtr< Expression > cond;      
    MakeTreePtr< Compound > s_comp, r_comp;  
    MakeTreePtr< Goto > s_then_goto, s_else_goto, r_goto;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< Multiplexor > mult;
    
    s_then_goto->destination = MakeTreePtr<Expression>();    
    s_else_goto->destination = MakeTreePtr<Expression>();
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakeTreePtr<Nop>(); // standed conditional branch has no else clause - our "else" is the next statement
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, s_if, s_else_goto, post );    

    mult->operands = (cond, s_then_goto->destination, s_else_goto->destination);
    r_goto->destination = mult;
    r_comp->statements = ( pre, r_goto, post );
    r_comp->members = ( decls );    
        
    SearchReplace::Configure( s_comp, r_comp );
}

CompactGotosFinal::CompactGotosFinal()
{
    MakeTreePtr< If > s_if;      
    MakeTreePtr< Expression > cond;      
    MakeTreePtr< Compound > s_comp, r_comp;  
    MakeTreePtr< Goto > s_then_goto, s_else_goto, r_goto;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< Multiplexor > mult;
    MakeTreePtr< Label > label;    
    MakeTreePtr< BuildLabelIdentifier > label_id("SEQUENTIAL");
    
    s_then_goto->destination = MakeTreePtr<Expression>();    
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakeTreePtr<Nop>(); // standard conditional branch has no else clause - our "else" is the next statement
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, s_if, post );    

    label->identifier = label_id;
    mult->operands = (cond, s_then_goto->destination, label_id);
    r_goto->destination = mult;
    r_comp->members = ( decls );
    r_comp->statements = ( pre, r_goto, label, post );
    
    SearchReplace::Configure( s_comp, r_comp );
}


static TreePtr<Statement> MakeResetAssignmentPattern()
{
    MakeTreePtr<Assign> ass;
    MakeTreePtr< TransformOf<InstanceIdentifier> > decl( &GetDeclaration::instance );
    decl->pattern = MakeTreePtr<LocalVariable>();
    ass->operands = (decl, MakeTreePtr<Literal>());    
    return ass;
}


EnsureBootstrap::EnsureBootstrap()
{
    MakeTreePtr<Instance> fn;
    MakeTreePtr<Subroutine> sub;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< NotMatch<Compound> > s_not;    
    MakeTreePtr<Compound> s_body, r_body, sx_body;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, sx_pre, post;
    MakeTreePtr<Goto> r_goto;
    MakeTreePtr<Label> r_label;    
    MakeTreePtr<BuildLabelIdentifier> r_labelid("BOOTSTRAP");
    MakeTreePtr< NotMatch<Statement> > stop;
        
    fn->type = sub;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (s_not, s_body);
    s_not->pattern = sx_body;
    // only exclude if there is a goto; a goto to anywhere will suffice to boot the state machine
    sx_body->members = (MakeTreePtr< Star<Declaration> >());
    sx_body->statements = (sx_pre, MakeTreePtr<Goto>(), MakeTreePtr< Star<Statement> >()); 
    sx_pre->pattern = MakeResetAssignmentPattern();
    over->overlay = r_body;
    s_body->members = decls;
    s_body->statements = (pre, stop, post);
    pre->pattern = MakeResetAssignmentPattern();    
    stop->pattern = MakeResetAssignmentPattern();
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;
    r_body->members = decls;
    r_body->statements = (pre, r_goto, r_label, stop, post);    

    SearchReplace::Configure( fn );
}


AddStateLabelVar::AddStateLabelVar()
{
    MakeTreePtr<Compound> s_comp, r_comp, lr_compound;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<Goto> ls_goto, lr_goto, sx_goto;
    MakeTreePtr<Assign> lr_assign;
    MakeTreePtr<Automatic> state_var;
    MakeTreePtr< NotMatch<Expression> > sx_not, lsx_not;
    MakeTreePtr<Pointer> ptr_type;
    
    s_comp->members = (decls);
    s_comp->statements = (pre, sx_goto, post); 
    sx_goto->destination = sx_not;
    sx_not->pattern = MakeTreePtr<InstanceIdentifier>();
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, sx_goto, post); 
    state_var->identifier = MakeTreePtr< BuildInstanceIdentifier >("state");
    state_var->type = ptr_type;    
    state_var->initialiser = MakeTreePtr<Uninitialised>();
    ptr_type->destination = MakeTreePtr<Void>();

    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave( r_comp, ls_goto, lr_compound );
     
    ls_goto->destination = lsx_not;
    lsx_not->pattern = state_var->identifier; //  MakeTreePtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var->identifier, ls_goto->destination);
    lr_goto->destination = state_var->identifier;
            
    SearchReplace::Configure( s_comp, r_slave );
}


EnsureSuperLoop::EnsureSuperLoop()
{   
    MakeTreePtr<Instance> fn;
    MakeTreePtr<Subroutine> sub;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< NotMatch<Statement> > sx_not, s_limit;    
    MakeTreePtr< Stuff<Compound> > sx_stuff;
    MakeTreePtr< Goto > sx_goto, first_goto;
    MakeTreePtr<Compound> s_body, r_body, r_loop_body;
    MakeTreePtr< Star<Statement> > pre, post;    
    MakeTreePtr< Star<Declaration> > decls;    
    MakeTreePtr<Do> r_loop;
        
    fn->type = sub;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (sx_stuff, s_body);
    sx_stuff->terminus = sx_goto;
    sx_stuff->recurse_restriction = sx_not;
    sx_not->pattern = MakeTreePtr<Do>();
    s_body->members = (decls);
    s_body->statements = (pre, first_goto, post);
    pre->pattern = s_limit;
    s_limit->pattern = MakeTreePtr<Goto>();
    
    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (pre, r_loop);
    r_loop->body = r_loop_body;
    r_loop_body->statements = (first_goto, post);
    r_loop->condition = MakeTreePtr< SpecificInteger >(1);

    SearchReplace::Configure( fn );
}

ShareGotos::ShareGotos()
{   
    MakeTreePtr<Do> loop;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr<Compound> s_body, r_body;
    MakeTreePtr< Star<Declaration> > decls;    
    MakeTreePtr< Star<Statement> > pre, post;    
    MakeTreePtr< Goto > first_goto, r_goto;
    MakeTreePtr<Label> r_label;    
    MakeTreePtr<BuildLabelIdentifier> r_labelid("ITERATE");
                    
    loop->body = over;
    loop->condition = MakeTreePtr<SpecificInteger>(1);
    over->through = s_body;
    s_body->members = (decls);
    s_body->statements = (first_goto, pre, first_goto, post);    

    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (first_goto, pre, r_goto, post, r_label);    
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;

    SearchReplace::Configure( loop );
}


// Something to get the size of the Collection matched by a Star as a SpecificInteger
struct BuildContainerSize : CompareReplace::SoftReplacePattern,
                            Special<Integer>
{
    SPECIAL_NODE_FUNCTIONS
    TreePtr< StarBase > container;
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
        ASSERT( container );
	    TreePtr<Node> n = sr->DuplicateSubtreePattern( container );
	    ASSERT( n );
	    TreePtr<SearchReplace::SubCollection> sc = dynamic_pointer_cast<SearchReplace::SubCollection>(n);
	    ASSERT( sc );
	    int size = sc->size();
	    TreePtr<SpecificInteger> si = MakeTreePtr<SpecificInteger>(size);
	    return si;
    }                                                   
}; 


struct IsLabelReached : CompareReplace::SoftSearchPattern, Special<LabelIdentifier>
{
	SPECIAL_NODE_FUNCTIONS	
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
    virtual Result DecidedCompare( const CompareReplace *sr,
                                   TreePtr<Node> xx,
                                   bool can_key,
                                   Conjecture &conj ) const
    {
        INDENT;
        ASSERT( pattern );
        TreePtr<Node> n = sr->coupling_keys->GetCoupled( pattern ); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
        if( !n )
            n = pattern;
        TreePtr<Expression> y = dynamic_pointer_cast<Expression>( n );
        ASSERT( y )("IsLabelReached saw pattern coupled to ")(*n)(" but an Expression is needed\n"); 
        ASSERT( xx );
        TreePtr<LabelIdentifier> x = dynamic_pointer_cast<LabelIdentifier>( xx );
        ASSERT( x )("IsLabelReached at ")(*xx)(" but is of type LabelIdentifier\n"); 
        UniqueFilter uf;        
        Result r = CanReachExpr(sr, &uf, x, y);
        TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
        return r;
    }                 
    TreePtr<Expression> pattern;                  
private:
    Result CanReachExpr( const CompareReplace *sr,
                         Filter *f,
                         TreePtr<LabelIdentifier> x, 
                         TreePtr<Expression> y ) const // y is expression. Can it yield expression x?
    {
        INDENT;
        Result r = NOT_FOUND;
        if( TreePtr<LabelIdentifier> liy = dynamic_pointer_cast<LabelIdentifier>(y) )
            r = liy->IsLocalMatch( x.get() ) ? FOUND : NOT_FOUND; // y is x, so yes
        else if( TreePtr<InstanceIdentifier> iiy = dynamic_pointer_cast<InstanceIdentifier>( y ) )
            r = CanReachVar( sr, f, x, iiy );
        else if( TreePtr<Ternop> ty = dynamic_pointer_cast<Ternop>( y ) )
            r = (CanReachExpr(sr, f, x, ty->operands[1]) ||
                 CanReachExpr(sr, f, x, ty->operands[2]) ) ? FOUND : NOT_FOUND; // only the choices, not the condition
        else if( TreePtr<Comma> cy = dynamic_pointer_cast<Comma>( y ) )
            r = CanReachExpr(sr, f, x, ty->operands[1]) ? FOUND : NOT_FOUND; // second operand
        else if( dynamic_pointer_cast<Dereference>( y ) )
            r = FOUND; // assume everything is in memory
            
        TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
        return r;        
    }    
    Result CanReachVar( const CompareReplace *sr,
                        Filter *f,
                        TreePtr<LabelIdentifier> x, 
                        TreePtr<InstanceIdentifier> y ) const // y is instance identifier. Can expression x be assigned to it?
    {
        INDENT;
        Result r = NOT_FOUND;
        Expand e( *sr->pcontext, f ); // use a unique filter to ensure we only see each assignment once.
                                      // A single filter instance is used across the whole recursion.
                                      // This way we do not recurse forever when there are loops in the data flow.

        FOREACH( TreePtr<Node> n, e )
        {
            if( TreePtr<Assign> a = dynamic_pointer_cast<Assign>(n) )
            {
                TRACE("Examining assignment: ")(*a->operands[0])(" = ")(*a->operands[1])("\n"); 
                if( a->operands[0] == y )
                {
                    if( CanReachExpr( sr, f, x, a->operands[1] ) )
                    {
                        r = FOUND;
                        break; // early out, since we have the info we need
                    }
                }
            }
        }
        TRACE("I reakon ")(*x)(" at %p", x.get())(r?" does ":" does not ")("reach ")(*y)("\n"); 
        return r;
    }
};


InsertSwitch::InsertSwitch()
{
    MakeTreePtr<Instance> fn;
    MakeTreePtr<InstanceIdentifier> fn_id;
    MakeTreePtr<Subroutine> sub;
    MakeTreePtr< Overlay<Compound> > func_over, over, l_over;
    MakeTreePtr< Compound > ls_func_comp, lr_func_comp, s_func_comp, r_func_comp, s_comp, r_comp, r_switch_comp, l_comp, ls_switch_comp, lr_switch_comp;
    MakeTreePtr< Star<Declaration> > func_decls, decls, l_enum_vals;
    MakeTreePtr< Star<Statement> > func_pre, func_post, pre, body, l_func_pre, l_func_post, l_pre, l_post;
    MakeTreePtr< Stuff<Statement> > stuff, l_stuff; // TODO these are parallel stuffs, which is bad. Use two first-level slaves 
                                                    // and modify S&R to allow couplings between them. This means running slaves 
                                                    // in a post-pass and doing existing passes across all same-level slaves
    MakeTreePtr<Goto> s_first_goto; 
    MakeTreePtr<Label> ls_label; 
    MakeTreePtr<Switch> r_switch, l_switch;     
    MakeTreePtr<Enum> r_enum, ls_enum, lr_enum;         
    MakeTreePtr< NotMatch<Statement> > s_prenot, s_postnot, xs_rr;
    MakeTreePtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakeTreePtr<Static> lr_state_decl;    
    MakeTreePtr<BuildInstanceIdentifier> lr_state_id("STATE_%s");
    MakeTreePtr<Case> lr_case;
    MakeTreePtr<Signed> lr_int;
    MakeTreePtr<BuildContainerSize> lr_count;
    MakeTreePtr<IsLabelReached> ls_label_id;
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr<Instance> var_decl, l_var_decl;
    MakeTreePtr< Overlay<Type> > var_over;  
    MakeTreePtr<Pointer> s_ptr;
    MakeTreePtr<Label> xs_pre_label;
    MakeTreePtr<IsLabelReached> xs_pre_reach;
    MakeTreePtr< MatchAll<Node> > ll_all;
    MakeTreePtr< NotMatch<Node> > lls_not1, lls_not2;    
    MakeTreePtr< AnyNode<Node> > ll_any;
    MakeTreePtr< Overlay<Node> > ll_over;
    MakeTreePtr<Goto> lls_goto;    
    MakeTreePtr<Label> lls_label;    
            
    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls, var_decl);
    var_decl->type = var_over;
    var_decl->identifier = var_id;
    var_over->through = s_ptr;
    s_ptr->destination = MakeTreePtr<Void>();
    s_func_comp->statements = (func_pre, stuff, func_post);
    stuff->terminus = over;
    stuff->recurse_restriction = xs_rr; // TODO Add support for elsewhere restriction in stuff node, restrict for no reaches
    xs_rr->pattern = MakeTreePtr<Switch>(); // stop it doing a second switch inside one we just created
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (pre, s_first_goto, body);
    pre->pattern = s_prenot;
    s_prenot->pattern = xs_pre_label;
    xs_pre_label->identifier = xs_pre_reach;
    xs_pre_reach->pattern = var_id;
    s_first_goto->destination = var_id;

    r_func_comp->members = (func_decls, r_enum, var_decl);
    var_over->overlay = r_enum_id;
    r_enum->identifier = r_enum_id;
    r_enum_id->sources = (fn_id);        
    r_func_comp->statements = (func_pre, stuff, func_post);
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (pre, r_switch);    
    r_switch->body = r_switch_comp;
    r_switch_comp->statements = (body);
    r_switch->condition = var_id;

    MakeTreePtr< SlaveSearchReplace<Compound> > lr_sub_slave( lr_func_comp, ll_all );    
    MakeTreePtr< SlaveCompareReplace<Compound> > r_slave( r_func_comp, ls_func_comp, lr_sub_slave );
    func_over->overlay = r_slave;
    ls_func_comp->members = (func_decls, ls_enum, l_var_decl);
    ls_enum->members = (l_enum_vals);
    ls_enum->identifier = r_enum_id; // need to match id, not enum itself, because enum's members will change during slave
    ls_func_comp->statements = (l_func_pre, l_stuff, l_func_post);
    l_stuff->terminus = l_comp;
    l_comp->members = (decls);
    l_comp->statements = (pre, l_switch);
    l_switch->body = l_over;
    l_switch->condition = s_first_goto->destination;
    l_over->through = ls_switch_comp;
    ls_switch_comp->statements = (l_pre, ls_label, l_post);
    ls_label->identifier = ls_label_id;
    ls_label_id->pattern = var_id;
    
    lr_func_comp->members = (func_decls, lr_enum, l_var_decl);
    lr_enum->members = (l_enum_vals, lr_state_decl);
    lr_enum->identifier = r_enum_id;
    lr_state_decl->constancy = MakeTreePtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakeTreePtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (ls_label->identifier);
    lr_func_comp->statements = (l_func_pre, l_stuff, l_func_post);
    l_over->overlay = lr_switch_comp;
    lr_switch_comp->statements = (l_pre, lr_case, /*ls_label,*/ l_post); 
    // TODO retain the label for direct gotos, BUT we are spinning at 1st slave because we think the
    // label is still a state, because we think it reaches state var, because the LABEL->STATE_LABEL change 
    // is not being seen by IsLabelReached, because this is done by 2nd slave and stays in temps until
    // the master's SingleCompareReplace() completes. Uh-oh!
    // but only after fixing S&R to ensure slave output goes into context
    lr_case->value = lr_state_id;

    ll_all->patterns = (ll_any, lls_not1, lls_not2);
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = lr_state_id;
    lls_not1->pattern = lls_goto;
    lls_goto->destination = ls_label_id; // leave gotos alone in the body
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    SearchReplace::Configure( fn );    
}


SwitchCleanUp::SwitchCleanUp()
{
    MakeTreePtr<Compound> r_comp, s_body, r_body;
    MakeTreePtr<Switch> s_switch, r_switch;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > main, tail;
    MakeTreePtr<Label> label;
    MakeTreePtr<Expression> cond;
    MakeTreePtr< NotMatch<Statement> > sx_not_tail, sx_not_main;
    MakeTreePtr< MatchAny<Statement> > sx_any_tail;

    s_switch->condition = cond;
    s_switch->body = s_body;
    s_body->members = decls;
    s_body->statements = (main, label, tail);
    main->pattern = sx_not_main;
    sx_not_main->pattern = MakeTreePtr<Break>();
    tail->pattern = sx_not_tail;
    sx_not_tail->pattern = sx_any_tail;
    sx_any_tail->patterns = (MakeTreePtr<Break>(), MakeTreePtr<Case>());
    
    r_comp->statements = (r_switch, label, tail);
    r_switch->condition = cond;
    r_switch->body = r_body;
    r_body->members = decls;
    r_body->statements = (main);    
    
    SearchReplace::Configure( s_switch, r_comp );        
}


InferBreak::InferBreak()
{
    MakeTreePtr<Goto> ls_goto;
    MakeTreePtr<Label> label;
    MakeTreePtr<LabelIdentifier> label_id;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<Breakable> breakable;
    MakeTreePtr<Break> lr_break;
    MakeTreePtr<Compound> s_comp, r_comp;

    ls_goto->destination = label_id;

    MakeTreePtr< SlaveSearchReplace<Breakable> > slave( breakable, ls_goto, lr_break ); // todo restrict to not go through more breakables

    s_comp->members = decls;
    s_comp->statements = (pre, breakable, label, post);
    label->identifier = label_id;
    
    r_comp->members = decls;
    r_comp->statements = (pre, slave, post); 
    
    SearchReplace::Configure( s_comp, r_comp );        
}


FixFallthrough::FixFallthrough()
{
    // don't actually need a switch statement here, just look in the body, pattern contains Case statements
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, cb1, cb2, post;
    MakeTreePtr<Case> case1, case2;
    MakeTreePtr<Break> breakk;
    MakeTreePtr< NotMatch<Statement> > s_not1, s_not2;
    
    s_comp->members = (decls);
    s_comp->statements = (pre, case1, cb1,              case2, cb2, breakk, post);
    r_comp->members = (decls);
    r_comp->statements = (pre, case1, cb1, cb2, breakk, case2, cb2, breakk, post);
    cb1->pattern = s_not1;
    s_not1->pattern = MakeTreePtr<Break>();
    cb2->pattern = s_not2;
    s_not2->pattern = MakeTreePtr<Case>();
        
    SearchReplace::Configure( s_comp, r_comp );            
}

