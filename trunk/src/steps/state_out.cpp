
#include "steps/state_out.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/typeof.hpp"

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
}

