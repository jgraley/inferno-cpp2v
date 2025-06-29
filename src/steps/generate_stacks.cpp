/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"
#include "tree/sctree.hpp"
#include "inferno_agents.hpp"

using namespace CPPTree;
using namespace Steps;
using namespace SCTree;

/// Place return at end of void functions, if not already there
ExplicitiseReturn::ExplicitiseReturn()
{
    auto module = MakePatternNode<Module>();
    auto other_decls = MakePatternNode<Star<Declaration>>();
    
    auto fi = MakePatternNode< Instance >();
    auto s_comp = MakePatternNode<Compound>();
    auto sx_comp = MakePatternNode<Compound>();
    auto r_comp = MakePatternNode<Compound>();
    auto pre = MakePatternNode< Star<Statement> >();
    auto sx_pre = MakePatternNode< Star<Statement> >();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto sx_decls = MakePatternNode< Star<Declaration> >();
    auto over = MakePatternNode< Delta<Compound> >();
    auto sx_return = MakePatternNode<Return>();
    auto r_return = MakePatternNode<Return>();
    auto s_any = MakePatternNode< Disjunction<Callable> >();
    auto s_func = MakePatternNode<Function>();
    auto s_proc = MakePatternNode<Procedure>();
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_not = MakePatternNode< Negation<Compound> >();
        
    module->members = (fi, other_decls);
    
    fi->type = s_any;
    s_any->disjuncts = (s_func, s_proc, MakePatternNode<Subroutine>() );
    s_proc->params = MakePatternNode< Star<Parameter> >();
    s_func->params = MakePatternNode< Star<Parameter> >();
    s_func->return_type = MakePatternNode< Void >();
    fi->initialiser = over;
    s_comp->members = decls;
    s_comp->statements = (pre);
    over->through = s_all;
    s_all->conjuncts = (s_comp, s_not);
    s_not->negand = sx_comp;
    sx_comp->members = sx_decls;
    sx_comp->statements = (sx_pre, sx_return);
    
    over->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, r_return);
    r_return->return_value = MakePatternNode<Uninitialised>();
    
    Configure( SEARCH_REPLACE, module );
}    


UseTempForReturnValue::UseTempForReturnValue()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
    auto s_return = MakeTreeNode<Return>();
    auto s_and = MakeTreeNode< Conjunction<Expression> >();
    s_return->return_value = s_and;
    auto retval = MakePatternNode< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternNode<Type>();
    retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    auto cs_stuff = MakeTreeNode< Stuff<Expression> >(); // TODO the exclusion Stuff<GetDec<Automatic>> is too strong;
                                                                    // use Not<GetDec<Temp>>
    s_and->conjuncts = ( retval, cs_stuff );
    auto cs_id = MakePatternNode< TransformOf<InstanceIdentifier> >( &DeclarationOf::instance );
    cs_stuff->terminus = cs_id;
    auto cs_instance = MakeTreeNode<Instance>();
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
    auto r_sub_comp= MakeTreeNode<Compound>();
    auto r_newvar = MakeTreeNode< Temporary >();
    r_newvar->type = type;
    auto id = MakePatternNode<BuildInstanceIdentifierAgent>("temp_retval");
    r_newvar->identifier = id;
    r_newvar->initialiser = MakePatternNode<Uninitialised>();
    r_sub_comp->members = ( r_newvar );
    auto r_assign = MakeTreeNode<Assign>();
    r_assign->operands.push_back( id );
    r_assign->operands.push_back( retval );
    r_sub_comp->statements.push_back( r_assign );
    auto r_return = MakeTreeNode<Return>();
    r_sub_comp->statements.push_back( r_return );
    r_return->return_value = id;
       
    Configure( SEARCH_REPLACE, s_return, r_sub_comp );
}


ReturnViaTemp::ReturnViaTemp()
{
    auto s_module = MakePatternNode<Module>();
    auto r_module = MakePatternNode<Module>();
    auto func = MakePatternNode<Instance>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto cp = MakePatternNode<Function>();
    auto return_type = MakePatternNode< Negation<Type> >();
    auto sx_void = MakePatternNode<Void>();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto lr_comp = MakePatternNode<Compound>();
    auto statements = MakePatternNode< Star<Statement> >();
    auto locals = MakePatternNode< Star<Declaration> >();
    auto func_id = MakePatternNode<InstanceIdentifier>();
    auto module_id = MakePatternNode<TypeIdentifier>();
    auto params = MakePatternNode< Star<Parameter> >();
    auto m_call = MakePatternNode<Call>();
    auto m_any = MakePatternNode<Disjunction<Expression>>();
    auto m_lookup = MakePatternNode<Lookup>();
    auto m_operands = MakePatternNode< Star<MapOperand> >();
    auto r_temp = MakePatternNode<Temporary>();
    auto mr_assign = MakePatternNode<Assign>();
    auto lr_assign = MakePatternNode<Assign>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("%s_return");
    auto ls_return = MakePatternNode<Return>();
    auto lr_return = MakePatternNode<Return>();
    auto l_return_value = MakePatternNode<Expression>();
    auto mr_comp = MakePatternNode<StatementExpression>();
    auto ms_gg = MakePatternNode< GreenGrass<Call> >();
    auto overcp = MakePatternNode< Delta<Type> >();
    auto overi = MakePatternNode< Delta<Initialiser> >();
    
    auto embedded_l = MakePatternNode< EmbeddedSearchReplace<Compound> >( r_body, ls_return, lr_comp );
    ls_return->return_value = l_return_value; // note this also pre-restricts away Return<Uninitialised>
    lr_comp->statements = (lr_assign, lr_return);
    lr_assign->operands = (r_temp_id, l_return_value);
    lr_return->return_value = MakePatternNode<Uninitialised>(); // means no return value given

    ms_gg->through = m_call;
    m_call->callee = m_any;
    m_any->disjuncts = ( func_id, m_lookup );
    m_lookup->member = func_id;
    m_call->operands = (m_operands);
    mr_comp->statements = (m_call, r_temp_id);
    auto embedded_m = MakePatternNode< EmbeddedSearchReplace<Scope> >( r_module, ms_gg, mr_comp );
    
    s_module->members = (decls, func);
    r_module->members = (decls, func, r_temp);
    func->type = cp;
    func->initialiser = overi;
    overi->through = s_body;
    func->identifier = func_id;
    s_body->members = (locals);
    s_body->statements = (statements);
    cp->params = (params);  
    cp->return_type = overcp;
    overcp->through = return_type;
    return_type->negand = sx_void;
    overi->overlay = embedded_l;
    r_body->members = (locals);
    r_body->statements = (statements);
    overcp->overlay = MakePatternNode<Void>();
    r_temp->initialiser = MakePatternNode<Uninitialised>();
    r_temp->type = return_type;
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id);
    
    Configure( SEARCH_REPLACE, s_module, embedded_m );  
}


struct TempReturnAddress : Temporary
{
    NODE_FUNCTIONS_FINAL
};


AddLinkAddress::AddLinkAddress()
{
    auto s_module = MakePatternNode<Module>();
    auto r_module = MakePatternNode<Module>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto r_retaddr = MakePatternNode<Temporary>();
    auto r_retaddr_id = MakePatternNode<BuildInstanceIdentifierAgent>("%s_link");
    auto lr_retaddr = MakePatternNode<Parameter>();
    auto lr_retaddr_id = MakePatternNode<BuildInstanceIdentifierAgent>("link");
    auto lr_temp_retaddr = MakePatternNode<TempReturnAddress>();
    auto lr_temp_retaddr_id = MakePatternNode<BuildInstanceIdentifierAgent>("temp_link");
    auto s_nm = MakePatternNode< Negation<Declaration> >();
    auto ls_nm = MakePatternNode< Negation<Declaration> >();
    auto gg = MakePatternNode< GreenGrass<Declaration> >();
    auto l_inst = MakePatternNode<Instance>();
    auto l_over = MakePatternNode< Delta<Compound> >();
    auto ls_comp = MakePatternNode<Compound>();
    auto lr_comp = MakePatternNode<Compound>();
    auto l_decls = MakePatternNode< Star<Declaration> >();
    auto l_stmts = MakePatternNode< Star<Statement> >();
    auto msx_stmts = MakePatternNode< Star<Statement> >();
    auto msx_assign = MakePatternNode<Assign>();
    auto ms_call = MakePatternNode<Call>();
    auto mr_call = MakePatternNode<Call>();
    auto mr_comp = MakePatternNode<Compound>();
    auto msx_comp = MakePatternNode<Compound>();
    auto mr_label = MakePatternNode<Label>();
    auto mr_labelid = MakePatternNode<BuildLabelIdentifierAgent>("LINK");
    auto m_all = MakePatternNode< Conjunction<Statement> >();
    auto m_any = MakePatternNode< Child<Statement> >(); // TODO rename Child -> Blob
    auto ms_not = MakePatternNode< Negation<Statement> >();
    auto m_over = MakePatternNode< Delta<Statement> >();
    auto l_func_over = MakePatternNode< Delta<Function> >();
    auto ls_func = MakePatternNode<Function>();
    auto lr_func = MakePatternNode<Function>();
    auto ident = MakePatternNode<TypeIdentifier>();
    auto l_inst_id = MakePatternNode<InstanceIdentifier>();
    auto ll_return = MakePatternNode<Return>();
    auto llr_comp = MakePatternNode<Compound>();
    auto llsx_comp = MakePatternNode<Compound>();
    auto llr_assign = MakePatternNode<Assign>();
    auto llsx_assign = MakePatternNode<Assign>();
    auto ll_all = MakePatternNode< Conjunction<Statement> >();
    auto ll_any = MakePatternNode< Child<Statement> >();
    auto lls_not = MakePatternNode< Negation<Statement> >();
    auto ll_over = MakePatternNode< Delta<Statement> >();
    auto m_gg = MakePatternNode< GreenGrass<Statement> >();
    auto ll_gg = MakePatternNode< GreenGrass<Statement> >();
    auto mr_operand = MakePatternNode<MapOperand>();

    ll_gg->through = ll_return;
    ll_over->overlay = llr_comp;        
    //llr_comp->members = ();
    llr_comp->statements = (llr_assign, ll_return);
    llr_assign->operands = (lr_temp_retaddr_id, lr_retaddr_id);
   
    auto embedded_ll = MakePatternNode< EmbeddedSearchReplace<Compound> >( lr_comp, ll_gg, llr_comp );   
   
    m_gg->through = ms_call;
    ms_call->operands = (MakePatternNode< Star<MapOperand> >());
    ms_call->callee = l_inst_id;
    mr_comp->statements = (mr_call, mr_label);  
    mr_call->operands = (ms_call->operands, mr_operand);
    mr_call->callee = l_inst_id;
    mr_operand->identifier = lr_retaddr_id;
    mr_operand->value = mr_labelid;
    mr_label->identifier = mr_labelid;    
    
    auto embedded_m = MakePatternNode< EmbeddedSearchReplace<Scope> >( r_module, m_gg, mr_comp );
    
    l_inst->type = l_func_over;
    l_func_over->through = ls_func;
    l_func_over->overlay = lr_func;    
    ls_func->return_type = MakePatternNode<Void>();
    ls_func->params = MakePatternNode< Star<Parameter> >(); // Params OK here, just not in MergeFunctions
    lr_func->return_type = ls_func->return_type;
    lr_func->params = (ls_func->params, lr_retaddr);
    l_inst->initialiser = l_over;
    l_inst->identifier = l_inst_id;
    l_over->through = ls_comp;
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_stmts);
    l_over->overlay = embedded_ll;
    lr_comp->members = (l_decls, lr_temp_retaddr);
    lr_retaddr->identifier = lr_retaddr_id;
    lr_retaddr->type = MakePatternNode<Labeley>();
    lr_retaddr->initialiser = MakePatternNode<Uninitialised>();
    lr_temp_retaddr->identifier = lr_temp_retaddr_id;
    lr_temp_retaddr->type = MakePatternNode<Labeley>();
    lr_temp_retaddr->initialiser = MakePatternNode<Uninitialised>();
    lr_comp->statements = (l_stmts);
    
    s_module->members = (gg, decls);
    r_module->members = (gg, decls, r_retaddr);
    gg->through = l_inst;
    r_retaddr->identifier = r_retaddr_id;
    r_retaddr->type = MakePatternNode<Labeley>();
    r_retaddr->initialiser = MakePatternNode<Uninitialised>();
    //r_retaddr->virt = MakePatternNode<NonVirtual>();
    //r_retaddr->access = MakePatternNode<Private>();
    //r_retaddr->constancy = MakePatternNode<NonConst>();
    r_retaddr_id->sources = (l_inst_id);
    Configure( SEARCH_REPLACE, s_module, embedded_m );  
}


ParamsViaTemps::ParamsViaTemps()
{
    auto s_module = MakePatternNode<Module>();
    auto r_module = MakePatternNode<Module>();
    auto s_func = MakePatternNode<Instance>();
    auto r_func = MakePatternNode<Instance>();
    auto decls = MakePatternNode< Star<Declaration> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto s_cp = MakePatternNode<Function>();
    auto r_cp = MakePatternNode<Function>();
    auto return_type = MakePatternNode<Type>();
    auto s_body = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Compound>();
    auto mr_comp = MakePatternNode<Compound>();
    auto statements = MakePatternNode< Star<Statement> >();
    auto locals = MakePatternNode< Star<Declaration> >();
    auto func_id = MakePatternNode<InstanceIdentifier>();
    auto param_id = MakePatternNode<InstanceIdentifier>();
    auto module_id = MakePatternNode<TypeIdentifier>();
    auto s_param = MakePatternNode<Parameter>();
    auto params = MakePatternNode< Star<Parameter> >();
    auto ms_call = MakePatternNode<Call>();
    auto mr_call = MakePatternNode<Call>();
    auto ms_operand = MakePatternNode<MapOperand>();
    auto m_operands = MakePatternNode< Star<MapOperand> >();
    auto r_param = MakePatternNode<Automatic>();
    auto param_type = MakePatternNode<Type>();
    auto r_temp = MakePatternNode<Temporary>();
    auto mr_assign = MakePatternNode<Assign>();
    auto m_expr = MakePatternNode<Expression>();
    auto r_temp_id = MakePatternNode<BuildInstanceIdentifierAgent>("%s_%s");
    auto over = MakePatternNode< Delta<Declaration> >();
    
    ms_call->callee = func_id;
    ms_call->operands = (m_operands, ms_operand);
    ms_operand->identifier = param_id;
    ms_operand->value = m_expr;
    mr_comp->statements = (mr_assign, mr_call);
    mr_assign->operands = (r_temp_id, m_expr);
    mr_call->callee = func_id;
    mr_call->operands = (m_operands);
    auto embedded_m = MakePatternNode< EmbeddedSearchReplace<Scope> >( r_module, ms_call, mr_comp );
    
    s_module->members = (decls, over);
    r_module->members = (decls, over, r_temp);
    over->through = s_func;
    over->overlay = r_func;
    s_func->type = s_cp;
    r_func->type = r_cp;
    s_func->initialiser = s_body;
    r_func->initialiser = r_body;
    s_func->identifier = func_id;
    r_func->identifier = func_id;
    s_body->members = (locals);
    r_body->members = (locals);
    s_body->statements = (statements);
    r_body->statements = (r_param, statements);
    s_cp->params = (params, s_param);  
    s_cp->return_type = return_type;
    s_param->identifier = param_id;
    s_param->type = param_type;
    r_param->type = param_type;
    r_param->initialiser = r_temp_id;
    r_param->identifier = param_id;
    r_cp->params = (params);
    r_cp->return_type = return_type;
    r_temp->type = param_type;
    r_temp->initialiser = MakePatternNode<Uninitialised>();
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id, param_id);
    
    Configure( SEARCH_REPLACE, s_module, embedded_m );  
}


GenerateStacks::GenerateStacks()
{
    // Search for a function and require it to have at least one automatic
    // variable using an and rule. Add a stack index variable (static) to the 
    // function. Increment at the beginning of the function body and decrement 
    // at the end. 
    //
    // Use an embedded pattern to find automatic variables in the function. Replace them
    // with arrays of the same type. Using another embedded pattern, add a decrement of
    // the stack pointer before any return statements.
    //
    // Using a sub-embedded pattern of the variable-finding pattern, look for usages of 
    // the variable. Replace with an indexing operation into the array using
    // the stack index.    
    auto s_fi = MakePatternNode<Instance>();
    auto r_fi = MakePatternNode<Instance>();
    auto l_fi = MakePatternNode<Instance>();
    auto sx_thread = MakePatternNode<Thread>();
    auto sx_method = MakePatternNode<Method>();
    auto sx_any = MakePatternNode< Disjunction<Type> >();
    auto s_not = MakePatternNode< Negation<Type> >();
    auto s_and = MakePatternNode< Conjunction<Initialiser> >();
    auto s_top_comp = MakePatternNode<Compound>();
    auto r_top_comp = MakePatternNode<Compound>();
    auto r_ret_comp = MakePatternNode<Compound>();
    auto temp = MakePatternNode<Compound>();
    auto top_decls = MakePatternNode< Star<Declaration> >();
    auto top_pre = MakePatternNode< Star<Statement> >();
    auto stuff = MakePatternNode< Stuff<Initialiser> >();
    auto cs_stuff = MakePatternNode< Stuff<Compound> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto over = MakePatternNode< Delta<Declaration> >();
    auto cs_instance = MakePatternNode<Automatic>();
    auto s_instance = MakePatternNode<Automatic>();
    auto r_index = MakePatternNode<Field>();
    auto r_instance = MakePatternNode<Field>();
    auto r_index_type = MakePatternNode<Unsigned>();
    auto r_inc = MakePatternNode<PostIncrement>();
    auto r_ret_dec = MakePatternNode<PostDecrement>();
    auto s_identifier = MakePatternNode<InstanceIdentifier>();
    auto r_array = MakePatternNode<Array>();
    auto ret = MakePatternNode<Return>();
    auto l_r_sub = MakePatternNode<Subscript>();
    auto s_and3 = MakePatternNode< Conjunction<Node> >();
    auto r_index_identifier = MakePatternNode<BuildInstanceIdentifierAgent>("%s_stack_index");
    auto r_identifier = MakePatternNode<BuildInstanceIdentifierAgent>("%s_stack");
    auto s_gg = MakePatternNode< GreenGrass<Statement> >();
    auto r_index_init = MakePatternNode<Assign>();
    auto members = MakePatternNode< Star<Declaration> >();
    auto s_module = MakePatternNode<Scope>();
    auto r_module = MakePatternNode<Scope>();
    auto ls_module = MakePatternNode<Scope>();
    auto lr_module = MakePatternNode<Scope>();
    auto bases = MakePatternNode< Star<Base> >();
    auto l_bases = MakePatternNode< Star<Base> >();
    auto module_id = MakePatternNode<TypeIdentifier>();
    auto l_module_id = MakePatternNode<TypeIdentifier>();
    auto s_vcomp = MakePatternNode<Compound>();
    auto r_vcomp = MakePatternNode<Compound>();
    auto vdecls = MakePatternNode< Star<Declaration> >();
    auto l_members = MakePatternNode< Star<Declaration> >();
    auto vstmts = MakePatternNode< Star<Statement> >();
    auto fi_id = MakePatternNode<InstanceIdentifier>();

    // Sub-embedded pattern replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Statement> >( r_vcomp, s_identifier, l_r_sub );

    // EmbeddedSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_vcomp;
    s_vcomp->members = (vdecls, s_instance);
    s_vcomp->statements = (vstmts);
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakePatternNode<Uninitialised>(); 
    s_instance->type = MakePatternNode<Type>();

    // EmbeddedSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakePatternNode<NonConst>();
    r_instance->initialiser = MakePatternNode<Uninitialised>();
    overlay->overlay = r_embedded;
    r_vcomp->members = (vdecls);
    r_vcomp->statements = (vstmts);
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
    r_instance->virt = MakePatternNode<NonVirtual>();
    r_instance->access = MakePatternNode<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakePatternNode<SpecificInteger>(10);

    // EmbeddedSearchReplace to find early returns in the function
    s_gg->through = ret;

    // EmbeddedSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    ls_module->members = (l_members, l_fi);
    lr_module->members = (l_members, l_fi, r_instance);
    l_fi->initialiser = stuff;
    l_fi->identifier = fi_id;
    
    auto r_mid = MakePatternNode< EmbeddedCompareReplace<Scope> >( r_module, ls_module, lr_module ); // stuff, stuff

    auto r_embedded_3 = MakePatternNode< EmbeddedSearchReplace<Statement> >( r_top_comp, s_gg, r_ret_comp );
    temp->statements = (r_embedded_3);
    
    // Master search - look for functions satisfying the construct limitation and get
    s_module->members = (over, members); // #580 here
    r_module->members = (over, members, r_index);
    over->through = s_fi;
    over->overlay = r_fi;    
    s_fi->identifier = r_fi->identifier = fi_id;
    s_fi->type = r_fi->type = s_not;
    s_not->negand = sx_any;
    sx_any->disjuncts = (sx_thread, sx_method); // Do not provide stacks for these because they do not recurse
    s_fi->initialiser = s_and;   
    r_fi->initialiser = temp;   
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict top-level search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->conjuncts = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    // top-level decls
    r_top_comp->members = (top_decls);
    r_index->type = r_index_type;
    r_index_type->width = MakePatternNode<SpecificInteger>(32);
    r_index_identifier->sources = (fi_id);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakePatternNode<NonConst>();
    r_index->initialiser = MakePatternNode<SpecificInteger>(0);
    //r_index->initialiser = MakePatternNode<SpecificInteger>(0);
    r_index_init->operands = (r_index_identifier, MakePatternNode<SpecificInteger>(0));
    r_index->virt = MakePatternNode<NonVirtual>();
    r_index->access = MakePatternNode<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_inc, top_pre );

    Configure( SEARCH_REPLACE, s_module, r_mid );
}

/*
OLD VERSION
GenerateStacks::GenerateStacks()
{
    // Search for a function and require it to have at least one automatic
    // variable using an and rule. Add a stack index variable (static) to the 
    // function. Increment at the beginning of the function body and decrement 
    // at the end. 
    //
    // Use an embedded pattern to find automatic variables in the function. Replace them
    // with arrays of the same type. Using another embedded pattern, add a decrement of
    // the stack pointer before any return statements.
    //
    // Using a sub-embedded pattern of the variable-finding pattern, look for usages of 
    // the variable. Replace with an indexing operation into the array using
    // the stack index.    
    auto fi = MakePatternNode<Instance>();
    auto oinit = MakePatternNode< Delta<Initialiser> >();
    auto s_func = MakePatternNode<Callable>();
    auto s_and = MakePatternNode< Conjunction<Initialiser> >();
    auto s_top_comp = MakePatternNode<Compound>();
    auto r_top_comp = MakePatternNode<Compound>();
    auto r_ret_comp = MakePatternNode<Compound>();
    auto temp = MakePatternNode<Compound>();
    auto top_decls = MakePatternNode< Star<Declaration> >();
    auto top_pre = MakePatternNode< Star<Statement> >();
    auto stuff = MakePatternNode< Stuff<Statement> >();
    auto cs_stuff = MakePatternNode< Stuff<Compound> >();
    auto overlay = MakePatternNode< Delta<Statement> >();
    auto cs_instance = MakePatternNode<Automatic>();
    auto s_instance = MakePatternNode<Automatic>();
    auto r_index = MakePatternNode<Static>();
    auto r_instance = MakePatternNode<Static>();
    auto r_index_type = MakePatternNode<Unsigned>();
    auto r_inc = MakePatternNode<PostIncrement>();
    auto r_ret_dec = MakePatternNode<PostDecrement>();
    auto s_identifier = MakePatternNode<InstanceIdentifier>();
    auto r_array = MakePatternNode<Array>();
    auto ret = MakePatternNode<Return>();
    auto l_r_sub = MakePatternNode<Subscript>();
    auto s_and3 = MakePatternNode< Conjunction<Node> >();
    auto r_index_identifier = MakePatternNode<BuildInstanceIdentifierAgent>("%s_stack_index");
    auto r_identifier = MakePatternNode<BuildInstanceIdentifierAgent>("%s_stack");
    auto s_gg = MakePatternNode< GreenGrass<Statement> >();

    // Master search - look for functions satisfying the construct limitation and get
    fi->identifier = MakePatternNode<InstanceIdentifier>();
    fi->type = s_func;
    fi->initialiser = oinit;   
    oinit->through = s_and;
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict top-level search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->conjuncts = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    auto r_embedded = MakePatternNode< EmbeddedSearchReplace<Statement> >( stuff, s_identifier, l_r_sub );
    auto r_mid = MakePatternNode< EmbeddedCompareReplace<Statement> >( r_top_comp, stuff, r_embedded );
    auto r_embedded_3 = MakePatternNode< EmbeddedSearchReplace<Statement> >( r_mid, s_gg, r_ret_comp );
    temp->statements = (r_embedded_3);
    oinit->overlay = temp;//r_embedded_3; 

    // top-level decls
    r_top_comp->members = ( top_decls, r_index );
    r_index->type = r_index_type;
    r_index_type->width = MakePatternNode<SpecificInteger>(32);
    r_index_identifier->sources = (fi->identifier);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakePatternNode<NonConst>();
    r_index->initialiser = MakePatternNode<SpecificInteger>(0);
//    r_index->virt = MakePatternNode<NonVirtual>();
//    r_index->access = MakePatternNode<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_inc, top_pre );

    // EmbeddedSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_instance;
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakePatternNode<Uninitialised>(); // can't handle initialisers!
    s_instance->type = MakePatternNode<Type>();

    // EmbeddedSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakePatternNode<NonConst>();
    r_instance->initialiser = MakePatternNode<Uninitialised>();
    overlay->overlay = r_instance;
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
//    r_instance->virt = MakePatternNode<NonVirtual>();
//    r_instance->access = MakePatternNode<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakePatternNode<SpecificInteger>(10);

    // Sub-embedded pattern replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    // EmbeddedSearchReplace to find early returns in the function
    s_gg->through = ret;

    // EmbeddedSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    Configure( SEARCH_REPLACE, fi );
}

*/


MergeFunctions::MergeFunctions()
{
    auto s_module = MakePatternNode<Scope>();
    auto r_module = MakePatternNode<Scope>();
    auto thread = MakePatternNode<Field>();
    auto s_func = MakePatternNode<Field>();
    auto thread_type = MakePatternNode<Thread>();
    auto func_type = MakePatternNode<Callable>();
    auto members = MakePatternNode< Star<Declaration> >();
    auto thread_decls = MakePatternNode< Star<Declaration> >();
    auto thread_stmts = MakePatternNode< Star<Statement> >();
    auto bases = MakePatternNode< Star<Base> >();
    auto thread_over = MakePatternNode< Delta<Compound> >();
    auto s_thread_comp = MakePatternNode<Compound>();
    auto r_thread_comp = MakePatternNode<Compound>();
    auto s_call = MakePatternNode<Call>();
    auto ls_call = MakePatternNode<Call>();
    auto func_id = MakePatternNode<InstanceIdentifier>();
    auto r_label = MakePatternNode<Label>();
    auto r_label_id = MakePatternNode< BuildLabelIdentifierAgent >("ENTER_%s");
    auto s_all = MakePatternNode< Conjunction<Compound> >();
    auto s_stuff = MakePatternNode< Stuff<Compound> >();
    auto func_stuff = MakePatternNode< Stuff<Compound> >();
    auto module_id = MakePatternNode<TypeIdentifier>();
    auto lr_goto = MakePatternNode<Goto>();
    auto mr_goto = MakePatternNode<Goto>();
    auto ms_return = MakePatternNode<Return>();
    auto retaddr = MakePatternNode<TempReturnAddress>();
    auto retaddr_id = MakePatternNode<InstanceIdentifier>();
    
    mr_goto->destination = retaddr_id;
     
    auto embedded_m = MakePatternNode< EmbeddedSearchReplace<Compound> >( func_stuff, ms_return, mr_goto );        
    
    ls_call->callee = func_id;
    lr_goto->destination = r_label_id;
        
    auto embedded_l = MakePatternNode< EmbeddedSearchReplace<Compound> >( r_thread_comp, ls_call, lr_goto );    
    
    s_module->members = (members, thread, s_func);
    r_module->members = (members, thread);
    thread->type = thread_type;
    thread->initialiser = thread_over;
    thread_over->through = s_all;
    s_all->conjuncts = (s_thread_comp, s_stuff);
    s_stuff->terminus = s_call;
    s_thread_comp->members = (thread_decls);
    s_thread_comp->statements = (thread_stmts);
    s_call->callee = func_id;    
    s_func->type = func_type;
    s_func->initialiser = func_stuff;
    s_func->identifier = func_id;
    func_stuff->terminus = retaddr;
    retaddr->identifier = retaddr_id;
    thread_over->overlay = embedded_l;
    r_thread_comp->members = (thread_decls);
    r_thread_comp->statements = (thread_stmts, r_label, embedded_m);
    r_label->identifier = r_label_id;
    r_label_id->sources = (func_id);
         
    Configure( SEARCH_REPLACE, s_module, r_module );
}


