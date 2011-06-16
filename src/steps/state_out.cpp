
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
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< Multiplexor > mult;
    
    s_then_goto->destination = MakeTreePtr<Expression>();    
    s_else_goto->destination = MakeTreePtr<Expression>();
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakeTreePtr<Nop>(); // standed conditional branch has no else clause - our "else" is the next statement
    s_comp->statements = ( pre, s_if, s_else_goto, post );    

    mult->operands = (cond, s_then_goto->destination, s_else_goto->destination);
    r_goto->destination = mult;
    r_comp->statements = ( pre, r_goto, post );
    
    SearchReplace::Configure( s_comp, r_comp );
}

CompactGotosFinal::CompactGotosFinal()
{
    MakeTreePtr< If > s_if;      
    MakeTreePtr< Expression > cond;      
    MakeTreePtr< Compound > s_comp, r_comp;  
    MakeTreePtr< Goto > s_then_goto, s_else_goto, r_goto;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< Multiplexor > mult;
    MakeTreePtr< Label > label;    
    MakeTreePtr< BuildLabelIdentifier > label_id("NOJUMP");
    
    s_then_goto->destination = MakeTreePtr<Expression>();    
    s_if->condition = cond;
    s_if->body = s_then_goto;
    s_if->else_body = MakeTreePtr<Nop>(); // standard conditional branch has no else clause - our "else" is the next statement
    s_comp->statements = ( pre, s_if, post );    

    label->identifier = label_id;
    mult->operands = (cond, s_then_goto->destination, label_id);
    r_goto->destination = mult;
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

    SearchReplace::Configure( fn, fn );
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

    SearchReplace::Configure( fn, fn );
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

    SearchReplace::Configure( loop, loop );
}


