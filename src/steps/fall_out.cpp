
#include "steps/fall_out.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "inferno_agents.hpp"

 
using namespace CPPTree;
using namespace SCTree;
using namespace Steps;


// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : Label
{
    NODE_FUNCTIONS_FINAL
    TreePtr<InstanceIdentifier> state;
};



PlaceLabelsInArray::PlaceLabelsInArray()
{
    auto s_module = MakePatternNode<Scope>();
    auto r_module = MakePatternNode<Scope>();
    auto gg = MakePatternNode< GreenGrass<Type> >();
    auto func = MakePatternNode<Field>();
    auto m_func = MakePatternNode<Field>();
    auto func_id = MakePatternNode<InstanceIdentifier>();
    auto thread = MakePatternNode<Thread>();
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto module_decls = MakePatternNode< Star<Declaration> >();
    auto func_stmts = MakePatternNode< Star<Statement> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto r_module_enum = MakePatternNode<Enum>();
    auto r_enum_id = MakePatternNode<BuildTypeIdentifierAgent>("%sStates");
    auto var_id = MakePatternNode<InstanceIdentifier>();
    auto module_id = MakePatternNode<TypeIdentifier>();
    auto l_func_decls = MakePatternNode< Star<Declaration> >();
    auto l_enum_vals = MakePatternNode< Star<Declaration> >();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_module_decls = MakePatternNode< Star<Declaration> >();
    auto l_func_pre = MakePatternNode< Star<Statement> >();
    auto l_func_post = MakePatternNode< Star<Statement> >();
    auto l_pre = MakePatternNode< Star<Statement> >();
    auto l_block = MakePatternNode< Star<Statement> >();
    auto l_post = MakePatternNode< Star<Statement> >();
    auto l_stmts = MakePatternNode< Star<Statement> >();
    auto l_dead_gotos = MakePatternNode< Star<Statement> >();
    auto l_switch = MakePatternNode<Switch>();
    auto l_over_enum = MakePatternNode< Delta<Enum> >();
    auto ls_enum = MakePatternNode<Enum>();
    auto lr_enum = MakePatternNode<Enum>();
    auto lr_state_decl = MakePatternNode<Static>();
    auto lr_state_id = MakePatternNode<BuildInstanceIdentifierAgent>("%s_STATE_%s");
    auto lr_case = MakePatternNode<Case>();
    auto lr_int = MakePatternNode<Signed>();
    auto lr_count = MakePatternNode<BuildContainerSizeAgent>();
    auto ls_label_id = MakePatternNode<LabelIdentifier>();
    auto var_decl = MakePatternNode<Instance>();
    auto l_var_decl = MakePatternNode<Instance>();
    auto ll_all = MakePatternNode< Conjunction<Node> >();
    auto lls_not1 = MakePatternNode< Negation<Node> >();
    auto lls_not2 = MakePatternNode< Negation<Node> >();
    auto ll_any = MakePatternNode< AnyNode<Node> >();
    auto ll_over = MakePatternNode< Delta<Node> >();
    auto ll_all_over = MakePatternNode< Delta<Node> >();
    auto lls_goto = MakePatternNode<Goto>();
    auto lls_label = MakePatternNode<Label>();
    auto ls_goto = MakePatternNode<Goto>();
    auto ls_label = MakePatternNode<Label>();
    auto l_label = MakePatternNode<Label>();
    auto lr_if = MakePatternNode<If>();
    auto lr_equal = MakePatternNode<Equal>();
    auto l_loop = MakePatternNode<Loop>();
    auto l_over = MakePatternNode< Delta<Statement> >();
    auto l_not = MakePatternNode< Negation<Statement> >();
    auto m_stuff_func = MakePatternNode< Stuff<Scope> >();
    auto l_module = MakePatternNode<Scope>();
    auto l_func = MakePatternNode<Field>();
    auto r_lmap = MakePatternNode<Static>();
    auto l_lmap = MakePatternNode<Static>();
    auto r_lmap_id = MakePatternNode<BuildInstanceIdentifierAgent>("lmap");
    auto r_array = MakePatternNode<Array>();
    auto r_make = MakePatternNode<MakeArray>();
    auto ls_make = MakePatternNode<MakeArray>();
    auto lr_make = MakePatternNode<MakeArray>();
    auto lls_make = MakePatternNode<MakeArray>();
    auto l_mover = MakePatternNode< Delta<Expression> >();
    auto l_stuff = MakePatternNode< Stuff<Statement> >();
    auto l_existing = MakePatternNode< Star<Expression> >();
    auto l_mnot = MakePatternNode< Negation<Expression> >();
    auto ll_sub = MakePatternNode< Subscript >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto l_comp = MakePatternNode<Compound>();
    auto over_comp = MakePatternNode< Delta<Compound> >();
    auto l_overll = MakePatternNode< Delta<Statement> >();
    auto l_state_label = MakePatternNode< StateLabel >();
    auto comp_membs = MakePatternNode< Star<Declaration> >();
            
    ll_all_over->through = ll_all;
    ll_all_over->overlay = ll_any;
    ll_all->conjuncts = (ll_any, lls_not1, lls_not2); 
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = ll_sub;
    ll_sub->operands = (r_lmap_id, lr_state_id);
    lls_not1->negand = lls_make;
    lls_make->operands = (MakePatternNode< Star<Expression> >(), 
                          ls_label_id,
                          MakePatternNode< Star<Expression> >()); // TODO too strong, will hit (arr){LABEL} in original code
    lls_not2->negand = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    auto slavell = MakePatternNode< SlaveSearchReplace<Scope> >( l_module, ll_all_over );    
    
    l_func->identifier = func_id;
    l_func->initialiser = l_comp;
    l_comp->members = (MakePatternNode< Star<Declaration> >(), l_lmap);
    l_comp->statements = (MakePatternNode< Star<Statement> >(), l_stuff, MakePatternNode< Star<Statement> >());
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    ls_enum->members = (l_enum_vals);
    lr_enum->members = (l_enum_vals, lr_state_decl);
    lr_enum->identifier = ls_enum->identifier = r_enum_id;
    l_block->restriction = l_not;
    l_not->negand = MakePatternNode<Goto>();
    l_post->restriction = MakePatternNode<If>();    
    l_dead_gotos->restriction = MakePatternNode<Goto>();
    l_over_enum->through = ls_enum;
    l_over_enum->overlay = lr_enum;
    l_module->members = (l_module_decls, l_over_enum, l_func);
    lr_state_decl->constancy = MakePatternNode<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = r_enum_id;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_state_id->sources = (func_id, ls_label->identifier);
    l_lmap->identifier = r_lmap_id;
    l_lmap->initialiser = l_mover;
    l_mover->through = ls_make;
    l_mover->overlay = lr_make;
    ls_make->operands = (l_existing);
    lr_make->operands = (l_existing, ls_label_id);
    l_existing->restriction = l_mnot;
    l_mnot->negand = ls_label_id;
    l_label->identifier = ls_label_id;
    l_stuff->terminus = l_overll;
    l_overll->through = l_label;
    l_overll->overlay = l_state_label;
    l_state_label->identifier = ls_label_id;
    l_state_label->state = lr_state_id;

    auto slavel = MakePatternNode< SlaveCompareReplace<Scope> >( r_module, l_module, slavell );
    
    //s_module->bases = (bases);
    //s_module->identifier = module_id;
    func->type = gg;
    gg->through = thread;
    func->identifier = func_id;
    func->initialiser = over_comp;
    over_comp->through = s_comp;
    over_comp->overlay = r_comp;
    s_comp->members = (comp_membs);
    r_comp->members = (comp_membs, r_lmap);
    r_comp->statements = s_comp->statements = (MakePatternNode< Star<Statement> >());
    s_module->members = (module_decls, func);
    r_module->members = (module_decls, func, r_module_enum);
    //r_module->bases = (bases);
    //r_module->identifier = module_id;
    r_module_enum->identifier = r_enum_id;
    r_enum_id->sources = (func_id);            
    //r_module_enum->members = ();
    r_lmap->type = r_array;
    r_lmap->identifier = r_lmap_id;
    r_lmap->initialiser = r_make;    
    r_lmap->constancy = MakePatternNode<Const>();        
//    r_lmap->virt = MakePatternNode<NonVirtual>();
  //  r_lmap->access = MakePatternNode<Private>();    
    r_array->element = MakePatternNode<Labeley>();
    r_array->size = MakePatternNode<Uninitialised>();
    //r_make->operands = ()
    
    Configure( SEARCH_REPLACE, s_module, slavel );    
}

// New better way of propogating lmap through variables. This supercedes LabelVarsToEnum
// and SwapSubscriptConditionalOperator. It works by just changing all appearances of the Labeley 
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
    auto stuff_labeley = MakePatternNode< Stuff<Scope> >();
    auto stuff_lmap = MakePatternNode< Stuff<Scope> >();
    auto labeley = MakePatternNode<Labeley>();
    auto lmap = MakePatternNode<Static>();
    auto lmap_const = MakePatternNode<Const>();
    auto lmap_type = MakePatternNode<Array>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto apall = MakePatternNode< Conjunction<Node> >();
    auto l_apall = MakePatternNode< Conjunction<Node> >();
    auto apnot = MakePatternNode< Negation<Node> >();
    auto l_apnot = MakePatternNode< Negation<Node> >();
    auto apany = MakePatternNode< AnyNode<Node> >();
    auto l_apany = MakePatternNode< AnyNode<Node> >();
    auto l_apall_over = MakePatternNode< Delta<Node> >();
    auto l_over = MakePatternNode< Delta<Type> >();
    auto ms_sub = MakePatternNode<Subscript>();
    auto nr_sub = MakePatternNode<Subscript>();
    auto nsx_sub = MakePatternNode<Subscript>();
    auto m_state_id = MakePatternNode<InstanceIdentifier>();
    auto ns_goto = MakePatternNode<Goto>();
    auto nr_goto = MakePatternNode<Goto>();
    auto n_dest_expr = MakePatternNode< Negation<Expression> >();
    auto l_enum = MakePatternNode<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternNode< Conjunction<Scope> >();
    auto record = MakePatternNode<Record>();
    auto decls = MakePatternNode< Star<Declaration> >();
    
    record->members = ( decls );

    l_apall_over->through = l_apall;
    l_apall_over->overlay = l_apany;
    l_apall->conjuncts = (l_apany, l_apnot);
    l_apnot->negand = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternNode<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternNode<SpecificInteger>(32);
            
    auto slavel = MakePatternNode< SlaveSearchReplace<Scope> >( record, l_apall_over );   

    ms_sub->operands = (lmap_id, m_state_id);
    
    auto slavem = MakePatternNode< SlaveSearchReplace<Scope> >( slavel, ms_sub, m_state_id );   

    ns_goto->destination = n_dest_expr;
    nr_goto->destination = nr_sub;
    nr_sub->operands = (lmap_id, n_dest_expr);
    n_dest_expr->negand = nsx_sub;
    nsx_sub->operands = (lmap_id, MakePatternNode<Expression>());
    
    auto slaven = MakePatternNode< SlaveSearchReplace<Scope> >( slavem, ns_goto, nr_goto );   
    
    all->conjuncts = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternNode<Labeley>();
    stuff_labeley->terminus = apall;
    apall->conjuncts = (apany, apnot);
    apany->terminus = labeley;
    apnot->negand = lmap;
    
    Configure( SEARCH_REPLACE, all, slaven );
}

/*
RemoveLabelSubscript::RemoveLabelSubscript()
{
    auto stuff_labeley = MakePatternNode< Stuff<Scope> >();
    auto stuff_lmap = MakePatternNode< Stuff<Scope> >();
    auto labeley = MakePatternNode<Labeley>();
    auto lmap = MakePatternNode<Static>();
    auto lmap_const = MakePatternNode<Const>();
    auto lmap_type = MakePatternNode<Array>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto apall = MakePatternNode< Conjunction<Node> >();
    auto l_apall = MakePatternNode< Conjunction<Node> >();
    auto apnot = MakePatternNode< Negation<Node> >();
    auto l_apnot = MakePatternNode< Negation<Node> >();
    auto apany = MakePatternNode< AnyNode<Node> >();
    auto l_apany = MakePatternNode< AnyNode<Node> >();
    auto l_over = MakePatternNode< Delta<Type> >();
    auto ms_sub = MakePatternNode<Subscript>();
    auto nr_sub = MakePatternNode<Subscript>();
    auto nsx_sub = MakePatternNode<Subscript>();
    auto m_state_id = MakePatternNode<InstanceIdentifier>();
    auto ns_goto = MakePatternNode<Goto>();
    auto nr_goto = MakePatternNode<Goto>();
    auto n_dest_expr = MakePatternNode< Negation<Expression> >();
    auto l_enum = MakePatternNode<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternNode< Conjunction<Scope> >();
    auto record = MakePatternNode<Record>();
    auto decls = MakePatternNode< Star<Declaration> >();
    
    ms_sub->operands = (lmap_id, m_state_id);    
    auto slavem = MakePatternNode< SlaveSearchReplace<Scope> >( slavel, ms_sub, m_state_id );       
    Configure( SEARCH_REPLACE, all, slaven );
}


LabelInstancesToEnum::LabelInstancesToEnum()
{
    auto stuff_labeley = MakePatternNode< Stuff<Scope> >();
    auto stuff_lmap = MakePatternNode< Stuff<Scope> >();
    auto labeley = MakePatternNode<Labeley>();
    auto lmap = MakePatternNode<Static>();
    auto lmap_const = MakePatternNode<Const>();
    auto lmap_type = MakePatternNode<Array>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto apall = MakePatternNode< Conjunction<Node> >();
    auto l_apall = MakePatternNode< Conjunction<Node> >();
    auto apnot = MakePatternNode< Negation<Node> >();
    auto l_apnot = MakePatternNode< Negation<Node> >();
    auto apany = MakePatternNode< AnyNode<Node> >();
    auto l_apany = MakePatternNode< AnyNode<Node> >();
    auto l_over = MakePatternNode< Delta<Type> >();
    auto ms_sub = MakePatternNode<Subscript>();
    auto nr_sub = MakePatternNode<Subscript>();
    auto nsx_sub = MakePatternNode<Subscript>();
    auto m_state_id = MakePatternNode<InstanceIdentifier>();
    auto ns_goto = MakePatternNode<Goto>();
    auto nr_goto = MakePatternNode<Goto>();
    auto n_dest_expr = MakePatternNode< Negation<Expression> >();
    auto l_enum = MakePatternNode<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternNode< Conjunction<Scope> >();
    auto record = MakePatternNode<Record>();
    auto decls = MakePatternNode< Star<Declaration> >();
    
    record->members = ( decls );

    l_apall->conjuncts = (l_apany, l_apnot);
    l_apnot->negand = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternNode<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternNode<SpecificInteger>(32);
            
    auto slavel = MakePatternNode< SlaveSearchReplace<Scope> >( record, l_apall );   

    all->conjuncts = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternNode<Labeley>();
    stuff_labeley->terminus = apall;
    apall->conjuncts = (apany, apnot);
    apany->terminus = labeley;
    apnot->negand = lmap;
    
    Configure( SEARCH_REPLACE, all, slavel );
}
*/

LabelVarsToEnum::LabelVarsToEnum()
{
    auto s_all = MakePatternNode< Conjunction<Scope> >();
    auto sx_all = MakePatternNode< Conjunction<Statement> >();
    auto scope = MakePatternNode< Stuff<Scope> >();
    auto var = MakePatternNode<Instance>();
    auto var_id = MakePatternNode<InstanceIdentifier>();
    auto s_stuff = MakePatternNode< Stuff<Scope> >();
    auto sx_stuff = MakePatternNode< Stuff<Scope> >();
    auto s_assign = MakePatternNode<Assign>();
    auto sx_assign = MakePatternNode<Assign>();
    auto l_assign = MakePatternNode<Assign>();
    auto s_sub = MakePatternNode<Subscript>();
    auto sx_sub = MakePatternNode<Subscript>();
    auto ls_sub = MakePatternNode<Subscript>();
    auto mr_sub = MakePatternNode<Subscript>();
    auto msx_sub = MakePatternNode<Subscript>();
    auto sx_not1 = MakePatternNode< Negation<Scope> >();
    auto sx_asop = MakePatternNode<AssignmentOperator>();
    auto ms_asop = MakePatternNode<AssignmentOperator>();
    auto sx_not2 = MakePatternNode< Negation<Statement> >();
    auto msx_not = MakePatternNode< Negation<Statement> >();
    auto msx_not2 = MakePatternNode< Negation<Statement> >();
    auto msx_not3 = MakePatternNode< Negation<Statement> >();
    auto over = MakePatternNode< Delta<Type> >();
    auto s_index = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternNode<TypeIdentifier>();
    auto l_over = MakePatternNode< Delta<Expression> >();
    auto m_over = MakePatternNode< Delta<Expression> >();
    auto l_index = MakePatternNode<Expression>();
    auto ms_all = MakePatternNode< Conjunction<Node> >();
    auto ms_anynode = MakePatternNode< AnyNode<Node> >();
    auto nested_array = MakePatternNode<NestedArrayAgent>();
    auto nested_subscript = MakePatternNode<NestedSubscriptLookupAgent>();
    auto nested_subscript2 = MakePatternNode<NestedSubscriptLookupAgent>();
    auto nested_subscript3 = MakePatternNode<NestedSubscriptLookupAgent>();
    auto l_nested_subscript = MakePatternNode<NestedSubscriptLookupAgent>();
    auto m_nested_subscript = MakePatternNode<NestedSubscriptLookupAgent>();
    auto m_nested_subscript2 = MakePatternNode<NestedSubscriptLookupAgent>();
    auto msx_inst = MakePatternNode<Instance>();
    auto depth = MakePatternNode<String>();
    auto lmap_stuff = MakePatternNode< Stuff<Scope> >();
    auto lmap = MakePatternNode<Static>();
    auto lmap_const = MakePatternNode<Const>();
    auto lmap_type = MakePatternNode<Array>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    
    l_assign->operands = (l_nested_subscript, l_over);
    l_nested_subscript->terminus = var_id;    
    l_nested_subscript->depth = depth;
    l_over->through = ls_sub;
    ls_sub->operands = (lmap_id, l_index);
    l_over->overlay = l_index;    
   
    auto slavel = MakePatternNode< SlaveSearchReplace<Scope> >( scope, l_assign );   
   
    ms_all->conjuncts = (ms_anynode, msx_not, msx_not2, msx_not3);
    msx_not2->negand = msx_inst;
    msx_inst->identifier = var_id;    
    msx_not->negand = ms_asop;
    ms_asop->operands = (m_nested_subscript2, MakePatternNode< Star<Expression> >() );
    m_nested_subscript2->terminus = var_id;
    m_nested_subscript2->depth = depth;
    ms_anynode->terminus = m_over;
    m_over->through = m_nested_subscript;
    m_nested_subscript->terminus = var_id;
    m_nested_subscript->depth = depth;
    m_over->overlay = mr_sub;
    msx_not3->negand = msx_sub;
    msx_sub->operands = (lmap_id, m_nested_subscript);
    mr_sub->operands = (lmap_id, m_nested_subscript);
    
    auto slavem = MakePatternNode< SlaveSearchReplace<Scope> >( slavel, ms_all, ms_anynode );   
   
    s_all->conjuncts = (scope, s_stuff, sx_not1, lmap_stuff);
    scope->terminus = var;
    var->type = nested_array;
    nested_array->terminus = over;
    //nested_array->depth = depth;
    over->through = MakePatternNode<Labeley>();    
    var->identifier = var_id;
    s_stuff->terminus = s_assign;
    s_assign->operands = (nested_subscript, s_sub);
    nested_subscript->terminus = var_id;
    nested_subscript->depth = depth;
    s_sub->operands = (lmap_id, s_index);
    s_index->pattern = type;
    sx_not1->negand = sx_stuff;
    sx_stuff->terminus = sx_all;
    sx_all->conjuncts = (sx_asop, sx_not2);
    sx_asop->operands = (nested_subscript3, MakePatternNode< Star<Expression> >() );
    nested_subscript3->terminus = var_id;
    nested_subscript3->depth = depth;
    sx_not2->negand = sx_assign;
    sx_assign->operands = (nested_subscript2, sx_sub);
    nested_subscript2->terminus = var_id;
    nested_subscript2->depth = depth;
    sx_sub->operands = (lmap_id, MakePatternNode<Expression>() );
    over->overlay = type;
    lmap_stuff->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternNode<Labeley>();
        
    Configure( COMPARE_REPLACE, s_all, slavem );
}


SwapSubscriptConditionalOperator::SwapSubscriptConditionalOperator()
{
    auto s_mux = MakePatternNode<ConditionalOperator>();
    auto r_mux = MakePatternNode<ConditionalOperator>();
    auto s_sub1 = MakePatternNode<Subscript>();
    auto s_sub2 = MakePatternNode<Subscript>();
    auto r_sub = MakePatternNode<Subscript>();
    auto cond = MakePatternNode<Expression>();
    auto index1 = MakePatternNode<Expression>();
    auto index2 = MakePatternNode<Expression>();
    auto array = MakePatternNode<InstanceIdentifier>(); // Instance used to prevent side effects, which would go out of sequence
    
    s_mux->operands = (cond, s_sub1, s_sub2);
    s_sub1->operands = (array, index1);
    s_sub2->operands = (array, index2);
    
    r_sub->operands = (array, r_mux);
    r_mux->operands = (cond, index1, index2);
    
    Configure( SEARCH_REPLACE, s_mux, r_sub );
}


AddStateEnumVar::AddStateEnumVar()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto lr_compound = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto ls_goto = MakePatternNode<Goto>();
    auto lr_goto = MakePatternNode<Goto>();
    auto s_goto = MakePatternNode<Goto>();
    auto lr_assign = MakePatternNode<Assign>();
    auto state_var = MakePatternNode<Automatic>();
    auto lx_not = MakePatternNode< Negation<Expression> >();
    auto state_var_id = MakePatternNode< BuildInstanceIdentifierAgent >("state");
    auto s_gg = MakePatternNode< GreenGrass<Compound> >();
    auto s_sub = MakePatternNode<Subscript>();
    auto ls_sub = MakePatternNode<Subscript>();
    auto lr_sub = MakePatternNode<Subscript>();
    auto array = MakePatternNode<InstanceIdentifier>();
    auto s_index = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternNode<Integral>(); // TODO use the enum type, and ensure properly supported in TypeOf
    
    ls_goto->destination = ls_sub;
    ls_sub->operands = (array, lx_not);
    lx_not->negand = state_var_id; //  MakePatternNode<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lx_not);
    lr_goto->destination = lr_sub;
    lr_sub->operands = (array, state_var_id);
            
    auto r_slave = MakePatternNode< SlaveSearchReplace<Statement> >( r_comp, ls_goto, lr_compound );
     
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
    state_var->initialiser = MakePatternNode<Uninitialised>();

    Configure( SEARCH_REPLACE, s_gg, r_slave );
}


ApplyCombGotoPolicy::ApplyCombGotoPolicy()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto body = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto s_goto1 = MakePatternNode<Goto>();
    auto goto2 = MakePatternNode<Goto>();
    auto sx_pre_goto = MakePatternNode<Goto>();
    auto sub = MakePatternNode<Subscript>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto state_var_id = MakePatternNode<InstanceIdentifier>();
    auto state_id = MakePatternNode<InstanceIdentifier>();
    auto label = MakePatternNode<StateLabel>();
    auto sx_pre = MakePatternNode< Negation<Statement> >();
    auto sx_body = MakePatternNode< Negation<Statement> >();
    auto sx_uncombable = MakePatternNode<Uncombable>();
    auto r_if = MakePatternNode<If>();
    auto r_equal = MakePatternNode<Equal>();
    
    r_comp->members = s_comp->members = (decls);
    s_comp->statements = (pre, s_goto1, label, body, goto2, post);
    r_comp->statements = (pre, label, r_if, goto2, post);
    pre->restriction = sx_pre,
    sx_pre->negand = sx_pre_goto; // ensure we act on the first goto only
    s_goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    body->restriction = sx_body;
    sx_body->negand = sx_uncombable; 
    goto2->destination = sub;    
    
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakePatternNode<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyYieldGotoPolicy::ApplyYieldGotoPolicy()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto body1 = MakePatternNode< Star<Statement> >();
    auto body2 = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto s_goto1 = MakePatternNode<Goto>();
    auto goto2 = MakePatternNode<Goto>();
    auto sx_pre_goto = MakePatternNode<Goto>();
    auto sub = MakePatternNode<Subscript>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto state_var_id = MakePatternNode<InstanceIdentifier>();
    auto state_id = MakePatternNode<InstanceIdentifier>();
    auto label = MakePatternNode<StateLabel>();
    auto sx_pre = MakePatternNode< Negation<Statement> >();
    auto sx_body1 = MakePatternNode< Negation<Statement> >();
    auto sx_body2 = MakePatternNode< Negation<Statement> >();
    auto r_if = MakePatternNode<If>();
    auto r_equal = MakePatternNode<Equal>();
    auto wait = MakePatternNode<Wait>();
    auto sx_uncombable1 = MakePatternNode<Uncombable>();
    auto sx_uncombable2 = MakePatternNode<Uncombable>();
    
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (pre, s_goto1, label, body1, wait, body2, goto2, post);
    r_comp->statements = (pre, label, r_if, goto2, post);
    pre->restriction = sx_pre,
    sx_pre->negand = sx_pre_goto; // ensure we act on the first goto only
    s_goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    goto2->destination = sub;    
    body1->restriction = sx_body1;
    sx_body1->negand = sx_uncombable1;
    body2->restriction = sx_body2;
    sx_body2->negand = sx_uncombable2;
        
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakePatternNode<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, goto2);
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyBottomPolicy::ApplyBottomPolicy()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto body = MakePatternNode< Star<Statement> >();
    auto goto1 = MakePatternNode<Goto>();
    auto sx_pre_goto = MakePatternNode<Goto>();
    auto sub = MakePatternNode<Subscript>();
    auto lmap_id = MakePatternNode<InstanceIdentifier>();
    auto state_var_id = MakePatternNode<InstanceIdentifier>();
    auto state_id = MakePatternNode<InstanceIdentifier>();
    auto label = MakePatternNode<StateLabel>();
    auto sx_pre = MakePatternNode< Negation<Statement> >();
    auto sx_body = MakePatternNode< Negation<Statement> >();
    auto r_if = MakePatternNode<If>();
    auto r_if2 = MakePatternNode<If>();
    auto r_equal = MakePatternNode<Equal>();
    auto r_not_equal = MakePatternNode<NotEqual>();
    auto sx_uncombable = MakePatternNode<Uncombable>();
    
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (pre, goto1, label, body);
    r_comp->statements = (pre, label, r_if, goto1);
    pre->restriction = sx_pre,
    sx_pre->negand = sx_pre_goto; // ensure we act on the first goto only
    goto1->destination = sub;
    sub->operands = (lmap_id, state_var_id);
    label->state = state_id;
    body->restriction = sx_body,
    sx_body->negand = sx_uncombable; 
    
    r_if->condition = r_equal;
    r_if->body = r_body_comp;
    r_if->else_body = MakePatternNode<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    // TODO: with the condition, superloop is exiting before the last state block has run
    r_if2->condition = r_not_equal;
    r_if2->body = goto1;
    r_if2->else_body = MakePatternNode<Nop>();
    r_not_equal->operands = (state_var_id, state_id);    
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyLabelPolicy::ApplyLabelPolicy()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto item_name = MakePatternNode<StateLabel>();
    auto item_extra = MakePatternNode<Label>();
    auto sx_post_label = MakePatternNode<Label>();
    auto iif = MakePatternNode<If>();
    auto equal = MakePatternNode<Equal>();
    auto sx_post = MakePatternNode< Negation<Statement> >();
    auto state_var_id = MakePatternNode<InstanceIdentifier>();
    auto state_id = MakePatternNode<InstanceIdentifier>();
    auto label_star = MakePatternNode< Star<Label> >();
        
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (pre, item_name, iif, item_extra, label_star, post);
    r_comp->statements = (pre, item_extra, label_star, item_name, iif, post);
    item_name->state = state_id;
    iif->condition = equal;
    equal->operands = (state_var_id, state_id);
    post->restriction = sx_post;
    sx_post->negand = sx_post_label;
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}

ApplyTopPolicy::ApplyTopPolicy()
{
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto body1 = MakePatternNode< Star<Statement> >();
    auto body2 = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto label = MakePatternNode<Label>();
    auto sx_label = MakePatternNode<Label>();
    auto sx_label2 = MakePatternNode<Label>();
    auto sx_stmt = MakePatternNode< Negation<Statement> >();
    auto r_if = MakePatternNode<If>();
    auto r_equal = MakePatternNode<Equal>();
    auto r_delta_count = MakePatternNode<DeltaCount>();
    auto r_zero = MakePatternNode<SpecificInteger>(0);
    auto wait = MakePatternNode<Wait>();
    auto s_stuff = MakePatternNode< Stuff<Compound> >();
    auto gotoo = MakePatternNode<Goto>();
    auto sx_body1 = MakePatternNode< Negation<Statement> >();
    auto sx_body2 = MakePatternNode< Negation<Statement> >();
    auto sx_uncombable1 = MakePatternNode<Uncombable>();
    auto sx_uncombable2 = MakePatternNode<Uncombable>();
       
    s_all->conjuncts = (s_comp, s_stuff);
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (body1, wait, body2, label, post);
    r_comp->statements = (label, r_if, post);
    s_stuff->terminus = gotoo;
    body1->restriction = sx_body1;
    sx_body1->negand = sx_uncombable1;
    body2->restriction = sx_body2;
    sx_body2->negand = sx_uncombable2;
    
    r_if->condition = r_equal;
    r_equal->operands = (r_delta_count, r_zero);
    r_if->body = r_body_comp;
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, gotoo);
    r_if->else_body = MakePatternNode<Nop>();
    
    Configure( SEARCH_REPLACE, s_all, r_comp );
}


EnsureResetYield::EnsureResetYield()
{
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto pre = MakePatternNode< Star<Statement> >();
    auto post = MakePatternNode< Star<Statement> >();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    auto sx_any = MakePatternNode< Disjunction<Statement> >();
    auto gotoo = MakePatternNode<Goto>();
    auto r_yield = MakePatternNode<WaitDelta>();
    
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (pre, gotoo, post);
    r_comp->statements = (pre, r_yield, gotoo, post);
    pre->restriction = sx_not;
    sx_not->negand = sx_any;
    sx_any->disjuncts = (MakePatternNode<Goto>(), MakePatternNode<Label>(), MakePatternNode<Wait>() );
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


DetectSuperLoop::DetectSuperLoop( bool is_conditional_goto )
{
    auto inst = MakePatternNode<Instance>();
    auto s_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto r_body_comp = MakePatternNode<Compound>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto body = MakePatternNode< Star<Statement> >();
    auto s_label = MakePatternNode<Label>();
    auto s_ifgoto = MakePatternNode<If>();
    auto s_goto = MakePatternNode<Goto>();
    auto cond = MakePatternNode<Expression>();
    auto sx_not = MakePatternNode< Negation<Statement> >();
    auto r_do = MakePatternNode<Do>();
    auto over = MakePatternNode< Delta<Compound> >();
    
    auto slavell = MakePatternNode< SlaveSearchReplace<Statement> >( r_body_comp, MakePatternNode<Goto>(), MakePatternNode<Continue>() );    
    
    inst->type = MakePatternNode<Callable>();
    inst->initialiser = over;
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (s_label, body, is_conditional_goto 
                                         ? TreePtr<Statement>(s_ifgoto) 
                                         : TreePtr<Statement>(s_goto) );
    body->restriction = sx_not;
    sx_not->negand = MakePatternNode<Label>(); // so s_label is the only one - all gotos must go to it.
    s_ifgoto->condition = cond;
    s_ifgoto->body = s_goto;
    s_ifgoto->else_body = MakePatternNode<Nop>();
    
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (r_do);
    r_do->condition = is_conditional_goto 
                      ? cond 
                      : TreePtr<Expression>(MakePatternNode<True>());
    r_do->body = slavell;
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure( SEARCH_REPLACE, inst );
}


InsertInferredYield::InsertInferredYield()
{
    auto fn = MakePatternNode<Instance>();
    auto fn_id = MakePatternNode<InstanceIdentifier>();
    auto thread = MakePatternNode<Thread>(); // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    auto func_comp = MakePatternNode<Compound>();
    auto s_comp = MakePatternNode<Compound>();
    auto sx_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto func_decls = MakePatternNode< Star<Declaration> >();
    auto loop_decls = MakePatternNode< Star<Declaration> >();
    auto stmts = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto over = MakePatternNode< Delta<Statement> >();
    auto flag_decl = MakePatternNode<LocalVariable>();
    auto flag_id = MakePatternNode<InstanceIdentifier>();
    auto r_yield = MakePatternNode<WaitDelta>();
    auto loop = MakePatternNode<Loop>();
    auto r_if = MakePatternNode<If>();
    auto sx_if = MakePatternNode<If>();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_notmatch = MakePatternNode< Negation<Compound> >();
    auto r_not = MakePatternNode< LogicalNot >();
    auto sx_not = MakePatternNode< LogicalNot >();
    auto assign = MakePatternNode< Assign >();
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    func_comp->statements = (loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = (loop_decls);
    s_comp->statements = (stmts);
    stmts->restriction = MakePatternNode<If>();
    
    over->overlay = r_comp;
    r_comp->members = (loop_decls);
    r_comp->statements = (stmts, r_yield);
    
    Configure( SEARCH_REPLACE, fn );            
}


