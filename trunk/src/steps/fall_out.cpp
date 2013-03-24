
#include "steps/fall_out.hpp"
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


// Something to get the size of the Collection matched by a Star as a SpecificInteger
struct BuildContainerSize : CompareReplace::SoftReplacePattern,
                            Special<Integer>
{
    SPECIAL_NODE_FUNCTIONS
    shared_ptr< StarBase > container;
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

// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : Label
{
    NODE_FUNCTIONS_FINAL
    TreePtr<InstanceIdentifier> state;
};



PlaceLabelsInArray::PlaceLabelsInArray()
{
    MakeTreePtr<Scope> module;
    MakeTreePtr< Insert<Declaration> > insert;
    MakeTreePtr< GreenGrass<Type> > gg;
    MakeTreePtr<Field> func, m_func;
    MakeTreePtr<InstanceIdentifier> func_id;
    MakeTreePtr<Thread> thread;
    MakeTreePtr< Star<Declaration> > func_decls, module_decls;
    MakeTreePtr< Star<Statement> > func_stmts;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr<Enum> r_module_enum;
    MakeTreePtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr<TypeIdentifier> module_id;
    MakeTreePtr< Star<Declaration> > l_func_decls, l_enum_vals, l_decls, l_module_decls;
    MakeTreePtr< Star<Statement> > l_func_pre, l_func_post, l_pre, l_block, l_post, l_stmts, l_dead_gotos;
    MakeTreePtr<Switch> l_switch;     
    MakeTreePtr<Enum> l_enum;     
    MakeTreePtr< Insert<Declaration> > l_insert;
    MakeTreePtr< NotMatch<Statement> > xs_rr;
    MakeTreePtr<Static> lr_state_decl;    
    MakeTreePtr<BuildInstanceIdentifier> lr_state_id("%s_STATE_%s");
    MakeTreePtr<Case> lr_case;
    MakeTreePtr<Signed> lr_int;
    MakeTreePtr<BuildContainerSize> lr_count;
    MakeTreePtr<LabelIdentifier> ls_label_id;
    MakeTreePtr<Instance> var_decl, l_var_decl;
    MakeTreePtr< MatchAll<Node> > ll_all;
    MakeTreePtr< NotMatch<Node> > lls_not1, lls_not2;    
    MakeTreePtr< AnyNode<Node> > ll_any;
    MakeTreePtr< Overlay<Node> > ll_over;
    MakeTreePtr<Goto> lls_goto;    
    MakeTreePtr<Label> lls_label;    
    MakeTreePtr<Goto> ls_goto;   
    MakeTreePtr<Label> ls_label, l_label; 
    MakeTreePtr<If> lr_if;            
    MakeTreePtr<Equal> lr_equal;
    MakeTreePtr<Loop> l_loop;
    MakeTreePtr< Overlay<Statement> > l_over;
    MakeTreePtr< NotMatch<Statement> > l_not;             
    MakeTreePtr< Stuff<Scope> > m_stuff_func;
    MakeTreePtr<Scope> l_module;
    MakeTreePtr<Field> l_func;
    MakeTreePtr<Static> r_lmap, l_lmap;
    MakeTreePtr<BuildInstanceIdentifier> r_lmap_id("lmap");
    MakeTreePtr<Array> r_array;
    MakeTreePtr<MakeArray> r_make, l_make, lls_make;
    MakeTreePtr< Insert<Expression> > l_minsert;
    MakeTreePtr< Stuff<Statement> > l_stuff;
    MakeTreePtr< Star<Expression> > l_existing;
    MakeTreePtr< NotMatch<Expression> > l_mnot;
    MakeTreePtr< Subscript > ll_sub;
    MakeTreePtr<Compound> comp, l_comp;
    MakeTreePtr< Insert<Declaration> > finsert, l_finsert;
    MakeTreePtr< Overlay<Statement> > l_overll;
    MakeTreePtr< StateLabel > l_state_label;
            
    ll_all->patterns = (ll_any, lls_not1, lls_not2); 
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = ll_sub;
    ll_sub->operands = (r_lmap_id, lr_state_id);
    lls_not1->pattern = lls_make;
    lls_make->operands = (MakeTreePtr< Star<Expression> >(), 
                          ls_label_id,
                          MakeTreePtr< Star<Expression> >()); // TODO too strong, will hit (arr){LABEL} in original code
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    MakeTreePtr< SlaveSearchReplace<Scope> > slavell( l_module, ll_all );    
    
    l_func->identifier = func_id;
    l_func->initialiser = l_comp;
    l_comp->members = (MakeTreePtr< Star<Declaration> >(), l_lmap);
    l_comp->statements = (MakeTreePtr< Star<Statement> >(), l_stuff, MakeTreePtr< Star<Statement> >());
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    l_enum->members = (l_enum_vals, l_insert);
    l_enum->identifier = r_enum_id;
    l_block->pattern = l_not;
    l_not->pattern = MakeTreePtr<Goto>();
    l_post->pattern = MakeTreePtr<If>();    
    l_dead_gotos->pattern = MakeTreePtr<Goto>();
    l_module->members = (l_module_decls, l_enum, l_func);
    l_insert->insert = (lr_state_decl);
    lr_state_decl->constancy = MakeTreePtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = r_enum_id;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_state_id->sources = (func_id, ls_label->identifier);
    l_lmap->identifier = r_lmap_id;
    l_lmap->initialiser = l_make;
    l_make->operands = (l_existing, l_minsert);
    l_existing->pattern = l_mnot;
    l_mnot->pattern = ls_label_id;
    l_minsert->insert = ls_label_id;
    l_label->identifier = ls_label_id;
    l_stuff->terminus = l_overll;
    l_overll->through = l_label;
    l_overll->overlay = l_state_label;
    l_state_label->identifier = ls_label_id;
    l_state_label->state = lr_state_id;

    MakeTreePtr< SlaveCompareReplace<Scope> > slavel( module, l_module, slavell );
    
    //s_module->bases = (bases);
    //s_module->identifier = module_id;
    func->type = gg;
    gg->through = thread;
    func->identifier = func_id;
    func->initialiser = comp;
    comp->members = (MakeTreePtr< Star<Declaration> >(), finsert);
    comp->statements = (MakeTreePtr< Star<Statement> >());
    finsert->insert = (r_lmap);    
    module->members = (module_decls, func, insert);
    insert->insert = (r_module_enum);
    //r_module->bases = (bases);
    //r_module->identifier = module_id;
    r_module_enum->identifier = r_enum_id;
    r_enum_id->sources = (func_id);            
    //r_module_enum->members = ();
    r_lmap->type = r_array;
    r_lmap->identifier = r_lmap_id;
    r_lmap->initialiser = r_make;    
    r_lmap->constancy = MakeTreePtr<Const>();        
//    r_lmap->virt = MakeTreePtr<NonVirtual>();
  //  r_lmap->access = MakeTreePtr<Private>();    
    r_array->element = MakeTreePtr<Labeley>();
    r_array->size = MakeTreePtr<Uninitialised>();
    //r_make->operands = ()
    
    Configure( module, slavel );    
}

// New better way of propogating lmap through variables. This supercedes LabelVarsToEnum
// and SwapSubscriptMultiplex. It works by just changing all appearances of the Labeley 
// type (except in the decl of the lvar). 
// TODO Use local node for enum, so that we can change to this, and not unsigned int
// TODO does not handle the case where there are two threads in a module, one of 
// hwich does not have a state variable. Master can hit due to a thread that does have 
// a "state" variable, but will choose lmap randomly from either thread and only
// modify the usages of *that* lmap. Since there is now no state varaible in the 
// module, no more updates to usages of lmap will occur. The answer is to recode the 
// step based on pinning down a module and a compund within, and tying everything 
// together properly.
LabelTypeToEnum::LabelTypeToEnum()
{
    MakeTreePtr< Stuff<Scope> > stuff_labeley, stuff_lmap;
    MakeTreePtr<Labeley> labeley;
    MakeTreePtr<Static> lmap;
    MakeTreePtr<Const> lmap_const;
    MakeTreePtr<Array> lmap_type;
    MakeTreePtr<InstanceIdentifier> lmap_id; 
    MakeTreePtr< MatchAll<Node> > apall, l_apall;
    MakeTreePtr< NotMatch<Node> > apnot, l_apnot;
    MakeTreePtr< AnyNode<Node> > apany, l_apany;
    MakeTreePtr< Overlay<Type> > l_over;
    MakeTreePtr<Subscript> ms_sub, nr_sub, nsx_sub;;
    MakeTreePtr<InstanceIdentifier> m_state_id;
    MakeTreePtr<Goto> ns_goto, nr_goto;
    MakeTreePtr< NotMatch<Expression> > n_dest_expr;
    MakeTreePtr<Unsigned> l_enum; // TODO use the real enum!!
    MakeTreePtr< MatchAll<Scope> > all;
    MakeTreePtr<Record> record;
    MakeTreePtr< Star<Declaration> > decls;
    
    record->members = ( decls );

    l_apall->patterns = (l_apany, l_apnot);
    l_apnot->pattern = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakeTreePtr<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakeTreePtr<SpecificInteger>(32);
            
    MakeTreePtr< SlaveSearchReplace<Scope> > slavel( record, l_apall );   

    ms_sub->operands = (lmap_id, m_state_id);
    
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_sub, m_state_id );   

    ns_goto->destination = n_dest_expr;
    nr_goto->destination = nr_sub;
    nr_sub->operands = (lmap_id, n_dest_expr);
    n_dest_expr->pattern = nsx_sub;
    nsx_sub->operands = (lmap_id, MakeTreePtr<Expression>());
    
    MakeTreePtr< SlaveSearchReplace<Scope> > slaven( slavem, ns_goto, nr_goto );   
    
    all->patterns = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakeTreePtr<Labeley>();
    stuff_labeley->terminus = apall;
    apall->patterns = (apany, apnot);
    apany->terminus = labeley;
    apnot->pattern = lmap;
    
    Configure( all, slaven );
}


LabelVarsToEnum::LabelVarsToEnum()
{
    MakeTreePtr< MatchAll<Scope> > s_all;
    MakeTreePtr< MatchAll<Statement> > sx_all;
    MakeTreePtr< Stuff<Scope> > scope;
    MakeTreePtr<Instance> var;
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr< Stuff<Scope> > s_stuff, sx_stuff;
    MakeTreePtr<Assign> s_assign, sx_assign, l_assign;
    MakeTreePtr<Subscript> s_sub, sx_sub, ls_sub, mr_sub, msx_sub;
    MakeTreePtr< NotMatch<Scope> > sx_not1;
    MakeTreePtr<AssignmentOperator> sx_asop, ms_asop;
    MakeTreePtr< NotMatch<Statement> > sx_not2, msx_not, msx_not2, msx_not3;
    MakeTreePtr< Overlay<Type> > over;
    MakeTreePtr< TransformOf<Expression> > s_index( &TypeOf::instance );
    MakeTreePtr<TypeIdentifier> type; 
    MakeTreePtr< Overlay<Expression> > l_over, m_over;
    MakeTreePtr<Expression> l_index;
    MakeTreePtr< MatchAll<Node> > ms_all;
    MakeTreePtr< AnyNode<Node> > ms_anynode;    
    MakeTreePtr<NestedArray> nested_array;
    MakeTreePtr<NestedSubscriptLookup> nested_subscript, nested_subscript2, nested_subscript3;
    MakeTreePtr<NestedSubscriptLookup> l_nested_subscript, m_nested_subscript, m_nested_subscript2;
    MakeTreePtr<Instance> msx_inst;
    MakeTreePtr<String> depth;
    MakeTreePtr< Stuff<Scope> > lmap_stuff;
    MakeTreePtr<Static> lmap;
    MakeTreePtr<Const> lmap_const;
    MakeTreePtr<Array> lmap_type;
    MakeTreePtr<InstanceIdentifier> lmap_id; 
    
    l_assign->operands = (l_nested_subscript, l_over);
    l_nested_subscript->terminus = var_id;    
    l_nested_subscript->depth = depth;
    l_over->through = ls_sub;
    ls_sub->operands = (lmap_id, l_index);
    l_over->overlay = l_index;    
   
    MakeTreePtr< SlaveSearchReplace<Scope> > slavel( scope, l_assign );   
   
    ms_all->patterns = (ms_anynode, msx_not, msx_not2, msx_not3);
    msx_not2->pattern = msx_inst;
    msx_inst->identifier = var_id;    
    msx_not->pattern = ms_asop;
    ms_asop->operands = (m_nested_subscript2, MakeTreePtr< Star<Expression> >() );
    m_nested_subscript2->terminus = var_id;
    m_nested_subscript2->depth = depth;
    ms_anynode->terminus = m_over;
    m_over->through = m_nested_subscript;
    m_nested_subscript->terminus = var_id;
    m_nested_subscript->depth = depth;
    m_over->overlay = mr_sub;
    msx_not3->pattern = msx_sub;
    msx_sub->operands = (lmap_id, m_nested_subscript);
    mr_sub->operands = (lmap_id, m_nested_subscript);
    
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_all, ms_anynode );   
   
    s_all->patterns = (scope, s_stuff, sx_not1, lmap_stuff);
    scope->terminus = var;
    var->type = nested_array;
    nested_array->terminus = over;
    //nested_array->depth = depth;
    over->through = MakeTreePtr<Labeley>();    
    var->identifier = var_id;
    s_stuff->terminus = s_assign;
    s_assign->operands = (nested_subscript, s_sub);
    nested_subscript->terminus = var_id;
    nested_subscript->depth = depth;
    s_sub->operands = (lmap_id, s_index);
    s_index->pattern = type;
    sx_not1->pattern = sx_stuff;
    sx_stuff->terminus = sx_all;
    sx_all->patterns = (sx_asop, sx_not2);
    sx_asop->operands = (nested_subscript3, MakeTreePtr< Star<Expression> >() );
    nested_subscript3->terminus = var_id;
    nested_subscript3->depth = depth;
    sx_not2->pattern = sx_assign;
    sx_assign->operands = (nested_subscript2, sx_sub);
    nested_subscript2->terminus = var_id;
    nested_subscript2->depth = depth;
    sx_sub->operands = (lmap_id, MakeTreePtr<Expression>() );
    over->overlay = type;
    lmap_stuff->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakeTreePtr<Labeley>();
        
    Configure( s_all, slavem );
}


SwapSubscriptMultiplex::SwapSubscriptMultiplex()
{
    MakeTreePtr<Multiplexor> s_mux, r_mux;
    MakeTreePtr<Subscript> s_sub1, s_sub2, r_sub;
    MakeTreePtr<Expression> cond, index1, index2;
    MakeTreePtr<InstanceIdentifier> array; // Instance used to prevent side effects, which would go out of sequence
    
    s_mux->operands = (cond, s_sub1, s_sub2);
    s_sub1->operands = (array, index1);
    s_sub2->operands = (array, index2);
    
    r_sub->operands = (array, r_mux);
    r_mux->operands = (cond, index1, index2);
    
    Configure( s_mux, r_sub );
}


AddStateEnumVar::AddStateEnumVar()
{
    MakeTreePtr<Compound> s_comp, r_comp, lr_compound;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<Goto> ls_goto, lr_goto, s_goto;
    MakeTreePtr<Assign> lr_assign;
    MakeTreePtr<Automatic> state_var;
    MakeTreePtr< NotMatch<Expression> > lx_not;
    MakeTreePtr< BuildInstanceIdentifier > state_var_id("state");
    MakeTreePtr< GreenGrass<Compound> > s_gg;
    MakeTreePtr<Subscript> s_sub, ls_sub, lr_sub;
    MakeTreePtr<InstanceIdentifier> array;
    MakeTreePtr< TransformOf<Expression> > s_index( &TypeOf::instance );
    MakeTreePtr<Integral> type; // TODO use the enum type, and ensure properly supported in TypeOf
    
    ls_goto->destination = ls_sub;
    ls_sub->operands = (array, lx_not);
    lx_not->pattern = state_var_id; //  MakeTreePtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lx_not);
    lr_goto->destination = lr_sub;
    lr_sub->operands = (array, state_var_id);
            
    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave( r_comp, ls_goto, lr_compound );
     
    s_gg->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (pre, s_goto, post); 
    s_goto->destination = s_sub;
    s_sub->operands = (array, s_index);
    s_index->pattern = type;
        
    r_comp->members = (state_var, decls);
    r_comp->statements = (pre, s_goto, post); 
    state_var->identifier = state_var_id;
    state_var->type = type;    
    state_var->initialiser = MakeTreePtr<Uninitialised>();

    Configure( s_gg, r_slave );
}


ApplyCombGotoPolicy::ApplyCombGotoPolicy()
{
    MakeTreePtr<Compound> comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, body, post;
    MakeTreePtr<Goto> s_goto1, goto2, sx_pre_goto;
    MakeTreePtr<Subscript> sub;
    MakeTreePtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakeTreePtr<StateLabel> label;
    MakeTreePtr< NotMatch<Statement> > sx_pre, sx_body;
    MakeTreePtr<Uncombable> sx_uncombable;
    MakeTreePtr< Erase<Statement> > s_erase;
    MakeTreePtr< Overlay<Statement> > over;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Equal> r_equal;   
    
    comp->members = (decls);
    comp->statements = (pre, s_erase, label, over, goto2, post);
    pre->pattern = sx_pre,
    sx_pre->pattern = sx_pre_goto; // ensure we act on the first goto only
    s_erase->erase = s_goto1;
    s_goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    over->through = body;
    body->pattern = sx_body;
    sx_body->pattern = sx_uncombable; 
    goto2->destination = sub;    
    
    over->overlay = r_if;
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakeTreePtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure(comp);
}


ApplyYieldGotoPolicy::ApplyYieldGotoPolicy()
{
    MakeTreePtr<Compound> comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, body1, body2, post;
    MakeTreePtr<Goto> s_goto1, goto2, sx_pre_goto;
    MakeTreePtr<Subscript> sub;
    MakeTreePtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakeTreePtr<StateLabel> label;
    MakeTreePtr< NotMatch<Statement> > sx_pre, sx_body1, sx_body2;
    MakeTreePtr< Erase<Statement> > s_erase, s_erase2;
    MakeTreePtr< Insert<Statement> > r_insert;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Equal> r_equal;   
    MakeTreePtr<Wait> wait;
    MakeTreePtr<Uncombable> sx_uncombable1, sx_uncombable2;
    
    comp->members = (decls);
    comp->statements = (pre, s_erase, label, s_erase2, r_insert, goto2, post);
    pre->pattern = sx_pre,
    sx_pre->pattern = sx_pre_goto; // ensure we act on the first goto only
    s_erase->erase = s_goto1;
    s_goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    s_erase2->erase = (body1, wait, body2);
    goto2->destination = sub;    
    body1->pattern = sx_body1;
    sx_body1->pattern = sx_uncombable1;
    body2->pattern = sx_body2;
    sx_body2->pattern = sx_uncombable2;
        
    r_insert->insert = r_if;
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakeTreePtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, goto2);
    
    Configure(comp);
}


ApplyBottomPolicy::ApplyBottomPolicy()
{
    MakeTreePtr<Compound> comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, body;
    MakeTreePtr<Goto> goto1, sx_pre_goto;
    MakeTreePtr<Subscript> sub;
    MakeTreePtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakeTreePtr<StateLabel> label;
    MakeTreePtr< NotMatch<Statement> > sx_pre, sx_body;
    MakeTreePtr< Erase<Statement> > s_erase;
    MakeTreePtr< Insert<Statement> > r_insert;
    MakeTreePtr< Overlay<Statement> > over;
    MakeTreePtr<If> r_if, r_if2;
    MakeTreePtr<Equal> r_equal;   
    MakeTreePtr<NotEqual> r_not_equal;   
    MakeTreePtr<Uncombable> sx_uncombable;
    
    comp->members = (decls);
    comp->statements = (pre, s_erase, label, over, r_insert);
    pre->pattern = sx_pre,
    sx_pre->pattern = sx_pre_goto; // ensure we act on the first goto only
    s_erase->erase = goto1;
    goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    over->through = body;
    body->pattern = sx_body,
    sx_body->pattern = sx_uncombable; 
    
    over->overlay = r_if;
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakeTreePtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    r_insert->insert = goto1; // r_if2; TODO: with the condition, superloop is exiting before the last state block has run
    r_if2->condition = r_not_equal;
    r_if2->body = goto1;
    r_if2->else_body = MakeTreePtr<Nop>();
    r_not_equal->operands = (state_var_id, state_id);    
    
    Configure(comp);
}


ApplyLabelPolicy::ApplyLabelPolicy()
{
    MakeTreePtr<Compound> comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr<StateLabel> label1;
    MakeTreePtr<Label> label2, sx_post_label;
    MakeTreePtr<If> iif;
    MakeTreePtr<Equal> equal;   
    MakeTreePtr< NotMatch<Statement> > sx_post;
    MakeTreePtr<InstanceIdentifier> state_var_id, state_id;
    MakeTreePtr< Erase<Label> > erase, erase_star;
    MakeTreePtr< Insert<Label> > insert, insert_star;
    MakeTreePtr< Star<Label> > label_star;    
        
    comp->members = (decls);
    comp->statements = (pre, insert, insert_star, label1, iif, erase, erase_star, post);
    label1->state = state_id;
    insert->insert = label2;
    insert_star->insert = label_star;
    iif->condition = equal;
    equal->operands = (state_var_id, state_id);
    erase->erase = label2;
    erase_star->erase = label_star;
    post->pattern = sx_post;
    sx_post->pattern = sx_post_label;
        
    Configure(comp);
}

ApplyTopPolicy::ApplyTopPolicy()
{
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr<Compound> comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > body1, body2, post;
    MakeTreePtr<Label> label, sx_label, sx_label2;
    MakeTreePtr< NotMatch<Statement> > sx_stmt;
    MakeTreePtr< Erase<Statement> > s_erase;
    MakeTreePtr< Insert<Statement> > r_insert;
    MakeTreePtr<If> r_if;
    MakeTreePtr<Equal> r_equal;   
    MakeTreePtr<DeltaCount> r_delta_count;
    MakeTreePtr<SpecificInteger> r_zero(0);
    MakeTreePtr<Wait> wait;
    MakeTreePtr< Stuff<Compound> > s_stuff;
    MakeTreePtr<Goto> gotoo;
    MakeTreePtr< NotMatch<Statement> > sx_body1, sx_body2;
    MakeTreePtr<Uncombable> sx_uncombable1, sx_uncombable2;
       
    s_all->patterns = (comp, s_stuff);
    comp->members = (decls);
    comp->statements = (s_erase, label, r_insert, post);
    s_stuff->terminus = gotoo;
    s_erase->erase = (body1, wait, body2);
    body1->pattern = sx_body1;
    sx_body1->pattern = sx_uncombable1;
    body2->pattern = sx_body2;
    sx_body2->pattern = sx_uncombable2;
    
    r_insert->insert = r_if;
    r_if->condition = r_equal;
    r_equal->operands = (r_delta_count, r_zero);
    r_if->body = r_body_comp;
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, gotoo);
    r_if->else_body = MakeTreePtr<Nop>();
    
    Configure(s_all, comp);
}


EnsureResetYield::EnsureResetYield()
{
    MakeTreePtr<Compound> comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > pre, post;
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr< MatchAny<Statement> > sx_any;
    MakeTreePtr<Goto> gotoo;
    MakeTreePtr< Insert<Statement> > insert;
    MakeTreePtr<WaitDelta> r_yield;
    
    comp->members = (decls);
    comp->statements = (pre, insert, gotoo, post);
    pre->pattern = sx_not;
    sx_not->pattern = sx_any;
    sx_any->patterns = (MakeTreePtr<Goto>(), MakeTreePtr<Label>(), MakeTreePtr<Wait>() );
    
    insert->insert = r_yield;
    
    Configure(comp);
}


DetectSuperLoop::DetectSuperLoop( bool is_conditional_goto )
{
    MakeTreePtr<Instance> inst;
    MakeTreePtr<Compound> s_comp, r_comp, r_body_comp;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Statement> > body;
    MakeTreePtr<Label> s_label;
    MakeTreePtr<If> s_ifgoto;
    MakeTreePtr<Goto> s_goto;
    MakeTreePtr<Expression> cond;
    MakeTreePtr< NotMatch<Statement> > sx_not;
    MakeTreePtr<Do> r_do;
    MakeTreePtr< Overlay<Compound> > over;
    
    MakeTreePtr< SlaveSearchReplace<Statement> > slavell( r_body_comp, MakeTreePtr<Goto>(), MakeTreePtr<Continue>() );    
    
    inst->type = MakeTreePtr<Callable>();
    inst->initialiser = over;
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (s_label, body, is_conditional_goto 
                                         ? TreePtr<Statement>(s_ifgoto) 
                                         : TreePtr<Statement>(s_goto) );
    body->pattern = sx_not;
    sx_not->pattern = MakeTreePtr<Label>(); // so s_label is the only one - all gotos must go to it.
    s_ifgoto->condition = cond;
    s_ifgoto->body = s_goto;
    s_ifgoto->else_body = MakeTreePtr<Nop>();
    
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (r_do);
    r_do->condition = is_conditional_goto 
                      ? cond 
                      : TreePtr<Expression>(MakeTreePtr<True>());
    r_do->body = slavell;
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure(inst);
}


InsertInferredYield::InsertInferredYield()
{
    MakeTreePtr<Instance> fn;
    MakeTreePtr<InstanceIdentifier> fn_id;
    MakeTreePtr<Thread> thread; // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    MakeTreePtr<Compound> func_comp, s_comp, sx_comp, r_comp;
    MakeTreePtr< Star<Declaration> > func_decls, loop_decls;
    MakeTreePtr< Star<Statement> >  stmts, sx_pre;    
    MakeTreePtr< Overlay<Statement> > over;    
    MakeTreePtr<LocalVariable> flag_decl; 
    MakeTreePtr<InstanceIdentifier> flag_id;   
    MakeTreePtr<WaitDelta> r_yield;
    MakeTreePtr<Loop> loop;
    MakeTreePtr<If> r_if, sx_if;
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< NotMatch<Compound> > s_notmatch;
    MakeTreePtr< LogicalNot > r_not, sx_not;
    MakeTreePtr< Assign > assign;
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    func_comp->statements = (loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = (loop_decls);
    s_comp->statements = (stmts);
    stmts->pattern = MakeTreePtr<If>();
    
    over->overlay = r_comp;
    r_comp->members = (loop_decls);
    r_comp->statements = (stmts, r_yield);
    
    Configure( fn );            
}


