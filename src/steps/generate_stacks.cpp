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
    auto fi = MakePatternPtr< Instance >();
    auto s_comp = MakePatternPtr<Compound>();
    auto sx_comp = MakePatternPtr<Compound>();
    auto r_comp = MakePatternPtr<Compound>();
    auto pre = MakePatternPtr< Star<Statement> >();
    auto sx_pre = MakePatternPtr< Star<Statement> >();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto sx_decls = MakePatternPtr< Star<Declaration> >();
    auto over = MakePatternPtr< Delta<Compound> >();
    auto sx_return = MakePatternPtr<Return>();
    auto r_return = MakePatternPtr<Return>();
    auto s_any = MakePatternPtr< Disjunction<Callable> >();
    auto s_func = MakePatternPtr<Function>();
    auto s_proc = MakePatternPtr<Procedure>();
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_not = MakePatternPtr< Negation<Compound> >();
        
    fi->type = s_any;
    s_any->disjuncts = (s_func, s_proc, MakePatternPtr<Subroutine>() );
    s_proc->members = MakePatternPtr< Star<Declaration> >();
    s_func->members = MakePatternPtr< Star<Declaration> >();
    s_func->return_type = MakePatternPtr< Void >();
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
    r_return->return_value = MakePatternPtr<Uninitialised>();
    
    Configure( SEARCH_REPLACE, fi );
}    


UseTempForReturnValue::UseTempForReturnValue()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
    TreePtr<Return> s_return( new Return );
    TreePtr< Conjunction<Expression> > s_and( new Conjunction<Expression> );
    s_return->return_value = s_and;
    auto retval = MakePatternPtr< TransformOf<Expression> >( &TypeOf::instance );
    auto type = MakePatternPtr<Type>();
    retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    TreePtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> ); // TODO the exclusion Stuff<GetDec<Automatic>> is too strong;
                                                                    // use Not<GetDec<Temp>>
    s_and->conjuncts = ( retval, cs_stuff );
    auto cs_id = MakePatternPtr< TransformOf<InstanceIdentifier> >( &GetDeclaration::instance );
    cs_stuff->terminus = cs_id;
    TreePtr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
    TreePtr<Compound> r_sub_comp( new Compound );
    TreePtr< Temporary > r_newvar( new Temporary );
    r_newvar->type = type;
    auto id = MakePatternPtr<BuildInstanceIdentifierAgent>("temp_retval");
    r_newvar->identifier = id;
    r_newvar->initialiser = MakePatternPtr<Uninitialised>();
    r_sub_comp->members = ( r_newvar );
    TreePtr<Assign> r_assign( new Assign );
    r_assign->operands.push_back( id );
    r_assign->operands.push_back( retval );
    r_sub_comp->statements.push_back( r_assign );
    TreePtr<Return> r_return( new Return );
    r_sub_comp->statements.push_back( r_return );
    r_return->return_value = id;
       
    Configure( SEARCH_REPLACE, s_return, r_sub_comp );
}


ReturnViaTemp::ReturnViaTemp()
{
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto func = MakePatternPtr<Instance>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto cp = MakePatternPtr<Function>();
    auto return_type = MakePatternPtr< Negation<Type> >();
    auto sx_void = MakePatternPtr<Void>();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto lr_comp = MakePatternPtr<Compound>();
    auto statements = MakePatternPtr< Star<Statement> >();
    auto locals = MakePatternPtr< Star<Declaration> >();
    auto func_id = MakePatternPtr<InstanceIdentifier>();
    auto module_id = MakePatternPtr<TypeIdentifier>();
    auto params = MakePatternPtr< Star<Instance> >();
    auto m_call = MakePatternPtr<Call>();
    auto m_operands = MakePatternPtr< Star<MapOperand> >();
    auto r_temp = MakePatternPtr<Temporary>();
    auto mr_assign = MakePatternPtr<Assign>();
    auto lr_assign = MakePatternPtr<Assign>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_return");
    auto ls_return = MakePatternPtr<Return>();
    auto lr_return = MakePatternPtr<Return>();
    auto l_return_value = MakePatternPtr<Expression>();
    auto mr_comp = MakePatternPtr<StatementExpression>();
    auto ms_gg = MakePatternPtr< GreenGrass<Call> >();
    auto overcp = MakePatternPtr< Delta<Type> >();
    auto overi = MakePatternPtr< Delta<Initialiser> >();
    
    auto slavel = MakePatternPtr< SlaveSearchReplace<Compound> >( r_body, ls_return, lr_comp );
    ls_return->return_value = l_return_value; // note this also pre-restricts away Return<Uninitialised>
    lr_comp->statements = (lr_assign, lr_return);
    lr_assign->operands = (r_temp_id, l_return_value);
    lr_return->return_value = MakePatternPtr<Uninitialised>(); // means no return value given

    ms_gg->through = m_call;
    m_call->callee = func_id;
    m_call->operands = (m_operands);
    mr_comp->statements = (m_call, r_temp_id);
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( r_module, ms_gg, mr_comp );
    
    s_module->members = (decls, func);
    r_module->members = (decls, func, r_temp);
    func->type = cp;
    func->initialiser = overi;
    overi->through = s_body;
    func->identifier = func_id;
    s_body->members = (locals);
    s_body->statements = (statements);
    cp->members = (params);  
    cp->return_type = overcp;
    overcp->through = return_type;
    return_type->negand = sx_void;
    overi->overlay = slavel;
    r_body->members = (locals);
    r_body->statements = (statements);
    overcp->overlay = MakePatternPtr<Void>();
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_temp->type = return_type;
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id);
    
    Configure( SEARCH_REPLACE, s_module, slavem );  
}


struct TempReturnAddress : Temporary
{
    NODE_FUNCTIONS_FINAL
};


AddLinkAddress::AddLinkAddress()
{
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto r_retaddr = MakePatternPtr<Temporary>();
    auto r_retaddr_id = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_link");
    auto lr_retaddr = MakePatternPtr<Automatic>();
    auto lr_retaddr_id = MakePatternPtr<BuildInstanceIdentifierAgent>("link");
    auto lr_temp_retaddr = MakePatternPtr<TempReturnAddress>();
    auto lr_temp_retaddr_id = MakePatternPtr<BuildInstanceIdentifierAgent>("temp_link");
    auto s_nm = MakePatternPtr< Negation<Declaration> >();
    auto ls_nm = MakePatternPtr< Negation<Declaration> >();
    auto gg = MakePatternPtr< GreenGrass<Declaration> >();
    auto l_inst = MakePatternPtr<Instance>();
    auto l_over = MakePatternPtr< Delta<Compound> >();
    auto ls_comp = MakePatternPtr<Compound>();
    auto lr_comp = MakePatternPtr<Compound>();
    auto l_decls = MakePatternPtr< Star<Declaration> >();
    auto l_stmts = MakePatternPtr< Star<Statement> >();
    auto msx_stmts = MakePatternPtr< Star<Statement> >();
    auto msx_assign = MakePatternPtr<Assign>();
    auto ms_call = MakePatternPtr<Call>();
    auto mr_call = MakePatternPtr<Call>();
    auto mr_comp = MakePatternPtr<Compound>();
    auto msx_comp = MakePatternPtr<Compound>();
    auto mr_label = MakePatternPtr<Label>();
    auto mr_labelid = MakePatternPtr<BuildLabelIdentifierAgent>("LINK");
    auto m_all = MakePatternPtr< Conjunction<Statement> >();
    auto m_any = MakePatternPtr< AnyNode<Statement> >(); // TODO rename AnyNode -> Blob
    auto ms_not = MakePatternPtr< Negation<Statement> >();
    auto m_over = MakePatternPtr< Delta<Statement> >();
    auto l_func_over = MakePatternPtr< Delta<Function> >();
    auto ls_func = MakePatternPtr<Function>();
    auto lr_func = MakePatternPtr<Function>();
    auto ident = MakePatternPtr<TypeIdentifier>();
    auto l_inst_id = MakePatternPtr<InstanceIdentifier>();
    auto ll_return = MakePatternPtr<Return>();
    auto llr_comp = MakePatternPtr<Compound>();
    auto llsx_comp = MakePatternPtr<Compound>();
    auto llr_assign = MakePatternPtr<Assign>();
    auto llsx_assign = MakePatternPtr<Assign>();
    auto ll_all = MakePatternPtr< Conjunction<Statement> >();
    auto ll_any = MakePatternPtr< AnyNode<Statement> >();
    auto lls_not = MakePatternPtr< Negation<Statement> >();
    auto ll_over = MakePatternPtr< Delta<Statement> >();
    auto m_gg = MakePatternPtr< GreenGrass<Statement> >();
    auto ll_gg = MakePatternPtr< GreenGrass<Statement> >();
    auto mr_operand = MakePatternPtr<MapOperand>();

    ll_gg->through = ll_return;
    ll_over->overlay = llr_comp;        
    //llr_comp->members = ();
    llr_comp->statements = (llr_assign, ll_return);
    llr_assign->operands = (lr_temp_retaddr_id, lr_retaddr_id);
   
    auto slavell = MakePatternPtr< SlaveSearchReplace<Compound> >( lr_comp, ll_gg, llr_comp );   
   
    m_gg->through = ms_call;
    ms_call->operands = (MakePatternPtr< Star<MapOperand> >());
    ms_call->callee = l_inst_id;
    mr_comp->statements = (mr_call, mr_label);  
    mr_call->operands = (ms_call->operands, mr_operand);
    mr_call->callee = l_inst_id;
    mr_operand->identifier = lr_retaddr_id;
    mr_operand->value = mr_labelid;
    mr_label->identifier = mr_labelid;    
    
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( r_module, m_gg, mr_comp );
    
    l_inst->type = l_func_over;
    l_func_over->through = ls_func;
    l_func_over->overlay = lr_func;    
    ls_func->return_type = MakePatternPtr<Void>();
    ls_func->members = MakePatternPtr< Star<Instance> >(); // Params OK here, just not in MergeFunctions
    lr_func->return_type = ls_func->return_type;
    lr_func->members = (ls_func->members, lr_retaddr);
    l_inst->initialiser = l_over;
    l_inst->identifier = l_inst_id;
    l_over->through = ls_comp;
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_stmts);
    l_over->overlay = slavell;
    lr_comp->members = (l_decls, lr_temp_retaddr);
    lr_retaddr->identifier = lr_retaddr_id;
    lr_retaddr->type = MakePatternPtr<Labeley>();
    lr_retaddr->initialiser = MakePatternPtr<Uninitialised>();
    lr_temp_retaddr->identifier = lr_temp_retaddr_id;
    lr_temp_retaddr->type = MakePatternPtr<Labeley>();
    lr_temp_retaddr->initialiser = MakePatternPtr<Uninitialised>();
    lr_comp->statements = (l_stmts);
    
    s_module->members = (gg, decls);
    r_module->members = (gg, decls, r_retaddr);
    gg->through = l_inst;
    r_retaddr->identifier = r_retaddr_id;
    r_retaddr->type = MakePatternPtr<Labeley>();
    r_retaddr->initialiser = MakePatternPtr<Uninitialised>();
    //r_retaddr->virt = MakePatternPtr<NonVirtual>();
    //r_retaddr->access = MakePatternPtr<Private>();
    //r_retaddr->constancy = MakePatternPtr<NonConst>();
    r_retaddr_id->sources = (l_inst_id);
    Configure( SEARCH_REPLACE, s_module, slavem );  
}


ParamsViaTemps::ParamsViaTemps()
{
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto s_func = MakePatternPtr<Instance>();
    auto r_func = MakePatternPtr<Instance>();
    auto decls = MakePatternPtr< Star<Declaration> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto s_cp = MakePatternPtr<Function>();
    auto r_cp = MakePatternPtr<Function>();
    auto return_type = MakePatternPtr<Type>();
    auto s_body = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Compound>();
    auto mr_comp = MakePatternPtr<Compound>();
    auto statements = MakePatternPtr< Star<Statement> >();
    auto locals = MakePatternPtr< Star<Declaration> >();
    auto func_id = MakePatternPtr<InstanceIdentifier>();
    auto param_id = MakePatternPtr<InstanceIdentifier>();
    auto module_id = MakePatternPtr<TypeIdentifier>();
    auto s_param = MakePatternPtr<Instance>();
    auto params = MakePatternPtr< Star<Instance> >();
    auto ms_call = MakePatternPtr<Call>();
    auto mr_call = MakePatternPtr<Call>();
    auto ms_operand = MakePatternPtr<MapOperand>();
    auto m_operands = MakePatternPtr< Star<MapOperand> >();
    auto r_param = MakePatternPtr<Automatic>();
    auto param_type = MakePatternPtr<Type>();
    auto r_temp = MakePatternPtr<Temporary>();
    auto mr_assign = MakePatternPtr<Assign>();
    auto m_expr = MakePatternPtr<Expression>();
    auto r_temp_id = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_%s");
    auto over = MakePatternPtr< Delta<Declaration> >();
    
    ms_call->callee = func_id;
    ms_call->operands = (m_operands, ms_operand);
    ms_operand->identifier = param_id;
    ms_operand->value = m_expr;
    mr_comp->statements = (mr_assign, mr_call);
    mr_assign->operands = (r_temp_id, m_expr);
    mr_call->callee = func_id;
    mr_call->operands = (m_operands);
    auto slavem = MakePatternPtr< SlaveSearchReplace<Scope> >( r_module, ms_call, mr_comp );
    
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
    s_cp->members = (params, s_param);  
    s_cp->return_type = return_type;
    s_param->identifier = param_id;
    s_param->type = param_type;
    r_param->type = param_type;
    r_param->initialiser = r_temp_id;
    r_param->identifier = param_id;
    r_cp->members = (params);
    r_cp->return_type = return_type;
    r_temp->type = param_type;
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id, param_id);
    
    Configure( SEARCH_REPLACE, s_module, slavem );  
}


GenerateStacks::GenerateStacks()
{
    // Search for a function and require it to have at least one automatic
    // variable using an and rule. Add a stack index variable (static) to the 
    // function. Increment at the beginning of the function body and decrement 
    // at the end. 
    //
    // Use a slave to find automatic variables in the function. Replace them
    // with arrays of the same type. Using another slave, add a decrement of
    // the stack pointer before any return statements.
    //
    // Using a sub-slave of the variable-finding slave, look for usages of 
    // the variable. Replace with an indexing operation into the array using
    // the stack index.    
    auto s_fi = MakePatternPtr<Instance>();
    auto r_fi = MakePatternPtr<Instance>();
    auto l_fi = MakePatternPtr<Instance>();
    auto sx_thread = MakePatternPtr<Thread>();
    auto sx_method = MakePatternPtr<Method>();
    auto sx_any = MakePatternPtr< Disjunction<Type> >();
    auto s_not = MakePatternPtr< Negation<Type> >();
    auto s_and = MakePatternPtr< Conjunction<Initialiser> >();
    auto s_top_comp = MakePatternPtr<Compound>();
    auto r_top_comp = MakePatternPtr<Compound>();
    auto r_ret_comp = MakePatternPtr<Compound>();
    auto temp = MakePatternPtr<Compound>();
    auto top_decls = MakePatternPtr< Star<Declaration> >();
    auto top_pre = MakePatternPtr< Star<Statement> >();
    auto stuff = MakePatternPtr< Stuff<Initialiser> >();
    auto cs_stuff = MakePatternPtr< Stuff<Compound> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto over = MakePatternPtr< Delta<Declaration> >();
    auto cs_instance = MakePatternPtr<Automatic>();
    auto s_instance = MakePatternPtr<Automatic>();
    auto r_index = MakePatternPtr<Field>();
    auto r_instance = MakePatternPtr<Field>();
    auto r_index_type = MakePatternPtr<Unsigned>();
    auto r_inc = MakePatternPtr<PostIncrement>();
    auto r_ret_dec = MakePatternPtr<PostDecrement>();
    auto s_identifier = MakePatternPtr<InstanceIdentifier>();
    auto r_array = MakePatternPtr<Array>();
    auto ret = MakePatternPtr<Return>();
    auto l_r_sub = MakePatternPtr<Subscript>();
    auto s_and3 = MakePatternPtr< Conjunction<Node> >();
    auto r_index_identifier = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_stack_index");
    auto r_identifier = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_stack");
    auto s_gg = MakePatternPtr< GreenGrass<Statement> >();
    auto r_index_init = MakePatternPtr<Assign>();
    auto members = MakePatternPtr< Star<Declaration> >();
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto ls_module = MakePatternPtr<Scope>();
    auto lr_module = MakePatternPtr<Scope>();
    auto bases = MakePatternPtr< Star<Base> >();
    auto l_bases = MakePatternPtr< Star<Base> >();
    auto module_id = MakePatternPtr<TypeIdentifier>();
    auto l_module_id = MakePatternPtr<TypeIdentifier>();
    auto s_vcomp = MakePatternPtr<Compound>();
    auto r_vcomp = MakePatternPtr<Compound>();
    auto vdecls = MakePatternPtr< Star<Declaration> >();
    auto l_members = MakePatternPtr< Star<Declaration> >();
    auto vstmts = MakePatternPtr< Star<Statement> >();
    auto fi_id = MakePatternPtr<InstanceIdentifier>();

    // Sub-slave replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    auto r_slave = MakePatternPtr< SlaveSearchReplace<Statement> >( r_vcomp, s_identifier, l_r_sub );

    // SlaveSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_vcomp;
    s_vcomp->members = (vdecls, s_instance);
    s_vcomp->statements = (vstmts);
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakePatternPtr<Uninitialised>(); 
    s_instance->type = MakePatternPtr<Type>();

    // SlaveSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakePatternPtr<NonConst>();
    r_instance->initialiser = MakePatternPtr<Uninitialised>();
    overlay->overlay = r_slave;
    r_vcomp->members = (vdecls);
    r_vcomp->statements = (vstmts);
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
    r_instance->virt = MakePatternPtr<NonVirtual>();
    r_instance->access = MakePatternPtr<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakePatternPtr<SpecificInteger>(10);

    // SlaveSearchReplace to find early returns in the function
    s_gg->through = ret;

    // SlaveSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    ls_module->members = (l_members, l_fi);
    lr_module->members = (l_members, l_fi, r_instance);
    l_fi->initialiser = stuff;
    l_fi->identifier = fi_id;
    
    auto r_mid = MakePatternPtr< SlaveCompareReplace<Scope> >( r_module, ls_module, lr_module ); // stuff, stuff

    auto r_slave3 = MakePatternPtr< SlaveSearchReplace<Statement> >( r_top_comp, s_gg, r_ret_comp );
    temp->statements = (r_slave3);
    
    // Master search - look for functions satisfying the construct limitation and get
    s_module->members = (over, members);
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

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->conjuncts = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    // top-level decls
    r_top_comp->members = (top_decls);
    r_index->type = r_index_type;
    r_index_type->width = MakePatternPtr<SpecificInteger>(32);
    r_index_identifier->sources = (fi_id);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakePatternPtr<NonConst>();
    r_index->initialiser = MakePatternPtr<SpecificInteger>(0);
    //r_index->initialiser = MakePatternPtr<SpecificInteger>(0);
    r_index_init->operands = (r_index_identifier, MakePatternPtr<SpecificInteger>(0));
    r_index->virt = MakePatternPtr<NonVirtual>();
    r_index->access = MakePatternPtr<Private>();

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
    // Use a slave to find automatic variables in the function. Replace them
    // with arrays of the same type. Using another slave, add a decrement of
    // the stack pointer before any return statements.
    //
    // Using a sub-slave of the variable-finding slave, look for usages of 
    // the variable. Replace with an indexing operation into the array using
    // the stack index.    
    auto fi = MakePatternPtr<Instance>();
    auto oinit = MakePatternPtr< Delta<Initialiser> >();
    auto s_func = MakePatternPtr<Callable>();
    auto s_and = MakePatternPtr< Conjunction<Initialiser> >();
    auto s_top_comp = MakePatternPtr<Compound>();
    auto r_top_comp = MakePatternPtr<Compound>();
    auto r_ret_comp = MakePatternPtr<Compound>();
    auto temp = MakePatternPtr<Compound>();
    auto top_decls = MakePatternPtr< Star<Declaration> >();
    auto top_pre = MakePatternPtr< Star<Statement> >();
    auto stuff = MakePatternPtr< Stuff<Statement> >();
    auto cs_stuff = MakePatternPtr< Stuff<Compound> >();
    auto overlay = MakePatternPtr< Delta<Statement> >();
    auto cs_instance = MakePatternPtr<Automatic>();
    auto s_instance = MakePatternPtr<Automatic>();
    auto r_index = MakePatternPtr<Static>();
    auto r_instance = MakePatternPtr<Static>();
    auto r_index_type = MakePatternPtr<Unsigned>();
    auto r_inc = MakePatternPtr<PostIncrement>();
    auto r_ret_dec = MakePatternPtr<PostDecrement>();
    auto s_identifier = MakePatternPtr<InstanceIdentifier>();
    auto r_array = MakePatternPtr<Array>();
    auto ret = MakePatternPtr<Return>();
    auto l_r_sub = MakePatternPtr<Subscript>();
    auto s_and3 = MakePatternPtr< Conjunction<Node> >();
    auto r_index_identifier = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_stack_index");
    auto r_identifier = MakePatternPtr<BuildInstanceIdentifierAgent>("%s_stack");
    auto s_gg = MakePatternPtr< GreenGrass<Statement> >();

    // Master search - look for functions satisfying the construct limitation and get
    fi->identifier = MakePatternPtr<InstanceIdentifier>();
    fi->type = s_func;
    fi->initialiser = oinit;   
    oinit->through = s_and;
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->conjuncts = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    auto r_slave = MakePatternPtr< SlaveSearchReplace<Statement> >( stuff, s_identifier, l_r_sub );
    auto r_mid = MakePatternPtr< SlaveCompareReplace<Statement> >( r_top_comp, stuff, r_slave );
    auto r_slave3 = MakePatternPtr< SlaveSearchReplace<Statement> >( r_mid, s_gg, r_ret_comp );
    temp->statements = (r_slave3);
    oinit->overlay = temp;//r_slave3; 

    // top-level decls
    r_top_comp->members = ( top_decls, r_index );
    r_index->type = r_index_type;
    r_index_type->width = MakePatternPtr<SpecificInteger>(32);
    r_index_identifier->sources = (fi->identifier);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakePatternPtr<NonConst>();
    r_index->initialiser = MakePatternPtr<SpecificInteger>(0);
//    r_index->virt = MakePatternPtr<NonVirtual>();
//    r_index->access = MakePatternPtr<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_inc, top_pre );

    // SlaveSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_instance;
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakePatternPtr<Uninitialised>(); // can't handle initialisers!
    s_instance->type = MakePatternPtr<Type>();

    // SlaveSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakePatternPtr<NonConst>();
    r_instance->initialiser = MakePatternPtr<Uninitialised>();
    overlay->overlay = r_instance;
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
//    r_instance->virt = MakePatternPtr<NonVirtual>();
//    r_instance->access = MakePatternPtr<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakePatternPtr<SpecificInteger>(10);

    // Sub-slave replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    // SlaveSearchReplace to find early returns in the function
    s_gg->through = ret;

    // SlaveSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    Configure( SEARCH_REPLACE, fi );
}

*/


MergeFunctions::MergeFunctions()
{
    auto s_module = MakePatternPtr<Scope>();
    auto r_module = MakePatternPtr<Scope>();
    auto thread = MakePatternPtr<Field>();
    auto s_func = MakePatternPtr<Field>();
    auto thread_type = MakePatternPtr<Thread>();
    auto func_type = MakePatternPtr<Callable>();
    auto members = MakePatternPtr< Star<Declaration> >();
    auto thread_decls = MakePatternPtr< Star<Declaration> >();
    auto thread_stmts = MakePatternPtr< Star<Statement> >();
    auto bases = MakePatternPtr< Star<Base> >();
    auto thread_over = MakePatternPtr< Delta<Compound> >();
    auto s_thread_comp = MakePatternPtr<Compound>();
    auto r_thread_comp = MakePatternPtr<Compound>();
    auto s_call = MakePatternPtr<Call>();
    auto ls_call = MakePatternPtr<Call>();
    auto func_id = MakePatternPtr<InstanceIdentifier>();
    auto r_label = MakePatternPtr<Label>();
    auto r_label_id = MakePatternPtr< BuildLabelIdentifierAgent >("ENTER_%s");
    auto s_all = MakePatternPtr< Conjunction<Compound> >();
    auto s_stuff = MakePatternPtr< Stuff<Compound> >();
    auto func_stuff = MakePatternPtr< Stuff<Compound> >();
    auto module_id = MakePatternPtr<TypeIdentifier>();
    auto lr_goto = MakePatternPtr<Goto>();
    auto mr_goto = MakePatternPtr<Goto>();
    auto ms_return = MakePatternPtr<Return>();
    auto retaddr = MakePatternPtr<TempReturnAddress>();
    auto retaddr_id = MakePatternPtr<InstanceIdentifier>();
    
    mr_goto->destination = retaddr_id;
     
    auto slavem = MakePatternPtr< SlaveSearchReplace<Compound> >( func_stuff, ms_return, mr_goto );        
    
    ls_call->callee = func_id;
    lr_goto->destination = r_label_id;
        
    auto slavel = MakePatternPtr< SlaveSearchReplace<Compound> >( r_thread_comp, ls_call, lr_goto );    
    
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
    thread_over->overlay = slavel;
    r_thread_comp->members = (thread_decls);
    r_thread_comp->statements = (thread_stmts, r_label, slavem);
    r_label->identifier = r_label_id;
    r_label_id->sources = (func_id);
         
    Configure( SEARCH_REPLACE, s_module, r_module );
}


