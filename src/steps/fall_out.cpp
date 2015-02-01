
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
struct BuildContainerSize : SoftReplacePattern,
                            Special<Integer>
{
    SPECIAL_NODE_FUNCTIONS
    shared_ptr< StarBase > container;
private:
    virtual TreePtr<Node> MyBuildReplace()
    {
        ASSERT( container );
	    TreePtr<Node> n = BuildReplace( container );
	    ASSERT( n );
	    ContainerInterface *n_container = dynamic_cast<ContainerInterface *>(n.get());
	    ASSERT( n_container );
	    int size = n_container->size();
	    TreePtr<SpecificInteger> si = MakePatternPtr<SpecificInteger>(size);
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
    MakePatternPtr<Scope> module;
    MakePatternPtr< Insert<Declaration> > insert;
    MakePatternPtr< GreenGrass<Type> > gg;
    MakePatternPtr<Field> func, m_func;
    MakePatternPtr<InstanceIdentifier> func_id;
    MakePatternPtr<Thread> thread;
    MakePatternPtr< Star<Declaration> > func_decls, module_decls;
    MakePatternPtr< Star<Statement> > func_stmts;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Enum> r_module_enum;
    MakePatternPtr<BuildTypeIdentifier> r_enum_id("%sStates");
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<TypeIdentifier> module_id;
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
    MakePatternPtr<Label> ls_label, l_label; 
    MakePatternPtr<If> lr_if;            
    MakePatternPtr<Equal> lr_equal;
    MakePatternPtr<Loop> l_loop;
    MakePatternPtr< Overlay<Statement> > l_over;
    MakePatternPtr< NotMatch<Statement> > l_not;             
    MakePatternPtr< Stuff<Scope> > m_stuff_func;
    MakePatternPtr<Scope> l_module;
    MakePatternPtr<Field> l_func;
    MakePatternPtr<Static> r_lmap, l_lmap;
    MakePatternPtr<BuildInstanceIdentifier> r_lmap_id("lmap");
    MakePatternPtr<Array> r_array;
    MakePatternPtr<MakeArray> r_make, l_make, lls_make;
    MakePatternPtr< Insert<Expression> > l_minsert;
    MakePatternPtr< Stuff<Statement> > l_stuff;
    MakePatternPtr< Star<Expression> > l_existing;
    MakePatternPtr< NotMatch<Expression> > l_mnot;
    MakePatternPtr< Subscript > ll_sub;
    MakePatternPtr<Compound> comp, l_comp;
    MakePatternPtr< Insert<Declaration> > finsert, l_finsert;
    MakePatternPtr< Overlay<Statement> > l_overll;
    MakePatternPtr< StateLabel > l_state_label;
            
    ll_all->patterns = (ll_any, lls_not1, lls_not2); 
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = ll_sub;
    ll_sub->operands = (r_lmap_id, lr_state_id);
    lls_not1->pattern = lls_make;
    lls_make->operands = (MakePatternPtr< Star<Expression> >(), 
                          ls_label_id,
                          MakePatternPtr< Star<Expression> >()); // TODO too strong, will hit (arr){LABEL} in original code
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    MakePatternPtr< SlaveSearchReplace<Scope> > slavell( l_module, ll_all );    
    
    l_func->identifier = func_id;
    l_func->initialiser = l_comp;
    l_comp->members = (MakePatternPtr< Star<Declaration> >(), l_lmap);
    l_comp->statements = (MakePatternPtr< Star<Statement> >(), l_stuff, MakePatternPtr< Star<Statement> >());
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    l_enum->members = (l_enum_vals, l_insert);
    l_enum->identifier = r_enum_id;
    l_block->pattern = l_not;
    l_not->pattern = MakePatternPtr<Goto>();
    l_post->pattern = MakePatternPtr<If>();    
    l_dead_gotos->pattern = MakePatternPtr<Goto>();
    l_module->members = (l_module_decls, l_enum, l_func);
    l_insert->insert = (lr_state_decl);
    lr_state_decl->constancy = MakePatternPtr<Const>();
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

    MakePatternPtr< SlaveCompareReplace<Scope> > slavel( module, l_module, slavell );
    
    //s_module->bases = (bases);
    //s_module->identifier = module_id;
    func->type = gg;
    gg->through = thread;
    func->identifier = func_id;
    func->initialiser = comp;
    comp->members = (MakePatternPtr< Star<Declaration> >(), finsert);
    comp->statements = (MakePatternPtr< Star<Statement> >());
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
    r_lmap->constancy = MakePatternPtr<Const>();        
//    r_lmap->virt = MakePatternPtr<NonVirtual>();
  //  r_lmap->access = MakePatternPtr<Private>();    
    r_array->element = MakePatternPtr<Labeley>();
    r_array->size = MakePatternPtr<Uninitialised>();
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
    MakePatternPtr< Stuff<Scope> > stuff_labeley, stuff_lmap;
    MakePatternPtr<Labeley> labeley;
    MakePatternPtr<Static> lmap;
    MakePatternPtr<Const> lmap_const;
    MakePatternPtr<Array> lmap_type;
    MakePatternPtr<InstanceIdentifier> lmap_id; 
    MakePatternPtr< MatchAll<Node> > apall, l_apall;
    MakePatternPtr< NotMatch<Node> > apnot, l_apnot;
    MakePatternPtr< AnyNode<Node> > apany, l_apany;
    MakePatternPtr< Overlay<Type> > l_over;
    MakePatternPtr<Subscript> ms_sub, nr_sub, nsx_sub;;
    MakePatternPtr<InstanceIdentifier> m_state_id;
    MakePatternPtr<Goto> ns_goto, nr_goto;
    MakePatternPtr< NotMatch<Expression> > n_dest_expr;
    MakePatternPtr<Unsigned> l_enum; // TODO use the real enum!!
    MakePatternPtr< MatchAll<Scope> > all;
    MakePatternPtr<Record> record;
    MakePatternPtr< Star<Declaration> > decls;
    
    record->members = ( decls );

    l_apall->patterns = (l_apany, l_apnot);
    l_apnot->pattern = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternPtr<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternPtr<SpecificInteger>(32);
            
    MakePatternPtr< SlaveSearchReplace<Scope> > slavel( record, l_apall );   

    ms_sub->operands = (lmap_id, m_state_id);
    
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_sub, m_state_id );   

    ns_goto->destination = n_dest_expr;
    nr_goto->destination = nr_sub;
    nr_sub->operands = (lmap_id, n_dest_expr);
    n_dest_expr->pattern = nsx_sub;
    nsx_sub->operands = (lmap_id, MakePatternPtr<Expression>());
    
    MakePatternPtr< SlaveSearchReplace<Scope> > slaven( slavem, ns_goto, nr_goto );   
    
    all->patterns = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternPtr<Labeley>();
    stuff_labeley->terminus = apall;
    apall->patterns = (apany, apnot);
    apany->terminus = labeley;
    apnot->pattern = lmap;
    
    Configure( all, slaven );
}

/*
RemoveLabelSubscript::RemoveLabelSubscript()
{
    MakePatternPtr< Stuff<Scope> > stuff_labeley, stuff_lmap;
    MakePatternPtr<Labeley> labeley;
    MakePatternPtr<Static> lmap;
    MakePatternPtr<Const> lmap_const;
    MakePatternPtr<Array> lmap_type;
    MakePatternPtr<InstanceIdentifier> lmap_id; 
    MakePatternPtr< MatchAll<Node> > apall, l_apall;
    MakePatternPtr< NotMatch<Node> > apnot, l_apnot;
    MakePatternPtr< AnyNode<Node> > apany, l_apany;
    MakePatternPtr< Overlay<Type> > l_over;
    MakePatternPtr<Subscript> ms_sub, nr_sub, nsx_sub;;
    MakePatternPtr<InstanceIdentifier> m_state_id;
    MakePatternPtr<Goto> ns_goto, nr_goto;
    MakePatternPtr< NotMatch<Expression> > n_dest_expr;
    MakePatternPtr<Unsigned> l_enum; // TODO use the real enum!!
    MakePatternPtr< MatchAll<Scope> > all;
    MakePatternPtr<Record> record;
    MakePatternPtr< Star<Declaration> > decls;
    
    ms_sub->operands = (lmap_id, m_state_id);    
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_sub, m_state_id );       
    Configure( all, slaven );
}


LabelInstancesToEnum::LabelInstancesToEnum()
{
    MakePatternPtr< Stuff<Scope> > stuff_labeley, stuff_lmap;
    MakePatternPtr<Labeley> labeley;
    MakePatternPtr<Static> lmap;
    MakePatternPtr<Const> lmap_const;
    MakePatternPtr<Array> lmap_type;
    MakePatternPtr<InstanceIdentifier> lmap_id; 
    MakePatternPtr< MatchAll<Node> > apall, l_apall;
    MakePatternPtr< NotMatch<Node> > apnot, l_apnot;
    MakePatternPtr< AnyNode<Node> > apany, l_apany;
    MakePatternPtr< Overlay<Type> > l_over;
    MakePatternPtr<Subscript> ms_sub, nr_sub, nsx_sub;;
    MakePatternPtr<InstanceIdentifier> m_state_id;
    MakePatternPtr<Goto> ns_goto, nr_goto;
    MakePatternPtr< NotMatch<Expression> > n_dest_expr;
    MakePatternPtr<Unsigned> l_enum; // TODO use the real enum!!
    MakePatternPtr< MatchAll<Scope> > all;
    MakePatternPtr<Record> record;
    MakePatternPtr< Star<Declaration> > decls;
    
    record->members = ( decls );

    l_apall->patterns = (l_apany, l_apnot);
    l_apnot->pattern = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternPtr<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternPtr<SpecificInteger>(32);
            
    MakePatternPtr< SlaveSearchReplace<Scope> > slavel( record, l_apall );   

    all->patterns = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternPtr<Labeley>();
    stuff_labeley->terminus = apall;
    apall->patterns = (apany, apnot);
    apany->terminus = labeley;
    apnot->pattern = lmap;
    
    Configure( all, slavel );
}
*/

LabelVarsToEnum::LabelVarsToEnum()
{
    MakePatternPtr< MatchAll<Scope> > s_all;
    MakePatternPtr< MatchAll<Statement> > sx_all;
    MakePatternPtr< Stuff<Scope> > scope;
    MakePatternPtr<Instance> var;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr< Stuff<Scope> > s_stuff, sx_stuff;
    MakePatternPtr<Assign> s_assign, sx_assign, l_assign;
    MakePatternPtr<Subscript> s_sub, sx_sub, ls_sub, mr_sub, msx_sub;
    MakePatternPtr< NotMatch<Scope> > sx_not1;
    MakePatternPtr<AssignmentOperator> sx_asop, ms_asop;
    MakePatternPtr< NotMatch<Statement> > sx_not2, msx_not, msx_not2, msx_not3;
    MakePatternPtr< Overlay<Type> > over;
    MakePatternPtr< TransformOf<Expression> > s_index( &TypeOf::instance );
    MakePatternPtr<TypeIdentifier> type; 
    MakePatternPtr< Overlay<Expression> > l_over, m_over;
    MakePatternPtr<Expression> l_index;
    MakePatternPtr< MatchAll<Node> > ms_all;
    MakePatternPtr< AnyNode<Node> > ms_anynode;    
    MakePatternPtr<NestedArray> nested_array;
    MakePatternPtr<NestedSubscriptLookup> nested_subscript, nested_subscript2, nested_subscript3;
    MakePatternPtr<NestedSubscriptLookup> l_nested_subscript, m_nested_subscript, m_nested_subscript2;
    MakePatternPtr<Instance> msx_inst;
    MakePatternPtr<String> depth;
    MakePatternPtr< Stuff<Scope> > lmap_stuff;
    MakePatternPtr<Static> lmap;
    MakePatternPtr<Const> lmap_const;
    MakePatternPtr<Array> lmap_type;
    MakePatternPtr<InstanceIdentifier> lmap_id; 
    
    l_assign->operands = (l_nested_subscript, l_over);
    l_nested_subscript->terminus = var_id;    
    l_nested_subscript->depth = depth;
    l_over->through = ls_sub;
    ls_sub->operands = (lmap_id, l_index);
    l_over->overlay = l_index;    
   
    MakePatternPtr< SlaveSearchReplace<Scope> > slavel( scope, l_assign );   
   
    ms_all->patterns = (ms_anynode, msx_not, msx_not2, msx_not3);
    msx_not2->pattern = msx_inst;
    msx_inst->identifier = var_id;    
    msx_not->pattern = ms_asop;
    ms_asop->operands = (m_nested_subscript2, MakePatternPtr< Star<Expression> >() );
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
    
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_all, ms_anynode );   
   
    s_all->patterns = (scope, s_stuff, sx_not1, lmap_stuff);
    scope->terminus = var;
    var->type = nested_array;
    nested_array->terminus = over;
    //nested_array->depth = depth;
    over->through = MakePatternPtr<Labeley>();    
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
    sx_asop->operands = (nested_subscript3, MakePatternPtr< Star<Expression> >() );
    nested_subscript3->terminus = var_id;
    nested_subscript3->depth = depth;
    sx_not2->pattern = sx_assign;
    sx_assign->operands = (nested_subscript2, sx_sub);
    nested_subscript2->terminus = var_id;
    nested_subscript2->depth = depth;
    sx_sub->operands = (lmap_id, MakePatternPtr<Expression>() );
    over->overlay = type;
    lmap_stuff->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternPtr<Labeley>();
        
    Configure( s_all, slavem );
}


SwapSubscriptMultiplex::SwapSubscriptMultiplex()
{
    MakePatternPtr<Multiplexor> s_mux, r_mux;
    MakePatternPtr<Subscript> s_sub1, s_sub2, r_sub;
    MakePatternPtr<Expression> cond, index1, index2;
    MakePatternPtr<InstanceIdentifier> array; // Instance used to prevent side effects, which would go out of sequence
    
    s_mux->operands = (cond, s_sub1, s_sub2);
    s_sub1->operands = (array, index1);
    s_sub2->operands = (array, index2);
    
    r_sub->operands = (array, r_mux);
    r_mux->operands = (cond, index1, index2);
    
    Configure( s_mux, r_sub );
}


AddStateEnumVar::AddStateEnumVar()
{
    MakePatternPtr<Compound> s_comp, r_comp, lr_compound;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<Goto> ls_goto, lr_goto, s_goto;
    MakePatternPtr<Assign> lr_assign;
    MakePatternPtr<Automatic> state_var;
    MakePatternPtr< NotMatch<Expression> > lx_not;
    MakePatternPtr< BuildInstanceIdentifier > state_var_id("state");
    MakePatternPtr< GreenGrass<Compound> > s_gg;
    MakePatternPtr<Subscript> s_sub, ls_sub, lr_sub;
    MakePatternPtr<InstanceIdentifier> array;
    MakePatternPtr< TransformOf<Expression> > s_index( &TypeOf::instance );
    MakePatternPtr<Integral> type; // TODO use the enum type, and ensure properly supported in TypeOf
    
    ls_goto->destination = ls_sub;
    ls_sub->operands = (array, lx_not);
    lx_not->pattern = state_var_id; //  MakePatternPtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lx_not);
    lr_goto->destination = lr_sub;
    lr_sub->operands = (array, state_var_id);
            
    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave( r_comp, ls_goto, lr_compound );
     
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
    state_var->initialiser = MakePatternPtr<Uninitialised>();

    Configure( s_gg, r_slave );
}


ApplyCombGotoPolicy::ApplyCombGotoPolicy()
{
    MakePatternPtr<Compound> comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, body, post;
    MakePatternPtr<Goto> s_goto1, goto2, sx_pre_goto;
    MakePatternPtr<Subscript> sub;
    MakePatternPtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakePatternPtr<StateLabel> label;
    MakePatternPtr< NotMatch<Statement> > sx_pre, sx_body;
    MakePatternPtr<Uncombable> sx_uncombable;
    MakePatternPtr< Erase<Statement> > s_erase;
    MakePatternPtr< Overlay<Statement> > over;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Equal> r_equal;   
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure(comp);
}


ApplyYieldGotoPolicy::ApplyYieldGotoPolicy()
{
    MakePatternPtr<Compound> comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, body1, body2, post;
    MakePatternPtr<Goto> s_goto1, goto2, sx_pre_goto;
    MakePatternPtr<Subscript> sub;
    MakePatternPtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakePatternPtr<StateLabel> label;
    MakePatternPtr< NotMatch<Statement> > sx_pre, sx_body1, sx_body2;
    MakePatternPtr< Erase<Statement> > s_erase, s_erase2;
    MakePatternPtr< Insert<Statement> > r_insert;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Equal> r_equal;   
    MakePatternPtr<Wait> wait;
    MakePatternPtr<Uncombable> sx_uncombable1, sx_uncombable2;
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, goto2);
    
    Configure(comp);
}


ApplyBottomPolicy::ApplyBottomPolicy()
{
    MakePatternPtr<Compound> comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, body;
    MakePatternPtr<Goto> goto1, sx_pre_goto;
    MakePatternPtr<Subscript> sub;
    MakePatternPtr<InstanceIdentifier> lmap_id, state_var_id, state_id;
    MakePatternPtr<StateLabel> label;
    MakePatternPtr< NotMatch<Statement> > sx_pre, sx_body;
    MakePatternPtr< Erase<Statement> > s_erase;
    MakePatternPtr< Insert<Statement> > r_insert;
    MakePatternPtr< Overlay<Statement> > over;
    MakePatternPtr<If> r_if, r_if2;
    MakePatternPtr<Equal> r_equal;   
    MakePatternPtr<NotEqual> r_not_equal;   
    MakePatternPtr<Uncombable> sx_uncombable;
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    r_insert->insert = goto1; // r_if2; TODO: with the condition, superloop is exiting before the last state block has run
    r_if2->condition = r_not_equal;
    r_if2->body = goto1;
    r_if2->else_body = MakePatternPtr<Nop>();
    r_not_equal->operands = (state_var_id, state_id);    
    
    Configure(comp);
}


ApplyLabelPolicy::ApplyLabelPolicy()
{
    MakePatternPtr<Compound> comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr<StateLabel> label1;
    MakePatternPtr<Label> label2, sx_post_label;
    MakePatternPtr<If> iif;
    MakePatternPtr<Equal> equal;   
    MakePatternPtr< NotMatch<Statement> > sx_post;
    MakePatternPtr<InstanceIdentifier> state_var_id, state_id;
    MakePatternPtr< Erase<Label> > erase, erase_star;
    MakePatternPtr< Insert<Label> > insert, insert_star;
    MakePatternPtr< Star<Label> > label_star;    
        
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
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr<Compound> comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > body1, body2, post;
    MakePatternPtr<Label> label, sx_label, sx_label2;
    MakePatternPtr< NotMatch<Statement> > sx_stmt;
    MakePatternPtr< Erase<Statement> > s_erase;
    MakePatternPtr< Insert<Statement> > r_insert;
    MakePatternPtr<If> r_if;
    MakePatternPtr<Equal> r_equal;   
    MakePatternPtr<DeltaCount> r_delta_count;
    MakePatternPtr<SpecificInteger> r_zero(0);
    MakePatternPtr<Wait> wait;
    MakePatternPtr< Stuff<Compound> > s_stuff;
    MakePatternPtr<Goto> gotoo;
    MakePatternPtr< NotMatch<Statement> > sx_body1, sx_body2;
    MakePatternPtr<Uncombable> sx_uncombable1, sx_uncombable2;
       
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
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure(s_all, comp);
}


EnsureResetYield::EnsureResetYield()
{
    MakePatternPtr<Compound> comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > pre, post;
    MakePatternPtr< NotMatch<Statement> > sx_not;
    MakePatternPtr< MatchAny<Statement> > sx_any;
    MakePatternPtr<Goto> gotoo;
    MakePatternPtr< Insert<Statement> > insert;
    MakePatternPtr<WaitDelta> r_yield;
    
    comp->members = (decls);
    comp->statements = (pre, insert, gotoo, post);
    pre->pattern = sx_not;
    sx_not->pattern = sx_any;
    sx_any->patterns = (MakePatternPtr<Goto>(), MakePatternPtr<Label>(), MakePatternPtr<Wait>() );
    
    insert->insert = r_yield;
    
    Configure(comp);
}


DetectSuperLoop::DetectSuperLoop( bool is_conditional_goto )
{
    MakePatternPtr<Instance> inst;
    MakePatternPtr<Compound> s_comp, r_comp, r_body_comp;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Statement> > body;
    MakePatternPtr<Label> s_label;
    MakePatternPtr<If> s_ifgoto;
    MakePatternPtr<Goto> s_goto;
    MakePatternPtr<Expression> cond;
    MakePatternPtr< NotMatch<Statement> > sx_not;
    MakePatternPtr<Do> r_do;
    MakePatternPtr< Overlay<Compound> > over;
    
    MakePatternPtr< SlaveSearchReplace<Statement> > slavell( r_body_comp, MakePatternPtr<Goto>(), MakePatternPtr<Continue>() );    
    
    inst->type = MakePatternPtr<Callable>();
    inst->initialiser = over;
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (s_label, body, is_conditional_goto 
                                         ? TreePtr<Statement>(s_ifgoto) 
                                         : TreePtr<Statement>(s_goto) );
    body->pattern = sx_not;
    sx_not->pattern = MakePatternPtr<Label>(); // so s_label is the only one - all gotos must go to it.
    s_ifgoto->condition = cond;
    s_ifgoto->body = s_goto;
    s_ifgoto->else_body = MakePatternPtr<Nop>();
    
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (r_do);
    r_do->condition = is_conditional_goto 
                      ? cond 
                      : TreePtr<Expression>(MakePatternPtr<True>());
    r_do->body = slavell;
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure(inst);
}


InsertInferredYield::InsertInferredYield()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Thread> thread; // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    MakePatternPtr<Compound> func_comp, s_comp, sx_comp, r_comp;
    MakePatternPtr< Star<Declaration> > func_decls, loop_decls;
    MakePatternPtr< Star<Statement> >  stmts, sx_pre;    
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
    func_comp->statements = (loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = (loop_decls);
    s_comp->statements = (stmts);
    stmts->pattern = MakePatternPtr<If>();
    
    over->overlay = r_comp;
    r_comp->members = (loop_decls);
    r_comp->statements = (stmts, r_yield);
    
    Configure( fn );            
}


