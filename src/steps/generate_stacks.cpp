/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/sctree.hpp"

using namespace CPPTree;
using namespace Steps;
using namespace SCTree;

/// Place return at end of void functions, if not already there
ExplicitiseReturn::ExplicitiseReturn()
{
    MakeTreePtr< Instance > fi;
    MakeTreePtr<Compound> s_comp, sx_comp, r_comp;        
    MakeTreePtr< Star<Statement> > pre, sx_pre;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr<Return> sx_return, r_return;
    MakeTreePtr< MatchAny<Callable> > s_any;
    MakeTreePtr<Function> s_func;
    MakeTreePtr<Procedure> s_proc;
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< NotMatch<Compound> > s_not;
        
    fi->type = s_any;
    s_any->patterns = (s_func, s_proc, MakeTreePtr<Subroutine>() );
    s_proc->members = MakeTreePtr< Star<Declaration> >();
    s_func->members = MakeTreePtr< Star<Declaration> >();
    s_func->return_type = MakeTreePtr< Void >();
    fi->initialiser = over;
    s_comp->members = decls;
    s_comp->statements = (pre);
    over->through = s_all;
    s_all->patterns = (s_comp, s_not);
    s_not->pattern = sx_comp;
    sx_comp->members = decls;
    sx_comp->statements = (sx_pre, sx_return);
    
    over->overlay = r_comp;
    r_comp->members = decls;
    r_comp->statements = (pre, r_return);
    r_return->return_value = MakeTreePtr<Uninitialised>();
    
    Configure( fi );
}    


UseTempForReturnValue::UseTempForReturnValue()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
    TreePtr<Return> s_return( new Return );
    TreePtr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
    s_return->return_value = s_and;
    MakeTreePtr< TransformOf<Expression> > retval( &TypeOf::instance );
    MakeTreePtr<Type> type;
    retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    TreePtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> ); // TODO the exclusion Stuff<GetDec<Automatic>> is too strong;
                                                                    // use Not<GetDec<Temp>>
    s_and->patterns = ( retval, cs_stuff );
    MakeTreePtr< TransformOf<InstanceIdentifier> > cs_id( &GetDeclaration::instance );
    cs_stuff->terminus = cs_id;
    TreePtr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
    TreePtr<Compound> r_sub_comp( new Compound );
    TreePtr< Temporary > r_newvar( new Temporary );
    r_newvar->type = type;
    MakeTreePtr<BuildInstanceIdentifier> id("temp_retval");
    r_newvar->identifier = id;
    r_newvar->initialiser = MakeTreePtr<Uninitialised>();
    r_sub_comp->members = ( r_newvar );
    TreePtr<Assign> r_assign( new Assign );
    r_assign->operands.push_back( id );
    r_assign->operands.push_back( retval );
    r_sub_comp->statements.push_back( r_assign );
    TreePtr<Return> r_return( new Return );
    r_sub_comp->statements.push_back( r_return );
    r_return->return_value = id;
       
    Configure( s_return, r_sub_comp );
}


ReturnViaTemp::ReturnViaTemp()
{
    MakeTreePtr<Scope> module;
    MakeTreePtr<Instance> func;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr<Function> cp;
    MakeTreePtr< NotMatch<Type> > return_type;
    MakeTreePtr<Void> sx_void;
    MakeTreePtr<Compound> s_body, r_body, lr_comp;
    MakeTreePtr< Star<Statement> > statements;
    MakeTreePtr< Star<Declaration> > locals;
    MakeTreePtr<InstanceIdentifier> func_id;
    MakeTreePtr<TypeIdentifier> module_id;
    MakeTreePtr< Star<Instance> > params;
    MakeTreePtr<Call> m_call;
    MakeTreePtr< Star<MapOperand> > m_operands;
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr<Assign> mr_assign, lr_assign;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("%s_return");
    MakeTreePtr<Return> ls_return, lr_return;
    MakeTreePtr<Expression> l_return_value;
    MakeTreePtr<CompoundExpression> mr_comp;
    MakeTreePtr< GreenGrass<Call> > ms_gg;
    MakeTreePtr< Overlay<Type> > overcp;
    MakeTreePtr< Overlay<Initialiser> > overi;
    MakeTreePtr< Insert<Declaration> > insert;
    
    MakeTreePtr< SlaveSearchReplace<Compound> > slavel( r_body, ls_return, lr_comp );
    ls_return->return_value = l_return_value; // note this also pre-restricts away Return<Uninitialised>
    lr_comp->statements = (lr_assign, lr_return);
    lr_assign->operands = (r_temp_id, l_return_value);
    lr_return->return_value = MakeTreePtr<Uninitialised>(); // means no return value given

    ms_gg->through = m_call;
    m_call->callee = func_id;
    m_call->operands = (m_operands);
    mr_comp->statements = (m_call, r_temp_id);
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( module, ms_gg, mr_comp );
    
    module->members = (decls, func, insert);
    insert->insert = (r_temp);
    func->type = cp;
    func->initialiser = overi;
    overi->through = s_body;
    func->identifier = func_id;
    s_body->members = (locals);
    s_body->statements = (statements);
    cp->members = (params);  
    cp->return_type = overcp;
    overcp->through = return_type;
    return_type->pattern = sx_void;
    overi->overlay = slavel;
    r_body->members = (locals);
    r_body->statements = (statements);
    overcp->overlay = MakeTreePtr<Void>();
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_temp->type = return_type;
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id);
    
    Configure( module, slavem );  
}



struct TempReturnAddress : Temporary
{
    NODE_FUNCTIONS_FINAL
};


AddLinkAddress::AddLinkAddress()
{
    MakeTreePtr<Scope> module;
    MakeTreePtr< Star<Declaration> > decls;    
    MakeTreePtr< Star<Base> > bases;    
    MakeTreePtr<Temporary> r_retaddr;
    MakeTreePtr<BuildInstanceIdentifier> r_retaddr_id("%s_link");
    MakeTreePtr<Automatic> lr_retaddr;
    MakeTreePtr<BuildInstanceIdentifier> lr_retaddr_id("link");
    MakeTreePtr<TempReturnAddress> lr_temp_retaddr;
    MakeTreePtr<BuildInstanceIdentifier> lr_temp_retaddr_id("temp_link");
    MakeTreePtr<Pointer> r_ptr, lr_ptr;
    MakeTreePtr< NotMatch<Declaration> > s_nm, ls_nm;
    MakeTreePtr< GreenGrass<Declaration> > gg;
    MakeTreePtr<Instance> l_inst;
    MakeTreePtr< Overlay<Compound> > l_over;
    MakeTreePtr<Compound> ls_comp, lr_comp;
    MakeTreePtr< Star<Declaration> > l_decls;
    MakeTreePtr< Star<Statement> > l_stmts, msx_stmts;
    MakeTreePtr<Assign> msx_assign;
    MakeTreePtr<Call> ms_call, mr_call;
    MakeTreePtr<Compound> mr_comp, msx_comp;
    MakeTreePtr<Label> mr_label;
    MakeTreePtr<BuildLabelIdentifier> mr_labelid("LINK");
    MakeTreePtr< MatchAll<Statement> > m_all;
    MakeTreePtr< AnyNode<Statement> > m_any; // TODO rename AnyNode -> Blob
    MakeTreePtr< NotMatch<Statement> > ms_not;
    MakeTreePtr< Overlay<Statement> > m_over;
    MakeTreePtr< Overlay<Function> > l_func_over;
    MakeTreePtr<Function> ls_func, lr_func;
    MakeTreePtr<TypeIdentifier> ident;
    MakeTreePtr<InstanceIdentifier> l_inst_id;
    MakeTreePtr<Return> ll_return;
    MakeTreePtr<Compound> llr_comp, llsx_comp;
    MakeTreePtr<Assign> llr_assign, llsx_assign;
    MakeTreePtr< MatchAll<Statement> > ll_all;
    MakeTreePtr< AnyNode<Statement> > ll_any;
    MakeTreePtr< NotMatch<Statement> > lls_not;
    MakeTreePtr< Overlay<Statement> > ll_over;
    MakeTreePtr< GreenGrass<Statement> > m_gg, ll_gg;
    MakeTreePtr<MapOperand> mr_operand;
    MakeTreePtr< Insert<Declaration> > insert;

    ll_gg->through = ll_return;
    ll_over->overlay = llr_comp;        
    //llr_comp->members = ();
    llr_comp->statements = (llr_assign, ll_return);
    llr_assign->operands = (lr_temp_retaddr_id, lr_retaddr_id);
   
    MakeTreePtr< SlaveSearchReplace<Compound> > slavell( lr_comp, ll_gg, llr_comp );   
   
    m_gg->through = ms_call;
    ms_call->operands = (MakeTreePtr< Star<MapOperand> >());
    ms_call->callee = l_inst_id;
    mr_comp->statements = (mr_call, mr_label);  
    mr_call->operands = (ms_call->operands, mr_operand);
    mr_call->callee = l_inst_id;
    mr_operand->identifier = lr_retaddr_id;
    mr_operand->value = mr_labelid;
    mr_label->identifier = mr_labelid;    
    
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( module, m_gg, mr_comp );
    
    l_inst->type = l_func_over;
    l_func_over->through = ls_func;
    l_func_over->overlay = lr_func;    
    ls_func->return_type = MakeTreePtr<Void>();
    ls_func->members = MakeTreePtr< Star<Instance> >(); // Params OK here, just not in MergeFunctions
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
    lr_retaddr->type = lr_ptr;
    lr_retaddr->initialiser = MakeTreePtr<Uninitialised>();
    lr_temp_retaddr->identifier = lr_temp_retaddr_id;
    lr_temp_retaddr->type = lr_ptr;
    lr_temp_retaddr->initialiser = MakeTreePtr<Uninitialised>();
    lr_ptr->destination = MakeTreePtr<Void>();
    lr_comp->statements = (l_stmts);
    
    module->members = (gg, decls, insert);
    insert->insert = (r_retaddr);
    gg->through = l_inst;
    r_retaddr->identifier = r_retaddr_id;
    r_retaddr->type = r_ptr;
    r_ptr->destination = MakeTreePtr<Void>();
    r_retaddr->initialiser = MakeTreePtr<Uninitialised>();
    //r_retaddr->virt = MakeTreePtr<NonVirtual>();
    //r_retaddr->access = MakeTreePtr<Private>();
    //r_retaddr->constancy = MakeTreePtr<NonConst>();
    r_retaddr_id->sources = (l_inst_id);
    Configure( module, slavem );  
}


ParamsViaTemps::ParamsViaTemps()
{
    MakeTreePtr<Scope> module;
    MakeTreePtr<Instance> func;
    MakeTreePtr< Star<Declaration> > decls;
    MakeTreePtr< Star<Base> > bases;
    MakeTreePtr<Function> s_cp, r_cp;
    MakeTreePtr<Type> return_type;
    MakeTreePtr<Compound> body, mr_comp;
    MakeTreePtr< Star<Statement> > statements;
    MakeTreePtr< Star<Declaration> > locals;
    MakeTreePtr<InstanceIdentifier> func_id, param_id;
    MakeTreePtr<TypeIdentifier> module_id;
    MakeTreePtr<Instance> s_param;
    MakeTreePtr< Star<Instance> > params;
    MakeTreePtr<Call> ms_call, mr_call;
    MakeTreePtr<MapOperand> ms_operand;
    MakeTreePtr< Star<MapOperand> > m_operands;
    MakeTreePtr<Automatic> r_param;
    MakeTreePtr<Type> param_type;
    MakeTreePtr<Temporary> r_temp;
    MakeTreePtr<Assign> mr_assign;
    MakeTreePtr<Expression> m_expr;
    MakeTreePtr<BuildInstanceIdentifier> r_temp_id("%s_%s");
    MakeTreePtr< Insert<Declaration> > insert;
    MakeTreePtr< Insert<Statement> > ins_param;    
    MakeTreePtr< Overlay<Type> > over;
    
    ms_call->callee = func_id;
    ms_call->operands = (m_operands, ms_operand);
    ms_operand->identifier = param_id;
    ms_operand->value = m_expr;
    mr_comp->statements = (mr_assign, mr_call);
    mr_assign->operands = (r_temp_id, m_expr);
    mr_call->callee = func_id;
    mr_call->operands = (m_operands);
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( module, ms_call, mr_comp );
    
    module->members = (decls, func, insert);
    func->type = over;
    over->through = s_cp;
    func->initialiser = body;
    func->identifier = func_id;
    body->members = (locals);
    body->statements = (ins_param, statements);
    s_cp->members = (params, s_param);  
    s_cp->return_type = return_type;
    s_param->identifier = param_id;
    s_param->type = param_type;
    over->overlay = r_cp;
    ins_param->insert = r_param;
    r_param->type = param_type;
    r_param->initialiser = r_temp_id;
    r_param->identifier = param_id;
    r_cp->members = (params);
    r_cp->return_type = return_type;
    insert->insert = (r_temp);
    r_temp->type = param_type;
    r_temp->initialiser = MakeTreePtr<Uninitialised>();
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id, param_id);
    
    Configure( module, slavem );  
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
    MakeTreePtr<Instance> fi, l_fi;
    MakeTreePtr< Overlay<Initialiser> > oinit;
    MakeTreePtr<Thread> sx_thread;
    MakeTreePtr<Method> sx_method;
    MakeTreePtr< MatchAny<Type> > sx_any;
    MakeTreePtr< NotMatch<Type> > s_not;
    MakeTreePtr< MatchAll<Initialiser> > s_and;
    MakeTreePtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakeTreePtr< Star<Declaration> > top_decls;
    MakeTreePtr< Star<Statement> > top_pre;
    MakeTreePtr< Stuff<Initialiser> > stuff;
    MakeTreePtr< Stuff<Compound> > cs_stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Automatic> cs_instance, s_instance;
    MakeTreePtr<Field> r_index, r_instance;
    MakeTreePtr<Unsigned> r_index_type;
    MakeTreePtr<PostIncrement> r_inc;
    MakeTreePtr<PostDecrement> r_ret_dec;
    MakeTreePtr<InstanceIdentifier> s_identifier;
    MakeTreePtr<Array> r_array;
    MakeTreePtr<Return> ret;
    MakeTreePtr<Subscript> l_r_sub;
    MakeTreePtr< MatchAll<Node> > s_and3;
    MakeTreePtr<BuildInstanceIdentifier> r_index_identifier("%s_stack_index");
    MakeTreePtr<BuildInstanceIdentifier> r_identifier("%s_stack");
    MakeTreePtr< GreenGrass<Statement> > s_gg;
    MakeTreePtr<Assign> r_index_init;
    MakeTreePtr< Star<Declaration> > members;
    MakeTreePtr<Scope> module, l_module;
    MakeTreePtr< Star<Base> > bases, l_bases;
    MakeTreePtr<TypeIdentifier> module_id, l_module_id;
    MakeTreePtr<Compound> s_vcomp, r_vcomp;
    MakeTreePtr< Star<Declaration> > vdecls, l_members;
    MakeTreePtr< Star<Statement> > vstmts;
    MakeTreePtr<InstanceIdentifier> fi_id;
    MakeTreePtr< Insert<Declaration> > insert, l_insert;

    // Sub-slave replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave( r_vcomp, s_identifier, l_r_sub );

    // SlaveSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_vcomp;
    s_vcomp->members = (vdecls, s_instance);
    s_vcomp->statements = (vstmts);
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakeTreePtr<Uninitialised>(); 
    s_instance->type = MakeTreePtr<Type>();

    // SlaveSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakeTreePtr<NonConst>();
    r_instance->initialiser = MakeTreePtr<Uninitialised>();
    overlay->overlay = r_slave;
    r_vcomp->members = (vdecls);
    r_vcomp->statements = (vstmts);
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
    r_instance->virt = MakeTreePtr<NonVirtual>();
    r_instance->access = MakeTreePtr<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakeTreePtr<SpecificInteger>(10);

    // SlaveSearchReplace to find early returns in the function
    s_gg->through = ret;

    // SlaveSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    l_module->members = (l_members, l_fi, l_insert);
    l_insert->insert = (r_instance);
    l_fi->initialiser = stuff;
    l_fi->identifier = fi_id;
    
    MakeTreePtr< SlaveCompareReplace<Scope> > r_mid( module, l_module ); // stuff, stuff

    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave3( r_top_comp, s_gg, r_ret_comp );
    temp->statements = (r_slave3);
    oinit->overlay = temp;//r_slave3; 
    
    // Master search - look for functions satisfying the construct limitation and get
    module->members = (fi, members, insert);
    insert->insert = (r_index);
    fi->identifier = fi_id;
    fi->type = s_not;
    s_not->pattern = sx_any;
    sx_any->patterns = (sx_thread, sx_method); // Do not provide stacks for these because they do not recurse
    fi->initialiser = oinit;   
    oinit->through = s_and;
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    // top-level decls
    r_top_comp->members = (top_decls);
    r_index->type = r_index_type;
    r_index_type->width = MakeTreePtr<SpecificInteger>(32);
    r_index_identifier->sources = (fi->identifier);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakeTreePtr<NonConst>();
    r_index->initialiser = MakeTreePtr<SpecificInteger>(0);
    //r_index->initialiser = MakeTreePtr<SpecificInteger>(0);
    r_index_init->operands = (r_index_identifier, MakeTreePtr<SpecificInteger>(0));
    r_index->virt = MakeTreePtr<NonVirtual>();
    r_index->access = MakeTreePtr<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_inc, top_pre );

    Configure( module, r_mid );
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
    MakeTreePtr<Instance> fi;
    MakeTreePtr< Overlay<Initialiser> > oinit;
    MakeTreePtr<Callable> s_func;
    MakeTreePtr< MatchAll<Initialiser> > s_and;
    MakeTreePtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakeTreePtr< Star<Declaration> > top_decls;
    MakeTreePtr< Star<Statement> > top_pre;
    MakeTreePtr< Stuff<Statement> > stuff;
    MakeTreePtr< Stuff<Compound> > cs_stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Automatic> cs_instance, s_instance;
    MakeTreePtr<Static> r_index, r_instance;
    MakeTreePtr<Unsigned> r_index_type;
    MakeTreePtr<PostIncrement> r_inc;
    MakeTreePtr<PostDecrement> r_ret_dec;
    MakeTreePtr<InstanceIdentifier> s_identifier;
    MakeTreePtr<Array> r_array;
    MakeTreePtr<Return> ret;
    MakeTreePtr<Subscript> l_r_sub;
    MakeTreePtr< MatchAll<Node> > s_and3;
    MakeTreePtr<BuildInstanceIdentifier> r_index_identifier("%s_stack_index");
    MakeTreePtr<BuildInstanceIdentifier> r_identifier("%s_stack");
    MakeTreePtr< GreenGrass<Statement> > s_gg;

    // Master search - look for functions satisfying the construct limitation and get
    fi->identifier = MakeTreePtr<InstanceIdentifier>();
    fi->type = s_func;
    fi->initialiser = oinit;   
    oinit->through = s_and;
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave( stuff, s_identifier, l_r_sub );
    MakeTreePtr< SlaveCompareReplace<Statement> > r_mid( r_top_comp, stuff, r_slave );
    MakeTreePtr< SlaveSearchReplace<Statement> > r_slave3( r_mid, s_gg, r_ret_comp );
    temp->statements = (r_slave3);
    oinit->overlay = temp;//r_slave3; 

    // top-level decls
    r_top_comp->members = ( top_decls, r_index );
    r_index->type = r_index_type;
    r_index_type->width = MakeTreePtr<SpecificInteger>(32);
    r_index_identifier->sources = (fi->identifier);
    r_index->identifier = r_index_identifier;
    r_index->constancy = MakeTreePtr<NonConst>();
    r_index->initialiser = MakeTreePtr<SpecificInteger>(0);
//    r_index->virt = MakeTreePtr<NonVirtual>();
//    r_index->access = MakeTreePtr<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_inc, top_pre );

    // SlaveSearchReplace search to find automatic variables within the function
    stuff->terminus = overlay;
    overlay->through = s_instance;
    s_instance->identifier = s_identifier;
    s_instance->initialiser = MakeTreePtr<Uninitialised>(); // can't handle initialisers!
    s_instance->type = MakeTreePtr<Type>();

    // SlaveSearchReplace replace to insert as a static array (TODO be a member of enclosing class)
    r_instance->constancy = MakeTreePtr<NonConst>();
    r_instance->initialiser = MakeTreePtr<Uninitialised>();
    overlay->overlay = r_instance;
    r_identifier->sources = (s_identifier);
    r_instance->identifier = r_identifier;
    r_instance->type = r_array;
//    r_instance->virt = MakeTreePtr<NonVirtual>();
//    r_instance->access = MakeTreePtr<Private>();
    r_array->element = s_instance->type;
    r_array->size = MakeTreePtr<SpecificInteger>(10);

    // Sub-slave replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    // SlaveSearchReplace to find early returns in the function
    s_gg->through = ret;

    // SlaveSearchReplace replace with a decrement of the stack index coming before the return
    //r_ret_comp->members = ( r_ret_decls );
    r_ret_dec->operands = ( r_index_identifier );
    r_ret_comp->statements = ( r_ret_dec, ret );

    Configure( fi );
}

*/


MergeFunctions::MergeFunctions()
{
    MakeTreePtr<Scope> module;
    MakeTreePtr<Field> thread, s_func;
    MakeTreePtr<Thread> thread_type;
    MakeTreePtr<Callable> func_type;
    MakeTreePtr< Star<Declaration> > members, thread_decls;
    MakeTreePtr< Star<Statement> > thread_stmts;
    MakeTreePtr< Star<Base> > bases;    
    MakeTreePtr< Overlay<Compound> > thread_over;
    MakeTreePtr<Compound> s_thread_comp, r_thread_comp;
    MakeTreePtr<Call> s_call, ls_call;
    MakeTreePtr<InstanceIdentifier> func_id;
    MakeTreePtr<Label> r_label;
    MakeTreePtr< BuildLabelIdentifier > r_label_id("ENTER_%s");
    MakeTreePtr< MatchAll<Compound> > s_all;
    MakeTreePtr< Stuff<Compound> > s_stuff, func_stuff;
    MakeTreePtr<TypeIdentifier> module_id;
    MakeTreePtr<Goto> lr_goto, mr_goto;
    MakeTreePtr<Return> ms_return;    
    MakeTreePtr<TempReturnAddress> retaddr;
    MakeTreePtr<InstanceIdentifier> retaddr_id;
    MakeTreePtr< Erase<Declaration> > erase;
    
    mr_goto->destination = retaddr_id;
     
    MakeTreePtr< SlaveSearchReplace<Compound> > slavem( func_stuff, ms_return, mr_goto );        
    
    ls_call->callee = func_id;
    lr_goto->destination = r_label_id;
        
    MakeTreePtr< SlaveSearchReplace<Compound> > slavel( r_thread_comp, ls_call, lr_goto );    
    
    module->members = (members, thread, erase);
    erase->erase = (s_func);
    thread->type = thread_type;
    thread->initialiser = thread_over;
    thread_over->through = s_all;
    s_all->patterns = (s_thread_comp, s_stuff);
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
         
    Configure( module );
}


