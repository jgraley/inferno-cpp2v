
#include "steps/state_out.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_patterns.hpp"

 
using namespace CPPTree;
using namespace SCTree;
using namespace Steps;
 
// TOOD go through step impls and use inline decls of leaf nodes, to reduce wordiness

GotoAfterWait::GotoAfterWait()
{
    // This step could have been tricky: we'd have needed 3 cases:
    // 1. { *, wait, !goto, * }
    // 2. { *, wait } // not covered by previous case because !goto needs to match a statement
    // 3. eg if() wait // not directly inside a compound block 
    // we use the but-not pattern, in which we principally search for anything->wait, but exclude
    // the only case that needs excluding which is a compound like in case 1 above but without
    // notting the goto. Cases 2 and 3 work because the excluded compound does not match.
    // Overall, a good example of but-not
    MakePatternPtr<Compound> sx_comp, r_comp;
    MakePatternPtr< Star<Declaration> > sx_decls;
    MakePatternPtr< Star<Statement> > sx_pre, sx_post;    
    MakePatternPtr<Wait> wait;
    MakePatternPtr< NotMatch<Statement> > notmatch;
    MakePatternPtr< MatchAll<Statement> > all;
    MakePatternPtr< AnyNode<Statement> > anynode;
    MakePatternPtr< Overlay<Statement> > over;
    MakePatternPtr<Goto> sx_goto, r_goto;
    MakePatternPtr<Label> r_label;
    MakePatternPtr<BuildLabelIdentifier> r_labelid("YIELD");
            
    all->patterns = (anynode, notmatch);
    anynode->terminus = over;
    over->through = wait;
    notmatch->pattern = sx_comp;
    sx_comp->members = sx_decls;
    sx_comp->statements = (sx_pre, wait, sx_goto, sx_post);    
    
    over->overlay = r_comp;
    //r_comp->members = ();
    r_comp->statements = (wait, r_goto, r_label);
    r_goto->destination = r_labelid;
    r_label->identifier = r_labelid;
    
    Configure( all );
}
/*
GotoAfterWait::GotoAfterWait()
{
    // TODO will miss a yield at the very end
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Wait> wait;
    MakePatternPtr< NotMatch<Statement> > notmatch;
    MakePatternPtr<Goto> sx_goto, r_goto;
    MakePatternPtr<Label> r_label;
    MakePatternPtr<BuildLabelIdentifier> r_labelid("YIELD");
        
    s_comp->members = (decls);
    s_comp->statements = (pre, wait, notmatch, post);
    notmatch->pattern = sx_goto;
    
    r_comp->members = (decls);
    r_comp->statements = (pre, wait, r_goto, r_label, notmatch, post);
    r_goto->destination = r_labelid;
    r_label->identifier = r_labelid;
    
    Configure( s_comp, r_comp );
}
*/

NormaliseConditionalGotos::NormaliseConditionalGotos()
{
    MakePatternPtr< If > iif;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp, sx_comp;  
    MakePatternPtr< Goto > then_goto, s_else_goto, r_goto, sx_goto;// TODO sx_goto could be any departure, like Return or Cease etc
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post, sx_post;
    MakePatternPtr< Multiplexor > mult;
    MakePatternPtr< Label > label;    
    MakePatternPtr< BuildLabelIdentifier > label_id("PROCEED");
    MakePatternPtr< MatchAll<Statement> > s_all;
    MakePatternPtr< NotMatch<Statement> > sx_not;    
    
    s_all->patterns = (s_comp, sx_not);
    sx_not->pattern = sx_comp;    
    iif->condition = cond;
    iif->body = then_goto;
    iif->else_body = MakePatternPtr<Nop>(); 
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, iif, post );    
    sx_comp->statements = ( pre, iif, sx_goto, sx_post );    

    label->identifier = label_id;
    r_goto->destination = label_id;
    r_comp->members = ( decls );
    r_comp->statements = ( pre, iif, r_goto, label, post );
    
    Configure( s_all, r_comp );
}


CompactGotos::CompactGotos()
{
    MakePatternPtr< If > s_if;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp;  
    MakePatternPtr< Goto > s_then_goto, s_else_goto, r_goto;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< Multiplexor > mult;
    
    s_then_goto->destination = MakePatternPtr<Expression>();    
    s_else_goto->destination = MakePatternPtr<Expression>();
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakePatternPtr<Nop>(); // standed conditional branch has no else clause - our "else" is the next statement
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, s_if, s_else_goto, post );    

    mult->operands = (cond, s_then_goto->destination, s_else_goto->destination);
    r_goto->destination = mult;
    r_comp->statements = ( pre, r_goto, post );
    r_comp->members = ( decls );    
        
    Configure( s_comp, r_comp );
}


AddGotoBeforeLabel::AddGotoBeforeLabel() // TODO really slow!!11
{
    MakePatternPtr< If > s_if;      
    MakePatternPtr< Expression > cond;      
    MakePatternPtr< Compound > s_comp, r_comp, sx_comp;  
    MakePatternPtr< Goto > r_goto;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post, sx_pre, sx_post;
    MakePatternPtr< Multiplexor > mult;
    MakePatternPtr< Label > label;    
    MakePatternPtr< LabelIdentifier > label_id;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Compound> > s_not;
        
    s_all->patterns = (s_comp, s_not);
    s_not->pattern = sx_comp;
    s_comp->members = ( decls );    
    s_comp->statements = ( pre, label, post );    
    label->identifier = label_id;
    sx_comp->members = ( decls );    
    sx_comp->statements = ( sx_pre, MakePatternPtr<Goto>(), label, sx_post );

    r_comp->members = ( decls );    
    r_comp->statements = ( pre, r_goto, label, post );
    r_goto->destination = label_id;

    Configure( s_all, r_comp );
}


static TreePtr<Statement> MakeResetAssignmentPattern()
{
    MakePatternPtr<Assign> ass;
    MakePatternPtr< TransformOf<InstanceIdentifier> > decl( &GetDeclaration::instance );
    decl->pattern = MakePatternPtr<LocalVariable>();
    ass->operands = (decl, MakePatternPtr<Literal>());    
    return ass;
}


EnsureBootstrap::EnsureBootstrap()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Compound> > s_not;    
    MakePatternPtr<Compound> s_body, r_body, sx_body;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, sx_pre, post;
    MakePatternPtr<Goto> r_goto;
    MakePatternPtr<Label> r_label;    
    MakePatternPtr<BuildLabelIdentifier> r_labelid("BOOTSTRAP");
    MakePatternPtr< NotMatch<Statement> > stop;
    MakePatternPtr<Goto> sx_goto;
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (s_not, s_body);
    s_not->pattern = sx_body;
    // only exclude if there is a goto; a goto to anywhere will suffice to boot the state machine
    sx_body->members = (MakePatternPtr< Star<Declaration> >());
    sx_body->statements = (sx_pre, sx_goto, MakePatternPtr< Star<Statement> >());     
    sx_pre->pattern = MakeResetAssignmentPattern();
    sx_goto->destination = MakePatternPtr<LabelIdentifier>(); // must be a hard goto to exclude - otherwise might 
                                                           // have calculations in it which is no good for bootstrapping
    over->overlay = r_body;
    s_body->members = decls;
    s_body->statements = (pre, stop, post);
    pre->pattern = MakeResetAssignmentPattern();    
    stop->pattern = MakeResetAssignmentPattern();
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;
    r_body->members = decls;
    r_body->statements = (pre, r_goto, r_label, stop, post);    

    Configure( fn );
}


AddStateLabelVar::AddStateLabelVar()
{
    MakePatternPtr<Compound> s_comp, r_comp, lr_compound;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Goto> ls_goto, lr_goto, sx_goto;
    MakePatternPtr<Assign> lr_assign;
    MakePatternPtr<Automatic> state_var;
    MakePatternPtr< NotMatch<Expression> > sx_not, lsx_not;
    MakePatternPtr< BuildInstanceIdentifier > state_var_id("state");
    
    ls_goto->destination = lsx_not;
    lsx_not->pattern = state_var_id; //  MakePatternPtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lsx_not);
    lr_goto->destination = state_var_id;
            
    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave( r_comp, ls_goto, lr_compound );
     
    s_comp->members = (decls);
    s_comp->statements = (pre, sx_goto, post); 
    sx_goto->destination = sx_not;
    sx_not->pattern = MakePatternPtr<InstanceIdentifier>();
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, sx_goto, post); 
    state_var->identifier = state_var_id;
    state_var->type = MakePatternPtr<Labeley>();    
    state_var->initialiser = MakePatternPtr<Uninitialised>();

    Configure( s_comp, r_slave );
}


EnsureSuperLoop::EnsureSuperLoop()
{   
    MakePatternPtr<Instance> fn;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Statement> > sx_not, s_limit;    
    MakePatternPtr< Stuff<Compound> > sx_stuff;
    MakePatternPtr< Goto > sx_goto, first_goto;
    MakePatternPtr<Compound> s_body, r_body, r_loop_body;
    MakePatternPtr< Star<Statement> > pre, post;    
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr<Do> r_loop;
        
    fn->type = thread;
    fn->initialiser = over;
    over->through = s_all;
    s_all->patterns = (sx_stuff, s_body);
    sx_stuff->terminus = sx_goto;
    sx_stuff->recurse_restriction = sx_not;
    sx_not->pattern = MakePatternPtr<Do>();
    s_body->members = (decls);
    s_body->statements = (pre, first_goto, post);
    pre->pattern = s_limit;
    s_limit->pattern = MakePatternPtr<Goto>();
    
    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (pre, r_loop);
    r_loop->body = r_loop_body;
    r_loop_body->statements = (first_goto, post);
    r_loop->condition = MakePatternPtr<True>();

    Configure( fn );
}

ShareGotos::ShareGotos()
{   
    MakePatternPtr<Do> loop;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr<Compound> s_body, r_body;
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr< Star<Statement> > pre, post;    
    MakePatternPtr< Goto > first_goto, r_goto;
    MakePatternPtr<Label> r_label;    
    MakePatternPtr<BuildLabelIdentifier> r_labelid("ITERATE");
                    
    loop->body = over;
    loop->condition = MakePatternPtr<SpecificInteger>(1);
    over->through = s_body;
    s_body->members = (decls);
    s_body->statements = (first_goto, pre, first_goto, post);    

    over->overlay = r_body;
    r_body->members = (decls);
    r_body->statements = (first_goto, pre, r_goto, post, r_label);    
    r_label->identifier = r_labelid;
    r_goto->destination = r_labelid;

    Configure( loop );
}


// Something to get the size of the Collection matched by a Star as a SpecificInteger
struct BuildContainerSize : SoftReplacePattern,
                            Special<Integer>
{
    SPECIAL_NODE_FUNCTIONS
    shared_ptr< StarBase > container;
private:
    virtual TreePtr<Node> MyBuildReplace()
    {
        ASSERT( container );
	    TreePtr<Node> n = DoBuildReplace( container );
	    ASSERT( n );
	    ContainerInterface *n_container = dynamic_cast<ContainerInterface *>(n.get());
	    ASSERT( n_container );
	    int size = n_container->size();
	    TreePtr<SpecificInteger> si = MakePatternPtr<SpecificInteger>(size);
	    return si;
    }                                                   
}; 


struct IsLabelReached : SoftSearchPattern, Special<LabelIdentifier>
{
	SPECIAL_NODE_FUNCTIONS	
	virtual void FlushCache()
	{
	    cache.clear();
	}
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
    virtual bool MyCompare( const TreePtrInterface &xx ) 
    {
        INDENT;
        ASSERT( pattern );
        if( IsCanKey() )
            return true; // Want to wait for our pattern to get keyed before we do the search, so wait for restricting pass
        TreePtr<Node> n = GetCoupled( pattern ); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
        if( !n )
            n = pattern;
        TreePtr<Expression> y = dynamic_pointer_cast<Expression>( n );
        ASSERT( y )("IsLabelReached saw pattern coupled to ")(*n)(" but an Expression is needed\n"); 
        ASSERT( xx );
        TreePtr<Node> nxx = xx;
        TreePtr<LabelIdentifier> x = dynamic_pointer_cast<LabelIdentifier>( nxx );
        ASSERT( x )("IsLabelReached at ")(*xx)(" but is of type LabelIdentifier\n"); 
        TRACE("Can label id ")(*x)(" reach expression ")(*y)("?\n");

        Set< TreePtr<InstanceIdentifier> > uf;        
        bool r = CanReachExpr(&uf, x, y);
        TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
        return r;
    }                 
    TreePtr<Expression> pattern;           
           
private:
    bool CanReachExpr( Set< TreePtr<InstanceIdentifier> > *f,
                         TreePtr<LabelIdentifier> x, 
                         TreePtr<Expression> y ) // y is expression. Can it yield label x?
    {
        INDENT;
        bool r = false;
        if( TreePtr<LabelIdentifier> liy = dynamic_pointer_cast<LabelIdentifier>(y) )
            r = liy->IsLocalMatch( x.get() ); // y is x, so yes
        else if( TreePtr<InstanceIdentifier> iiy = dynamic_pointer_cast<InstanceIdentifier>( y ) )
            r = CanReachVar(f, x, iiy );
        else if( TreePtr<Ternop> ty = dynamic_pointer_cast<Ternop>( y ) )
            r = CanReachExpr(f, x, ty->operands[1]) ||
                CanReachExpr(f, x, ty->operands[2]); // only the choices, not the condition
        else if( TreePtr<Comma> cy = dynamic_pointer_cast<Comma>( y ) )
            r = CanReachExpr(f, x, ty->operands[1]); // second operand
        else if( TreePtr<Subscript> sy = dynamic_pointer_cast<Subscript>( y ) ) // subscript as r-value
            r = CanReachExpr(f, x, sy->operands[0]); // first operand
        else if( dynamic_pointer_cast<Dereference>( y ) )
            ASSERTFAIL("IsLabelReached used on expression that is read from memory, cannot figure out the answer\n");
            
        TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
        return r;        
    }    
    
    bool CanReachVar( Set< TreePtr<InstanceIdentifier> > *f,
                      TreePtr<LabelIdentifier> x, 
                      TreePtr<InstanceIdentifier> y ) // y is instance identifier. Can expression x be assigned to it?
    {
        INDENT;
        Reaching rr( x, y );
        if( cache.IsExist(rr) )
        {
            TRACE("cache hit yeah yeah\n");
            return cache[rr];
        }
        bool r = false;        
        Walk e( *GetContext() ); 
        
        if( f->IsExist(y) )
            return false; // already processing this identifier, so we have a loop
                          // so don't recurse further
                          
        f->insert(y);                          

        TRACE("Looking for assignment like ")(*y)(" = ")(*x)("\n");

        FOREACH( TreePtr<Node> n, e )
        {
            if( TreePtr<Assign> a = dynamic_pointer_cast<Assign>(n) )
            {
                TreePtr<Expression> lhs = a->operands[0];
                if( TreePtr<Subscript> slhs = dynamic_pointer_cast<Subscript>( lhs ) ) // subscript as l-value 
                    lhs = slhs->operands[0];
                TRACE("Examining assignment: ")(*lhs)(" = ")(*a->operands[1])("\n"); 
                if( lhs == y )
                {
                    if( CanReachExpr( f, x, a->operands[1] ) )
                    {
                        r = true;                        
                        break; // early out, since we have the info we need
                    }
                }
            }
        }
        
        f->erase(y);
        return r;
    }
    
    struct Reaching
    {
        Reaching( TreePtr<LabelIdentifier> f, TreePtr<InstanceIdentifier> t ) : from(f), to(t) {}
        const TreePtr<LabelIdentifier> from;
        const TreePtr<InstanceIdentifier> to;
        bool operator<( const Reaching &other ) const 
        {
            return from==other.from ? to<other.to : from<other.from;
        }
    };
    Map<Reaching, bool> cache; 
};


InsertSwitch::InsertSwitch()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Callable> sub;
    MakePatternPtr< Overlay<Compound> > func_over, over, l_over;
    MakePatternPtr< Compound > ls_func_comp, lr_func_comp, s_func_comp, r_func_comp, s_comp, r_comp, r_switch_comp, l_comp, ls_switch_comp, lr_switch_comp;
    MakePatternPtr< Star<Declaration> > func_decls, decls, l_enum_vals;
    MakePatternPtr< Star<Statement> > func_pre, func_post, pre, body, l_func_pre, l_func_post, l_pre, l_post;
    MakePatternPtr< Stuff<Statement> > stuff, l_stuff; // TODO these are parallel stuffs, which is bad. Use two first-level slaves 
                                                    // and modify S&R to allow couplings between them. This means running slaves 
                                                    // in a post-pass and doing existing passes across all same-level slaves
    MakePatternPtr<Goto> s_first_goto; 
    MakePatternPtr<Label> ls_label; 
    MakePatternPtr<Switch> r_switch, l_switch;     
    MakePatternPtr<Enum> r_enum, ls_enum, lr_enum;         
    MakePatternPtr< NotMatch<Statement> > s_prenot, s_postnot, xs_rr;
    MakePatternPtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakePatternPtr<Static> lr_state_decl;    
    MakePatternPtr<BuildInstanceIdentifier> lr_state_id("STATE_%s");
    MakePatternPtr<Case> lr_case;
    MakePatternPtr<Signed> lr_int;
    MakePatternPtr<BuildContainerSize> lr_count;
    MakePatternPtr<IsLabelReached> ls_label_id;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<Instance> var_decl, l_var_decl;
    MakePatternPtr< Overlay<Type> > var_over;  
    MakePatternPtr<Label> xs_pre_label;
    MakePatternPtr<IsLabelReached> xs_pre_reach;
    MakePatternPtr< MatchAll<Node> > ll_all;
    MakePatternPtr< NotMatch<Node> > lls_not1, lls_not2;    
    MakePatternPtr< AnyNode<Node> > ll_any;
    MakePatternPtr< Overlay<Node> > ll_over;
    MakePatternPtr<Goto> lls_goto;    
    MakePatternPtr<Label> lls_label;    
            
    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls, var_decl);
    var_decl->type = var_over;
    var_decl->identifier = var_id;
    var_over->through = MakePatternPtr<Labeley>();
    s_func_comp->statements = (func_pre, stuff, func_post);
    stuff->terminus = over;
    stuff->recurse_restriction = xs_rr; // TODO Add support for elsewhere restriction in stuff node, restrict for no reaches
    xs_rr->pattern = MakePatternPtr<Switch>(); // stop it doing a second switch inside one we just created
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

    MakePatternPtr< SlaveSearchReplace<Compound> > lr_sub_slave( lr_func_comp, ll_all );    
    MakePatternPtr< SlaveCompareReplace<Compound> > r_slave( r_func_comp, ls_func_comp, lr_sub_slave );
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
    lr_state_decl->constancy = MakePatternPtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakePatternPtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (ls_label->identifier);
    lr_func_comp->statements = (l_func_pre, l_stuff, l_func_post);
    l_over->overlay = lr_switch_comp;
    lr_switch_comp->statements = (l_pre, lr_case, /*ls_label,*/ l_post); 
    // TODO retain the label for direct gotos, BUT we are spinning at 1st slave because we think the
    // label is still a state, because we think it reaches state var, because the LABEL->STATE_LABEL change 
    // is not being seen by IsLabelReached, because this is done by 2nd slave and stays in temps until
    // the master's SingleCompareReplace() completes. Uh-oh!
    // Only fix after fixing S&R to ensure slave output goes into context
    lr_case->value = lr_state_id;

    ll_all->patterns = (ll_any, lls_not1, lls_not2);
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = lr_state_id;
    lls_not1->pattern = lls_goto;
    lls_goto->destination = ls_label_id; // leave gotos alone in the body
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    Configure( fn );    
}


SwitchCleanUp::SwitchCleanUp()
{
    MakePatternPtr<Compound> r_comp, s_body, r_body;
    MakePatternPtr<Switch> s_switch, r_switch;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > main, tail;
    MakePatternPtr<Label> label;
    MakePatternPtr<Expression> cond;
    MakePatternPtr< NotMatch<Statement> > sx_not_tail, sx_not_main;
    MakePatternPtr< MatchAny<Statement> > sx_any_tail;

    s_switch->condition = cond;
    s_switch->body = s_body;
    s_body->members = decls;
    s_body->statements = (main, label, tail);
    main->pattern = sx_not_main;
    sx_not_main->pattern = MakePatternPtr<Break>();
    tail->pattern = sx_not_tail;
    sx_not_tail->pattern = sx_any_tail;
    sx_any_tail->patterns = (MakePatternPtr<Break>(), MakePatternPtr<Case>());
    
    r_comp->statements = (r_switch, label, tail);
    r_switch->condition = cond;
    r_switch->body = r_body;
    r_body->members = decls;
    r_body->statements = (main);    
    
    Configure( s_switch, r_comp );        
}


InferBreak::InferBreak()
{
    MakePatternPtr<Goto> ls_goto;
    MakePatternPtr<Label> label;
    MakePatternPtr<LabelIdentifier> label_id;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Breakable> breakable;
    MakePatternPtr<Break> lr_break;
    MakePatternPtr<Compound> s_comp, r_comp;

    ls_goto->destination = label_id;

    MakePatternPtr< SlaveSearchReplace<Breakable> > slave( breakable, ls_goto, lr_break ); // todo restrict to not go through more breakables

    s_comp->members = decls;
    s_comp->statements = (pre, breakable, label, post);
    label->identifier = label_id;
    
    r_comp->members = decls;
    r_comp->statements = (pre, slave, post); 
    
    Configure( s_comp, r_comp );        
}


FixFallthrough::FixFallthrough()
{
    // don't actually need a switch statement here, just look in the body, pattern contains Case statements
    MakePatternPtr<Compound> s_comp, r_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, cb1, cb2, post;
    MakePatternPtr<Case> case1, case2;
    MakePatternPtr<Break> breakk;
    MakePatternPtr< NotMatch<Statement> > s_not1, s_not2;
    
    s_comp->members = (decls);
    s_comp->statements = (pre, case1, cb1,              case2, cb2, breakk, post);
    r_comp->members = (decls);
    r_comp->statements = (pre, case1, cb1, cb2, breakk, case2, cb2, breakk, post);
    cb1->pattern = s_not1;
    s_not1->pattern = MakePatternPtr<Break>();
    cb2->pattern = s_not2;
    s_not2->pattern = MakePatternPtr<Case>();
        
    Configure( s_comp, r_comp );            
}

#if 1
MakeFallThroughMachine::MakeFallThroughMachine()
{
    MakePatternPtr<Scope> module;
    MakePatternPtr< Insert<Declaration> > insert;
    MakePatternPtr< GreenGrass<Type> > gg;
    MakePatternPtr<Field> func, m_func;
    MakePatternPtr<InstanceIdentifier> func_id;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Overlay<Compound> > func_over;    
    MakePatternPtr<Compound> s_func_comp, r_func_comp;
    MakePatternPtr< Star<Declaration> > func_decls, module_decls;
    MakePatternPtr< Star<Statement> > func_stmts;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Enum> r_module_enum;
    MakePatternPtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakePatternPtr< MatchAll<Scope> > m_all;
    MakePatternPtr< Stuff<Scope> > m_stuff_inst;
    MakePatternPtr< Stuff<Initialiser> > m_stuff_label;
    MakePatternPtr< Overlay<Type> > m_over;
    MakePatternPtr< Stuff<Type> > m_stuff;
    MakePatternPtr<Instance> m_inst;
    MakePatternPtr<InstanceIdentifier> m_inst_id, var_id;
    MakePatternPtr<TypeIdentifier> module_id;
    MakePatternPtr<Label> m_label;
    MakePatternPtr<IsLabelReached> m_ilr;
    MakePatternPtr< Compound > l_func_comp, lr_comp, ls_comp, lr_if_comp;
    MakePatternPtr< Star<Declaration> > l_func_decls, l_enum_vals, l_decls, l_module_decls;
    MakePatternPtr< Star<Statement> > l_func_pre, l_func_post, l_pre, l_block, l_post, l_stmts, l_dead_gotos;
    MakePatternPtr<Switch> l_switch;     
    MakePatternPtr<Enum> l_enum;     
    MakePatternPtr< Insert<Declaration> > l_insert;
    MakePatternPtr< NotMatch<Statement> > xs_rr;
    MakePatternPtr<Static> lr_state_decl;    
    MakePatternPtr<BuildInstanceIdentifier> lr_state_id("%s_STATE_%s");
    MakePatternPtr<Case> lr_case;
    MakePatternPtr<Signed> lr_int;
    MakePatternPtr<BuildContainerSize> lr_count;
    MakePatternPtr<LabelIdentifier> ls_label_id;
    MakePatternPtr<Instance> var_decl, l_var_decl;
    MakePatternPtr< MatchAll<Node> > ll_all;
    MakePatternPtr< NotMatch<Node> > lls_not1, lls_not2;    
    MakePatternPtr< AnyNode<Node> > ll_any;
    MakePatternPtr< Overlay<Node> > ll_over;
    MakePatternPtr<Goto> lls_goto;    
    MakePatternPtr<Label> lls_label;    
    MakePatternPtr<Goto> ls_goto;   
    MakePatternPtr<Label> ls_label; 
    MakePatternPtr<If> lr_if;            
    MakePatternPtr<Equal> lr_equal;
    MakePatternPtr<Loop> l_loop;
    MakePatternPtr< Overlay<Statement> > l_over;
    MakePatternPtr< NotMatch<Statement> > l_not;             
    MakePatternPtr< Stuff<Scope> > m_stuff_func;
    MakePatternPtr<Scope> l_module;
    MakePatternPtr<Field> l_func;
        
    ll_all->patterns = (ll_any, lls_not1, lls_not2); // TODO don't think we need the nots
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = lr_state_id;
    lls_not1->pattern = lls_goto;
    lls_goto->destination = ls_label_id; // leave gotos alone in the body
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    MakePatternPtr< SlaveSearchReplace<Scope> > slavell( l_module, ll_all );    
    
    m_all->patterns = (m_stuff_func, m_stuff_inst);
    m_stuff_func->terminus = m_func;
    m_func->identifier = func_id;
    m_func->initialiser = m_stuff_label;
    m_stuff_label->terminus = m_label;
    m_label->identifier = m_ilr;
    m_ilr->pattern = m_inst_id;
    m_stuff_inst->terminus = m_inst;
    m_inst->identifier = m_inst_id;
    m_inst->type = m_stuff;
    m_stuff->terminus = m_over;    
    m_over->through = MakePatternPtr<Labeley>();
    m_over->overlay = r_enum_id;    
    
    MakePatternPtr< SlaveCompareReplace<Scope> > slavem( module, m_all );

    l_func->identifier = func_id;
    l_func->initialiser = l_func_comp;
    l_func_comp->members = (func_decls);
    l_func_comp->statements = (l_func_pre, l_loop, l_func_post); 
    l_loop->body = l_over;
    l_over->through = ls_comp;
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_pre, ls_goto, ls_label, l_block, l_post, l_dead_gotos); 
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    l_enum->members = (l_enum_vals, l_insert);
    l_enum->identifier = r_enum_id;
    l_block->pattern = l_not;
    l_not->pattern = MakePatternPtr<Goto>();
    l_post->pattern = MakePatternPtr<If>();    
    l_dead_gotos->pattern = MakePatternPtr<Goto>();
    l_module->members = (l_module_decls, l_enum, l_func);
    l_over->overlay = lr_comp;
    l_insert->insert = (lr_state_decl);
    lr_state_decl->constancy = MakePatternPtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakePatternPtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (func_id, ls_label->identifier);
    lr_comp->members = (l_decls);
    lr_comp->statements = (l_pre, lr_if, l_post); // do final block first
    lr_if->condition = lr_equal;
    lr_if->body = lr_if_comp;
    lr_if->else_body = MakePatternPtr<Nop>();
    lr_equal->operands = (ls_label_id, var_id);
    //lr_if_comp->members = ();
    lr_if_comp->statements = l_block;

    MakePatternPtr< SlaveCompareReplace<Scope> > slavel( slavem, l_module, slavell );
    
    //s_module->bases = (bases);
    //s_module->identifier = module_id;
    func->type = gg;
    gg->through = thread;
    func->identifier = func_id;
    module->members = (module_decls, func, insert);
    insert->insert = (r_module_enum);
    //r_module->bases = (bases);
    //r_module->identifier = module_id;
    r_module_enum->identifier = r_enum_id;
    r_enum_id->sources = (func_id);            
    //r_module_enum->members = ();    
    
    Configure( module, slavel );    
}
#else
MakeFallThroughMachine::MakeFallThroughMachine()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Callable> sub;
    MakePatternPtr< Overlay<Compound> > func_over, over;
    MakePatternPtr< Compound > ls_func_comp, lr_func_comp, s_func_comp, r_func_comp, s_comp, r_comp, r_switch_comp, lr_comp, ls_comp, lr_if_comp;
    MakePatternPtr< Star<Declaration> > func_decls, l_func_decls, decls, l_enum_vals, l_decls;
    MakePatternPtr< Star<Statement> > func_pre, func_post, stmts, body, l_func_pre, l_func_post, l_pre, l_block, l_post, l_stmts;
    MakePatternPtr< Stuff<Statement> > stuff;
    MakePatternPtr<Goto> s_first_goto; 
    MakePatternPtr<Switch> r_switch, l_switch;     
    MakePatternPtr<Enum> r_enum, ls_enum, lr_enum;         
    MakePatternPtr< NotMatch<Statement> > s_prenot, s_postnot, xs_rr;
    MakePatternPtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakePatternPtr<Static> lr_state_decl;    
    MakePatternPtr<BuildInstanceIdentifier> lr_state_id("STATE_%s");
    MakePatternPtr<Case> lr_case;
    MakePatternPtr<Signed> lr_int;
    MakePatternPtr<BuildContainerSize> lr_count;
    MakePatternPtr<LabelIdentifier> ls_label_id;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<Instance> var_decl, l_var_decl;
    MakePatternPtr< Overlay<Type> > var_over;  
    MakePatternPtr<Label> xs_pre_label;
    MakePatternPtr<IsLabelReached> xs_pre_reach;
    MakePatternPtr< MatchAll<Node> > ll_all;
    MakePatternPtr< NotMatch<Node> > lls_not1, lls_not2;    
    MakePatternPtr< AnyNode<Node> > ll_any;
    MakePatternPtr< Overlay<Node> > ll_over;
    MakePatternPtr<Goto> lls_goto;    
    MakePatternPtr<Label> lls_label;    
    MakePatternPtr<Goto> ls_goto;   
    MakePatternPtr<Label> ls_label; 
    MakePatternPtr<If> lr_if;            
    MakePatternPtr<Equal> lr_equal;
    MakePatternPtr<Loop> l_loop;
    MakePatternPtr< Overlay<Statement> > l_over;
    MakePatternPtr< NotMatch<Statement> > l_not;             
    MakePatternPtr< MatchAny<Statement> > l_any;
                             
    MakePatternPtr< SlaveSearchReplace<Compound> > lr_sub_slave( lr_func_comp, ll_all );    
    MakePatternPtr< SlaveCompareReplace<Compound> > r_slave( r_func_comp, ls_func_comp, lr_sub_slave );

    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;
    
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls, var_decl);
    s_func_comp->statements = (stmts);
    var_decl->type = var_over;
    var_decl->identifier = var_id;
    var_over->through = MakePatternPtr<Labeley>();
    var_over->overlay = r_enum_id;

    func_over->overlay = r_slave;
    r_func_comp->members = (func_decls, r_enum, var_decl);
    r_func_comp->statements = (stmts);
    r_enum->identifier = r_enum_id;
    r_enum_id->sources = (fn_id);  
    
    ls_func_comp->members = (func_decls, ls_enum, l_var_decl);
    ls_func_comp->statements = (l_func_pre, l_loop, l_func_post); 
    ls_enum->members = (l_enum_vals);
    ls_enum->identifier = r_enum_id; // need to match id, not enum itself, because enum's members will change during slave
    l_loop->body = l_over;
    l_over->through = ls_comp;
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_pre, ls_goto, ls_label, l_block, l_post); 
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    l_block->pattern = l_not;
    l_not->pattern = l_any;
    l_any->patterns = (MakePatternPtr<Goto>(), MakePatternPtr<If>());
    l_post->pattern = MakePatternPtr<If>();
    
    lr_func_comp->members = (func_decls, lr_enum, l_var_decl);
    lr_func_comp->statements = (l_func_pre, l_loop, l_func_post); // do final block first
    l_over->overlay = lr_comp;
    lr_enum->members = (l_enum_vals, lr_state_decl);
    lr_enum->identifier = r_enum_id;
    lr_state_decl->constancy = MakePatternPtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakePatternPtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (ls_label->identifier);
    lr_comp->members = (l_decls);
    lr_comp->statements = (l_pre, lr_if, l_post); // do final block first
    lr_if->condition = lr_equal;
    lr_if->body = lr_if_comp;
    lr_if->else_body = MakePatternPtr<Nop>();
    lr_equal->operands = (ls_label_id, var_id);
    //lr_if_comp->members = ();
    lr_if_comp->statements = l_block;

    ll_all->patterns = (ll_any, lls_not1, lls_not2); // TODO don't think we need the nots
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = lr_state_id;
    lls_not1->pattern = lls_goto;
    lls_goto->destination = ls_label_id; // leave gotos alone in the body
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    Configure( fn );    
}
#endif



AddYieldFlag::AddYieldFlag()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Callable> sub;
    MakePatternPtr<Compound> s_func_comp, r_func_comp, s_comp, r_comp, ms_comp, mr_comp, msx_comp;
    MakePatternPtr< Star<Declaration> > enums, decls, func_decls, m_decls, msx_decls;
    MakePatternPtr<Instance> var_decl;
    MakePatternPtr<InstanceIdentifier> var_id;    
    MakePatternPtr<TypeIdentifier> enum_id;
    MakePatternPtr< Star<Statement> > func_pre, m_pre, msx_pre, m_post, msx_post, stmts;
    MakePatternPtr< Star<If> > l_pre, l_post;
    MakePatternPtr<Loop> loop;
    MakePatternPtr<If> ls_if, lr_if, ms_if, mr_if;
    MakePatternPtr<Wait> m_wait;
    MakePatternPtr<Enum> enum_decl;
    MakePatternPtr<Equal> l_equal;
    MakePatternPtr<LogicalAnd> lr_and;
    MakePatternPtr<LogicalNot> lr_not;
    MakePatternPtr< Overlay<Compound> > func_over, over;
    MakePatternPtr<Temporary> r_flag_decl;
    MakePatternPtr<Assign> r_flag_init, mr_assign, msx_assign;
    MakePatternPtr<BuildInstanceIdentifier> r_flag_id("yield_flag");
    MakePatternPtr< MatchAll<Compound> > ms_all;
    MakePatternPtr< NotMatch<Compound> > ms_not;
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slavem( r_func_comp, ms_all, mr_comp );
    MakePatternPtr< SlaveSearchReplace<Compound> > slave( r_comp, ls_if, lr_if );  
      
    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;  
    func_over->through = s_func_comp;    
    s_func_comp->members = (func_decls);
    s_func_comp->statements = (func_pre, loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = decls;
    s_comp->statements = (stmts);
    stmts->pattern = MakePatternPtr<If>(); // anti-spin
    func_over->overlay = slavem; 
    r_func_comp->members = (func_decls);
    r_flag_init->operands = (r_flag_id, MakePatternPtr<False>());
    r_func_comp->statements = (func_pre, loop);
    r_flag_decl->identifier = r_flag_id;
    r_flag_decl->type = MakePatternPtr<Boolean>();
    r_flag_decl->initialiser = MakePatternPtr<Uninitialised>();
    over->overlay = slave;
    r_comp->members = (decls, r_flag_decl);
    r_comp->statements = (r_flag_init, stmts);

    ls_if->condition = l_equal;
    l_equal->operands = (MakePatternPtr<InstanceIdentifier>(), MakePatternPtr<InstanceIdentifier>());
    // TODO yield_id should be of type enum_id?                         
    lr_if->condition = lr_and;
    lr_and->operands = (l_equal, lr_not);
    lr_not->operands = (r_flag_id);

    ms_all->patterns = (ms_comp, ms_not);
    ms_comp->members = (m_decls);
    ms_comp->statements = (m_pre, m_wait, m_post);
    ms_not->pattern = msx_comp;
    msx_comp->members = msx_decls;
    msx_comp->statements = (msx_pre, msx_assign, msx_post);
    msx_assign->operands = (r_flag_id, MakePatternPtr<Bool>());

    mr_comp->members = (m_decls);
    mr_comp->statements = (m_pre, m_wait, mr_assign, m_post);
    mr_assign->operands = (r_flag_id, MakePatternPtr<True>());

    Configure( fn );            
}

AddInferredYield::AddInferredYield()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    MakePatternPtr<Compound> func_comp, s_comp, sx_comp, r_comp;
    MakePatternPtr< Star<Declaration> > func_decls;
    MakePatternPtr< Star<Statement> > func_pre, stmts, sx_pre;    
    MakePatternPtr< Overlay<Statement> > over;    
    MakePatternPtr<LocalVariable> flag_decl; 
    MakePatternPtr<InstanceIdentifier> flag_id;   
    MakePatternPtr<WaitDelta> r_yield;
    MakePatternPtr<Loop> loop;
    MakePatternPtr<If> r_if, sx_if;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Compound> > s_notmatch;
    MakePatternPtr< LogicalNot > r_not, sx_not;
    MakePatternPtr< Assign > assign;
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    flag_decl->type = MakePatternPtr<Boolean>();
    flag_decl->initialiser = MakePatternPtr<Uninitialised>();
    flag_decl->identifier = flag_id;
    func_comp->statements = (func_pre, loop);
    loop->body = over;
    over->through = s_all;
    s_all->patterns = (s_comp, s_notmatch);
    s_comp->members = (flag_decl);
    s_comp->statements = (stmts);
    s_notmatch->pattern = sx_comp;
    sx_comp->members = (flag_decl);
    sx_comp->statements = (sx_pre, sx_if);
    sx_if->condition = sx_not;
    sx_not->operands = (flag_id);
    
    over->overlay = r_comp;
    r_comp->members = (flag_decl);
    r_comp->statements = (stmts, r_if);
    r_if->condition = r_not;
    r_not->operands = (flag_id);
    r_if->body = r_yield;
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure( fn );            
}


MoveInitIntoSuperLoop::MoveInitIntoSuperLoop()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce SC stuff
    MakePatternPtr< Star<Declaration> > func_decls;
    MakePatternPtr< Star<Statement> > inits, stmts;    
    MakePatternPtr<Loop> loop;
    MakePatternPtr<Compound> s_func_comp, r_func_comp, s_comp, r_comp, r_if_comp;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Equal> r_equal;
    MakePatternPtr< Overlay<Compound> > func_over, over;    
    MakePatternPtr<Statement> first_init;
                    
    fn->type = thread;
    fn->initialiser = func_over;
    fn->identifier = fn_id;  
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls);
    s_func_comp->statements = (first_init, inits, loop);
    loop->body = over;
    over->through = s_comp;
//    s_comp->members = ();
    s_comp->statements = (stmts);    
    
    func_over->overlay = r_func_comp;
    r_func_comp->members = (func_decls);
    r_func_comp->statements = (loop);
    over->overlay = r_comp;
//    r_comp->members = ();
    r_comp->statements = (r_if, stmts);
    r_if->condition = r_equal;
    r_equal->operands = ( MakePatternPtr<DeltaCount>(), MakePatternPtr<SpecificInteger>(0) );    
    r_if->body = r_if_comp;
//    r_if_comp->members = ();
    r_if_comp->statements = (first_init, inits);//, MakePatternPtr<WaitDelta>());
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure( fn );            
}


// rotate loops to avoid inferred yields when an explicit yield already exists
LoopRotation::LoopRotation()
{
    MakePatternPtr<Instance> fn, s_var_decl;
    MakePatternPtr<InstanceIdentifier> fn_id, s_var_id, s_cur_enum_id, s_outer_enum_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce SC stuff
    MakePatternPtr< Star<Declaration> > func_decls, decls, s_enums;
    MakePatternPtr<Static> s_cur_enum, s_outer_enum;
    MakePatternPtr< Star<Statement> > inits, stmts, pre, post, prepre, prepost, postpre, postpost;    
    MakePatternPtr<Loop> loop;
    MakePatternPtr<Compound> func_comp, s_comp_loop, s_comp_yield, r_comp, r_if_comp, sx_comp;
    MakePatternPtr<If> loop_top, loop_bottom, yield, outer_bottom, outer_top;
    MakePatternPtr< Star<If> > loop_body, pre_yield, post_yield;
    MakePatternPtr<Equal> r_equal;
    MakePatternPtr< Overlay<Compound> > func_over, over;    
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr<Enum> s_enum;
    MakePatternPtr<TypeIdentifier> s_enum_id;
    MakePatternPtr< Stuff<Expression> > loop_top_stuff, outer_top_stuff;
    MakePatternPtr<Equal> loop_top_equal, outer_top_equal;
    MakePatternPtr< Stuff<Statement> > loop_bottom_stuff_enum, outer_bottom_stuff_enum, 
                                    loop_bottom_stuff_noyield, yield_stuff, outer_bottom_stuff_noyield;
    MakePatternPtr< MatchAll<Statement> > loop_bottom_matchall, outer_bottom_matchall;
    MakePatternPtr< NotMatch<Statement> > loop_bottom_notmatch, outer_bottom_notmatch;
    MakePatternPtr< NotMatch<Compound> > s_notmatch;
    MakePatternPtr< MatchAny<If> > inner_state;
    
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls, /*s_enum,*/ s_var_decl); // enum removed because it is class member, not local to the function
    s_enum->identifier = s_enum_id;
    s_enum->members = (s_enums, s_cur_enum);
    s_cur_enum->identifier = s_cur_enum_id;
    s_outer_enum->identifier = s_outer_enum_id;
    s_var_decl->type = s_enum_id;
    s_var_decl->identifier = s_var_id;
    func_comp->statements = (inits, loop);
    loop->body = over;
    over->through = s_all;
    s_all->patterns = (s_comp_loop, s_comp_yield, s_notmatch);
    s_comp_loop->members = (decls);
    // Search for a loop. Assume that a state enum value in a body means "could transition to the state" and one in
    // the condition means "acts on the state". If we see the latter with the former blow it somewhere, we call
    // it a loop and assume the upward branch is normally takner as with C compilers.    
    s_comp_loop->statements = (pre, loop_top, loop_body, loop_bottom, post);    
    loop_top->condition = loop_top_stuff;
    loop_top_stuff->terminus = loop_top_equal;
    loop_top_equal->operands = (s_var_id, s_cur_enum_id);
    loop_bottom->body = loop_bottom_matchall;
    loop_bottom_matchall->patterns = (loop_bottom_stuff_enum, loop_bottom_notmatch);
    loop_bottom_stuff_enum->terminus = s_cur_enum_id;
    loop_bottom_notmatch->pattern = loop_bottom_stuff_noyield;
    loop_bottom_stuff_noyield->terminus = MakePatternPtr<Wait>();    
    s_comp_yield->members = (decls);
    
    // We need to restruct to loops that contain a yield anywhere but the bottom - these are the ones
    // that would benefit from loop rotation.
    s_comp_yield->statements = (pre, pre_yield, yield, post_yield, loop_bottom, post);    
    yield->body = yield_stuff;
    yield_stuff->terminus = MakePatternPtr<Wait>();
    
    // This part is to make sure we operate on the outermost loop first - look for another loop surrounding 
    // the current one, if it does not end in a yield, then do not transform. This way the outer loop will
    // keep getting hits until the yield is at the bottom, then inner loops can have a go.
    s_notmatch->pattern = sx_comp;
    sx_comp->members = (decls);
    sx_comp->statements = (prepre, outer_top, postpre, inner_state, prepost, outer_bottom, postpost);
    outer_top->condition = outer_top_stuff;
    outer_top_stuff->terminus = outer_top_equal;
    outer_top_equal->operands = (s_var_id, s_outer_enum_id);
    outer_bottom->body = outer_bottom_matchall;
    outer_bottom_matchall->patterns = (outer_bottom_stuff_enum, outer_bottom_notmatch);
    outer_bottom_stuff_enum->terminus = s_outer_enum_id;
    outer_bottom_notmatch->pattern = outer_bottom_stuff_noyield;
    outer_bottom_stuff_noyield->terminus = MakePatternPtr<Wait>();    
    inner_state->patterns = (loop_top, loop_bottom); // outer loop can share top or bottom state with inner loop; but not both, so at least one must be here
   
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (pre, loop_bottom, loop_top, loop_body, post);    // rotated version of s_comp_loop
        
    Configure( fn );            
}


