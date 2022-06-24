
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
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto gg = MakePatternPtr< GreenGrass<Type> >();
    auto func = MakePatternPtr<Field>();
    auto m_func = MakePatternPtr<Field>();
    auto func_id = MakePatternPtr<InstanceIdentifier>();
    auto thread = MakePatternPtr<Thread>();
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto module_decls = MakePatternPtr< Star<Declaration> >();
    auto func_stmts = MakePatternPtr< Star<Statement> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto r_module_enum = MakePatternPtr<Enum>();
    auto r_enum_id = MakePatternPtr<BuildTypeIdentifierAgent>("%sStates");
    auto var_id = MakePatternPtr<InstanceIdentifier>();
    auto module_id = MakePatternPtr<TypeIdentifier>();
    auto l_func_decls = MakePatternPtr< Star<Declaration> >();
    auto l_enum_vals = MakePatternPtr< Star<Declaration> >();
    auto l_decls = MakePatternPtr< Star<Declaration> >();
    auto l_module_decls = MakePatternPtr< Star<Declaration> >();
    auto l_func_pre = MakePatternPtr< Star<Statement> >();
    auto l_func_post = MakePatternPtr< Star<Statement> >();
    auto l_pre = MakePatternPtr< Star<Statement> >();
    auto l_block = MakePatternPtr< Star<Statement> >();
    auto l_post = MakePatternPtr< Star<Statement> >();
    auto l_stmts = MakePatternPtr< Star<Statement> >();
    auto l_dead_gotos = MakePatternPtr< Star<Statement> >();
    auto l_switch = MakePatternPtr<Switch>();
    auto l_over_enum = MakePatternPtr< Delta<Enum> >();
    auto ls_enum = MakePatternPtr<Enum>();
    auto lr_enum = MakePatternPtr<Enum>();
    auto lr_state_decl = MakePatternPtr<Static>();
    auto lr_state_id = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_STATE_%s");
    auto lr_case = MakePatternPtr<Case>();
    auto lr_int = MakePatternPtr<Signed>();
    auto lr_count = MakePatternPtr<BuildContainerSizeAgent>();
    auto ls_label_id = MakePatternPtr<LabelIdentifier>();
    auto var_decl = MakePatternPtr<Instance>();
    auto l_var_decl = MakePatternPtr<Instance>();
    auto ll_all = MakePatternPtr< Conjunction<Node> >();
    auto lls_not1 = MakePatternPtr< Negation<Node> >();
    auto lls_not2 = MakePatternPtr< Negation<Node> >();
    auto ll_any = MakePatternPtr< AnyNode<Node> >();
    auto ll_over = MakePatternPtr< Delta<Node> >();
    auto ll_all_over = MakePatternPtr< Delta<Node> >();
    auto lls_goto = MakePatternPtr<Goto>();
    auto lls_label = MakePatternPtr<Label>();
    auto ls_goto = MakePatternPtr<Goto>();
    auto ls_label = MakePatternPtr<Label>();
    auto l_label = MakePatternPtr<Label>();
    auto lr_if = MakePatternPtr<If>();
    auto lr_equal = MakePatternPtr<Equal>();
    auto l_loop = MakePatternPtr<Loop>();
    auto l_over = MakePatternPtr< Delta<Statement> >();
    auto l_not = MakePatternPtr< Negation<Statement> >();
    auto m_stuff_func = MakePatternPtr< Stuff<Scope> >();
    auto l_module = MakePatternPtr<Scope>();
    auto l_func = MakePatternPtr<Field>();
    auto r_lmap = MakePatternPtr<Static>();
    auto l_lmap = MakePatternPtr<Static>();
    auto r_lmap_id = MakePatternPtr<BuildInstanceIdentifierAgent>("lmap");
    auto r_array = MakePatternPtr<Array>();
    auto r_make = MakePatternPtr<MakeArray>();
    auto ls_make = MakePatternPtr<MakeArray>();
    auto lr_make = MakePatternPtr<MakeArray>();
    auto lls_make = MakePatternPtr<MakeArray>();
    auto l_mover = MakePatternPtr< Delta<Expression> >();
    auto l_stuff = MakePatternPtr< Stuff<Statement> >();
    auto l_existing = MakePatternPtr< Star<Expression> >();
    auto l_mnot = MakePatternPtr< Negation<Expression> >();
    auto ll_sub = MakePatternPtr< Subscript >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto l_comp = MakePatternPtr<Compound>();
    auto over_comp = MakePatternPtr< Delta<Compound> >();
    auto l_overll = MakePatternPtr< Delta<Statement> >();
    auto l_state_label = MakePatternPtr< StateLabel >();
    auto comp_membs = MakePatternPtr< Star<Declaration> >();
            
    ll_all_over->through = ll_all;
    ll_all_over->overlay = ll_any;
    ll_all->conjuncts = (ll_any, lls_not1, lls_not2); 
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = ll_sub;
    ll_sub->operands = (r_lmap_id, lr_state_id);
    lls_not1->negand = lls_make;
    lls_make->operands = (MakePatternPtr< Star<Expression> >(), 
                          ls_label_id,
                          MakePatternPtr< Star<Expression> >()); // TODO too strong, will hit (arr){LABEL} in original code
    lls_not2->negand = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    auto slavell = MakePatternPtr< SlaveSearchReplace<Scope> >( l_module, ll_all_over );    
    
    l_func->identifier = func_id;
    l_func->initialiser = l_comp;
    l_comp->members = (MakePatternPtr< Star<Declaration> >(), l_lmap);
    l_comp->statements = (MakePatternPtr< Star<Statement> >(), l_stuff, MakePatternPtr< Star<Statement> >());
    ls_goto->destination = var_id;
    ls_label->identifier = ls_label_id;
    ls_enum->members = (l_enum_vals);
    lr_enum->members = (l_enum_vals, lr_state_decl);
    lr_enum->identifier = ls_enum->identifier = r_enum_id;
    l_block->restriction = l_not;
    l_not->negand = MakePatternPtr<Goto>();
    l_post->restriction = MakePatternPtr<If>();    
    l_dead_gotos->restriction = MakePatternPtr<Goto>();
    l_over_enum->through = ls_enum;
    l_over_enum->overlay = lr_enum;
    l_module->members = (l_module_decls, l_over_enum, l_func);
    lr_state_decl->constancy = MakePatternPtr<Const>();
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

    auto slavel = MakePatternPtr< SlaveCompareReplace<Scope> >( r_module, l_module, slavell );
    
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
    r_comp->statements = s_comp->statements = (MakePatternPtr< Star<Statement> >());
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
    r_lmap->constancy = MakePatternPtr<Const>();        
//    r_lmap->virt = MakePatternPtr<NonVirtual>();
  //  r_lmap->access = MakePatternPtr<Private>();    
    r_array->element = MakePatternPtr<Labeley>();
    r_array->size = MakePatternPtr<Uninitialised>();
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
    auto stuff_labeley = MakePatternPtr< Stuff<Scope> >();
    auto stuff_lmap = MakePatternPtr< Stuff<Scope> >();
    auto labeley = MakePatternPtr<Labeley>();
    auto lmap = MakePatternPtr<Static>();
    auto lmap_const = MakePatternPtr<Const>();
    auto lmap_type = MakePatternPtr<Array>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto apall = MakePatternPtr< Conjunction<Node> >();
    auto l_apall = MakePatternPtr< Conjunction<Node> >();
    auto apnot = MakePatternPtr< Negation<Node> >();
    auto l_apnot = MakePatternPtr< Negation<Node> >();
    auto apany = MakePatternPtr< AnyNode<Node> >();
    auto l_apany = MakePatternPtr< AnyNode<Node> >();
    auto l_apall_over = MakePatternPtr< Delta<Node> >();
    auto l_over = MakePatternPtr< Delta<Type> >();
    auto ms_sub = MakePatternPtr<Subscript>();
    auto nr_sub = MakePatternPtr<Subscript>();
    auto nsx_sub = MakePatternPtr<Subscript>();
    auto m_state_id = MakePatternPtr<InstanceIdentifier>();
    auto ns_goto = MakePatternPtr<Goto>();
    auto nr_goto = MakePatternPtr<Goto>();
    auto n_dest_expr = MakePatternPtr< Negation<Expression> >();
    auto l_enum = MakePatternPtr<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternPtr< Conjunction<Scope> >();
    auto record = MakePatternPtr<Record>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    
    record->members = ( decls );

    l_apall_over->through = l_apall;
    l_apall_over->overlay = l_apany;
    l_apall->conjuncts = (l_apany, l_apnot);
    l_apnot->negand = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternPtr<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternPtr<SpecificInteger>(32);
            
    auto slavel = MakePatternPtr< SlaveSearchReplace<Scope> >( record, l_apall_over );   

    ms_sub->operands = (lmap_id, m_state_id);
    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( slavel, ms_sub, m_state_id );   

    ns_goto->destination = n_dest_expr;
    nr_goto->destination = nr_sub;
    nr_sub->operands = (lmap_id, n_dest_expr);
    n_dest_expr->negand = nsx_sub;
    nsx_sub->operands = (lmap_id, MakePatternPtr<Expression>());
    
    auto slaven = MakePatternPtr< SlaveSearchReplace<Scope> >( slavem, ns_goto, nr_goto );   
    
    all->conjuncts = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternPtr<Labeley>();
    stuff_labeley->terminus = apall;
    apall->conjuncts = (apany, apnot);
    apany->terminus = labeley;
    apnot->negand = lmap;
    
    Configure( SEARCH_REPLACE, all, slaven );
}

/*
RemoveLabelSubscript::RemoveLabelSubscript()
{
    auto stuff_labeley = MakePatternPtr< Stuff<Scope> >();
    auto stuff_lmap = MakePatternPtr< Stuff<Scope> >();
    auto labeley = MakePatternPtr<Labeley>();
    auto lmap = MakePatternPtr<Static>();
    auto lmap_const = MakePatternPtr<Const>();
    auto lmap_type = MakePatternPtr<Array>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto apall = MakePatternPtr< Conjunction<Node> >();
    auto l_apall = MakePatternPtr< Conjunction<Node> >();
    auto apnot = MakePatternPtr< Negation<Node> >();
    auto l_apnot = MakePatternPtr< Negation<Node> >();
    auto apany = MakePatternPtr< AnyNode<Node> >();
    auto l_apany = MakePatternPtr< AnyNode<Node> >();
    auto l_over = MakePatternPtr< Delta<Type> >();
    auto ms_sub = MakePatternPtr<Subscript>();
    auto nr_sub = MakePatternPtr<Subscript>();
    auto nsx_sub = MakePatternPtr<Subscript>();
    auto m_state_id = MakePatternPtr<InstanceIdentifier>();
    auto ns_goto = MakePatternPtr<Goto>();
    auto nr_goto = MakePatternPtr<Goto>();
    auto n_dest_expr = MakePatternPtr< Negation<Expression> >();
    auto l_enum = MakePatternPtr<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternPtr< Conjunction<Scope> >();
    auto record = MakePatternPtr<Record>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    
    ms_sub->operands = (lmap_id, m_state_id);    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( slavel, ms_sub, m_state_id );       
    Configure( SEARCH_REPLACE, all, slaven );
}


LabelInstancesToEnum::LabelInstancesToEnum()
{
    auto stuff_labeley = MakePatternPtr< Stuff<Scope> >();
    auto stuff_lmap = MakePatternPtr< Stuff<Scope> >();
    auto labeley = MakePatternPtr<Labeley>();
    auto lmap = MakePatternPtr<Static>();
    auto lmap_const = MakePatternPtr<Const>();
    auto lmap_type = MakePatternPtr<Array>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto apall = MakePatternPtr< Conjunction<Node> >();
    auto l_apall = MakePatternPtr< Conjunction<Node> >();
    auto apnot = MakePatternPtr< Negation<Node> >();
    auto l_apnot = MakePatternPtr< Negation<Node> >();
    auto apany = MakePatternPtr< AnyNode<Node> >();
    auto l_apany = MakePatternPtr< AnyNode<Node> >();
    auto l_over = MakePatternPtr< Delta<Type> >();
    auto ms_sub = MakePatternPtr<Subscript>();
    auto nr_sub = MakePatternPtr<Subscript>();
    auto nsx_sub = MakePatternPtr<Subscript>();
    auto m_state_id = MakePatternPtr<InstanceIdentifier>();
    auto ns_goto = MakePatternPtr<Goto>();
    auto nr_goto = MakePatternPtr<Goto>();
    auto n_dest_expr = MakePatternPtr< Negation<Expression> >();
    auto l_enum = MakePatternPtr<Unsigned>(); // TODO use the real enum!!
    auto all = MakePatternPtr< Conjunction<Scope> >();
    auto record = MakePatternPtr<Record>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    
    record->members = ( decls );

    l_apall->conjuncts = (l_apany, l_apnot);
    l_apnot->negand = lmap;
    l_apany->terminus = l_over;
    l_over->through = MakePatternPtr<Labeley>();
    l_over->overlay = l_enum; 
    l_enum->width = MakePatternPtr<SpecificInteger>(32);
            
    auto slavel = MakePatternPtr< SlaveSearchReplace<Scope> >( record, l_apall );   

    all->conjuncts = (record, stuff_labeley, stuff_lmap);
    stuff_lmap->terminus = lmap;
    lmap->identifier = lmap_id;
    lmap->type = lmap_type;
    lmap->constancy = lmap_const;
    lmap_type->element = MakePatternPtr<Labeley>();
    stuff_labeley->terminus = apall;
    apall->conjuncts = (apany, apnot);
    apany->terminus = labeley;
    apnot->negand = lmap;
    
    Configure( SEARCH_REPLACE, all, slavel );
}
*/

LabelVarsToEnum::LabelVarsToEnum()
{
    auto s_all = MakePatternPtr< Conjunction<Scope> >();
    auto sx_all = MakePatternPtr< Conjunction<Statement> >();
    auto scope = MakePatternPtr< Stuff<Scope> >();
    auto var = MakePatternPtr<Instance>();
    auto var_id = MakePatternPtr<InstanceIdentifier>();
    auto s_stuff = MakePatternPtr< Stuff<Scope> >();
    auto sx_stuff = MakePatternPtr< Stuff<Scope> >();
    auto s_assign = MakePatternPtr<Assign>();
    auto sx_assign = MakePatternPtr<Assign>();
    auto l_assign = MakePatternPtr<Assign>();
    auto s_sub = MakePatternPtr<Subscript>();
    auto sx_sub = MakePatternPtr<Subscript>();
    auto ls_sub = MakePatternPtr<Subscript>();
    auto mr_sub = MakePatternPtr<Subscript>();
    auto msx_sub = MakePatternPtr<Subscript>();
    auto sx_not1 = MakePatternPtr< Negation<Scope> >();
    auto sx_asop = MakePatternPtr<AssignmentOperator>();
    auto ms_asop = MakePatternPtr<AssignmentOperator>();
    auto sx_not2 = MakePatternPtr< Negation<Statement> >();
    auto msx_not = MakePatternPtr< Negation<Statement> >();
    auto msx_not2 = MakePatternPtr< Negation<Statement> >();
    auto msx_not3 = MakePatternPtr< Negation<Statement> >();
    auto over = MakePatternPtr< Delta<Type> >();
    auto s_index = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternPtr<TypeIdentifier>();
    auto l_over = MakePatternPtr< Delta<Expression> >();
    auto m_over = MakePatternPtr< Delta<Expression> >();
    auto l_index = MakePatternPtr<Expression>();
    auto ms_all = MakePatternPtr< Conjunction<Node> >();
    auto ms_anynode = MakePatternPtr< AnyNode<Node> >();
    auto nested_array = MakePatternPtr<NestedArrayAgent>();
    auto nested_subscript = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto nested_subscript2 = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto nested_subscript3 = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto l_nested_subscript = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto m_nested_subscript = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto m_nested_subscript2 = MakePatternPtr<NestedSubscriptLookupAgent>();
    auto msx_inst = MakePatternPtr<Instance>();
    auto depth = MakePatternPtr<String>();
    auto lmap_stuff = MakePatternPtr< Stuff<Scope> >();
    auto lmap = MakePatternPtr<Static>();
    auto lmap_const = MakePatternPtr<Const>();
    auto lmap_type = MakePatternPtr<Array>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    
    l_assign->operands = (l_nested_subscript, l_over);
    l_nested_subscript->terminus = var_id;    
    l_nested_subscript->depth = depth;
    l_over->through = ls_sub;
    ls_sub->operands = (lmap_id, l_index);
    l_over->overlay = l_index;    
   
    auto slavel = MakePatternPtr< SlaveSearchReplace<Scope> >( scope, l_assign );   
   
    ms_all->conjuncts = (ms_anynode, msx_not, msx_not2, msx_not3);
    msx_not2->negand = msx_inst;
    msx_inst->identifier = var_id;    
    msx_not->negand = ms_asop;
    ms_asop->operands = (m_nested_subscript2, MakePatternPtr< Star<Expression> >() );
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
    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( slavel, ms_all, ms_anynode );   
   
    s_all->conjuncts = (scope, s_stuff, sx_not1, lmap_stuff);
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
    sx_not1->negand = sx_stuff;
    sx_stuff->terminus = sx_all;
    sx_all->conjuncts = (sx_asop, sx_not2);
    sx_asop->operands = (nested_subscript3, MakePatternPtr< Star<Expression> >() );
    nested_subscript3->terminus = var_id;
    nested_subscript3->depth = depth;
    sx_not2->negand = sx_assign;
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
        
    Configure( COMPARE_REPLACE, s_all, slavem );
}


SwapSubscriptConditionalOperator::SwapSubscriptConditionalOperator()
{
    auto s_mux = MakePatternPtr<ConditionalOperator>();
    auto r_mux = MakePatternPtr<ConditionalOperator>();
    auto s_sub1 = MakePatternPtr<Subscript>();
    auto s_sub2 = MakePatternPtr<Subscript>();
    auto r_sub = MakePatternPtr<Subscript>();
    auto cond = MakePatternPtr<Expression>();
    auto index1 = MakePatternPtr<Expression>();
    auto index2 = MakePatternPtr<Expression>();
    auto array = MakePatternPtr<InstanceIdentifier>(); // Instance used to prevent side effects, which would go out of sequence
    
    s_mux->operands = (cond, s_sub1, s_sub2);
    s_sub1->operands = (array, index1);
    s_sub2->operands = (array, index2);
    
    r_sub->operands = (array, r_mux);
    r_mux->operands = (cond, index1, index2);
    
    Configure( SEARCH_REPLACE, s_mux, r_sub );
}


AddStateEnumVar::AddStateEnumVar()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto lr_compound = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto ls_goto = MakePatternPtr<Goto>();
    auto lr_goto = MakePatternPtr<Goto>();
    auto s_goto = MakePatternPtr<Goto>();
    auto lr_assign = MakePatternPtr<Assign>();
    auto state_var = MakePatternPtr<Automatic>();
    auto lx_not = MakePatternPtr< Negation<Expression> >();
    auto state_var_id = MakePatternPtr< BuildInstanceIdentifierAgent >("state");
    auto s_gg = MakePatternPtr< GreenGrass<Compound> >();
    auto s_sub = MakePatternPtr<Subscript>();
    auto ls_sub = MakePatternPtr<Subscript>();
    auto lr_sub = MakePatternPtr<Subscript>();
    auto array = MakePatternPtr<InstanceIdentifier>();
    auto s_index = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternPtr<Integral>(); // TODO use the enum type, and ensure properly supported in TypeOf
    
    ls_goto->destination = ls_sub;
    ls_sub->operands = (array, lx_not);
    lx_not->negand = state_var_id; //  MakePatternPtr<InstanceIdentifier>();
    
    lr_compound->statements = (lr_assign, lr_goto);
    lr_assign->operands = (state_var_id, lx_not);
    lr_goto->destination = lr_sub;
    lr_sub->operands = (array, state_var_id);
            
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Statement> >( r_comp, ls_goto, lr_compound );
     
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

    Configure( SEARCH_REPLACE, s_gg, r_slave );
}


ApplyCombGotoPolicy::ApplyCombGotoPolicy()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto body = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto s_goto1 = MakePatternPtr<Goto>();
    auto goto2 = MakePatternPtr<Goto>();
    auto sx_pre_goto = MakePatternPtr<Goto>();
    auto sub = MakePatternPtr<Subscript>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto state_var_id = MakePatternPtr<InstanceIdentifier>();
    auto state_id = MakePatternPtr<InstanceIdentifier>();
    auto label = MakePatternPtr<StateLabel>();
    auto sx_pre = MakePatternPtr< Negation<Statement> >();
    auto sx_body = MakePatternPtr< Negation<Statement> >();
    auto sx_uncombable = MakePatternPtr<Uncombable>();
    auto r_if = MakePatternPtr<If>();
    auto r_equal = MakePatternPtr<Equal>();
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyYieldGotoPolicy::ApplyYieldGotoPolicy()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto body1 = MakePatternPtr< Star<Statement> >();
    auto body2 = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto s_goto1 = MakePatternPtr<Goto>();
    auto goto2 = MakePatternPtr<Goto>();
    auto sx_pre_goto = MakePatternPtr<Goto>();
    auto sub = MakePatternPtr<Subscript>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto state_var_id = MakePatternPtr<InstanceIdentifier>();
    auto state_id = MakePatternPtr<InstanceIdentifier>();
    auto label = MakePatternPtr<StateLabel>();
    auto sx_pre = MakePatternPtr< Negation<Statement> >();
    auto sx_body1 = MakePatternPtr< Negation<Statement> >();
    auto sx_body2 = MakePatternPtr< Negation<Statement> >();
    auto r_if = MakePatternPtr<If>();
    auto r_equal = MakePatternPtr<Equal>();
    auto wait = MakePatternPtr<Wait>();
    auto sx_uncombable1 = MakePatternPtr<Uncombable>();
    auto sx_uncombable2 = MakePatternPtr<Uncombable>();
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = (body1, wait, body2, goto2);
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyBottomPolicy::ApplyBottomPolicy()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto body = MakePatternPtr< Star<Statement> >();
    auto goto1 = MakePatternPtr<Goto>();
    auto sx_pre_goto = MakePatternPtr<Goto>();
    auto sub = MakePatternPtr<Subscript>();
    auto lmap_id = MakePatternPtr<InstanceIdentifier>();
    auto state_var_id = MakePatternPtr<InstanceIdentifier>();
    auto state_id = MakePatternPtr<InstanceIdentifier>();
    auto label = MakePatternPtr<StateLabel>();
    auto sx_pre = MakePatternPtr< Negation<Statement> >();
    auto sx_body = MakePatternPtr< Negation<Statement> >();
    auto r_if = MakePatternPtr<If>();
    auto r_if2 = MakePatternPtr<If>();
    auto r_equal = MakePatternPtr<Equal>();
    auto r_not_equal = MakePatternPtr<NotEqual>();
    auto sx_uncombable = MakePatternPtr<Uncombable>();
    
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
    r_if->else_body = MakePatternPtr<Nop>();
    r_equal->operands = (state_var_id, state_id);
    //r_body_comp->members = ();
    r_body_comp->statements = body;
    // TODO: with the condition, superloop is exiting before the last state block has run
    r_if2->condition = r_not_equal;
    r_if2->body = goto1;
    r_if2->else_body = MakePatternPtr<Nop>();
    r_not_equal->operands = (state_var_id, state_id);    
    
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


ApplyLabelPolicy::ApplyLabelPolicy()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto item_name = MakePatternPtr<StateLabel>();
    auto item_extra = MakePatternPtr<Label>();
    auto sx_post_label = MakePatternPtr<Label>();
    auto iif = MakePatternPtr<If>();
    auto equal = MakePatternPtr<Equal>();
    auto sx_post = MakePatternPtr< Negation<Statement> >();
    auto state_var_id = MakePatternPtr<InstanceIdentifier>();
    auto state_id = MakePatternPtr<InstanceIdentifier>();
    auto label_star = MakePatternPtr< Star<Label> >();
        
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
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto body1 = MakePatternPtr< Star<Statement> >();
    auto body2 = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto label = MakePatternPtr<Label>();
    auto sx_label = MakePatternPtr<Label>();
    auto sx_label2 = MakePatternPtr<Label>();
    auto sx_stmt = MakePatternPtr< Negation<Statement> >();
    auto r_if = MakePatternPtr<If>();
    auto r_equal = MakePatternPtr<Equal>();
    auto r_delta_count = MakePatternPtr<DeltaCount>();
    auto r_zero = MakePatternPtr<SpecificInteger>(0);
    auto wait = MakePatternPtr<Wait>();
    auto s_stuff = MakePatternPtr< Stuff<Compound> >();
    auto gotoo = MakePatternPtr<Goto>();
    auto sx_body1 = MakePatternPtr< Negation<Statement> >();
    auto sx_body2 = MakePatternPtr< Negation<Statement> >();
    auto sx_uncombable1 = MakePatternPtr<Uncombable>();
    auto sx_uncombable2 = MakePatternPtr<Uncombable>();
       
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
    r_if->else_body = MakePatternPtr<Nop>();
    
    Configure( SEARCH_REPLACE, s_all, r_comp );
}


EnsureResetYield::EnsureResetYield()
{
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto post = MakePatternPtr< Star<Statement> >();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto sx_any = MakePatternPtr< Disjunction<Statement> >();
    auto gotoo = MakePatternPtr<Goto>();
    auto r_yield = MakePatternPtr<WaitDelta>();
    
    s_comp->members = r_comp->members = (decls);
    s_comp->statements = (pre, gotoo, post);
    r_comp->statements = (pre, r_yield, gotoo, post);
    pre->restriction = sx_not;
    sx_not->negand = sx_any;
    sx_any->disjuncts = (MakePatternPtr<Goto>(), MakePatternPtr<Label>(), MakePatternPtr<Wait>() );
        
    Configure( SEARCH_REPLACE, s_comp, r_comp );
}


DetectSuperLoop::DetectSuperLoop( bool is_conditional_goto )
{
    auto inst = MakePatternPtr<Instance>();
    auto s_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto r_body_comp = MakePatternPtr<Compound>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto body = MakePatternPtr< Star<Statement> >();
    auto s_label = MakePatternPtr<Label>();
    auto s_ifgoto = MakePatternPtr<If>();
    auto s_goto = MakePatternPtr<Goto>();
    auto cond = MakePatternPtr<Expression>();
    auto sx_not = MakePatternPtr< Negation<Statement> >();
    auto r_do = MakePatternPtr<Do>();
    auto over = MakePatternPtr< Delta<Compound> >();
    
    auto slavell = MakePatternPtr< SlaveSearchReplace<Statement> >( r_body_comp, MakePatternPtr<Goto>(), MakePatternPtr<Continue>() );    
    
    inst->type = MakePatternPtr<Callable>();
    inst->initialiser = over;
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (s_label, body, is_conditional_goto 
                                         ? TreePtr<Statement>(s_ifgoto) 
                                         : TreePtr<Statement>(s_goto) );
    body->restriction = sx_not;
    sx_not->negand = MakePatternPtr<Label>(); // so s_label is the only one - all gotos must go to it.
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
    
    Configure( SEARCH_REPLACE, inst );
}


InsertInferredYield::InsertInferredYield()
{
    auto fn = MakePatternPtr<Instance>();
    auto fn_id = MakePatternPtr<InstanceIdentifier>();
    auto thread = MakePatternPtr<Thread>(); // Must be SC_THREAD since we introduce new yield here, only makes sense in SC_THREAD
    auto func_comp = MakePatternPtr<Compound>();
    auto s_comp = MakePatternPtr<Compound>();
    auto sx_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto func_decls = MakePatternPtr< Star<Declaration> >();
    auto loop_decls = MakePatternPtr< Star<Declaration> >();
    auto stmts = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto over = MakePatternPtr< Delta<Statement> >();
    auto flag_decl = MakePatternPtr<LocalVariable>();
    auto flag_id = MakePatternPtr<InstanceIdentifier>();
    auto r_yield = MakePatternPtr<WaitDelta>();
    auto loop = MakePatternPtr<Loop>();
    auto r_if = MakePatternPtr<If>();
    auto sx_if = MakePatternPtr<If>();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_notmatch = MakePatternPtr< Negation<Compound> >();
    auto r_not = MakePatternPtr< LogicalNot >();
    auto sx_not = MakePatternPtr< LogicalNot >();
    auto assign = MakePatternPtr< Assign >();
          
    fn->type = thread;
    fn->initialiser = func_comp;
    fn->identifier = fn_id;  
    func_comp->members = (func_decls);
    func_comp->statements = (loop);
    loop->body = over;
    over->through = s_comp;
    s_comp->members = (loop_decls);
    s_comp->statements = (stmts);
    stmts->restriction = MakePatternPtr<If>();
    
    over->overlay = r_comp;
    r_comp->members = (loop_decls);
    r_comp->statements = (stmts, r_yield);
    
    Configure( SEARCH_REPLACE, fn );            
}


