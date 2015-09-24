/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/pointer_is_agent.hpp"
#include "sr/transform_of_agent.hpp"
#include "tree/sctree.hpp"
#include "inferno_agents.hpp"

using namespace CPPTree;
using namespace Steps;
using namespace SCTree;

/// Place return at end of void functions, if not already there
ExplicitiseReturn::ExplicitiseReturn()
{
    MakePatternPtr< Instance > fi;
    MakePatternPtr<Compound> s_comp, sx_comp, r_comp;        
    MakePatternPtr< Star<Statement> > pre, sx_pre;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Overlay<Compound> > over;
    MakePatternPtr<Return> sx_return, r_return;
    MakePatternPtr< MatchAny<Callable> > s_any;
    MakePatternPtr<Function> s_func;
    MakePatternPtr<Procedure> s_proc;
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< NotMatch<Compound> > s_not;
        
    fi->type = s_any;
    s_any->patterns = (s_func, s_proc, MakePatternPtr<Subroutine>() );
    s_proc->members = MakePatternPtr< Star<Declaration> >();
    s_func->members = MakePatternPtr< Star<Declaration> >();
    s_func->return_type = MakePatternPtr< Void >();
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
    r_return->return_value = MakePatternPtr<Uninitialised>();
    
    Configure( fi );
}    


UseTempForReturnValue::UseTempForReturnValue()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
    TreePtr<Return> s_return( new Return );
    TreePtr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
    s_return->return_value = s_and;
    MakePatternPtr< TransformOf<Expression> > retval( &TypeOf::instance );
    MakePatternPtr<Type> type;
    retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    TreePtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> ); // TODO the exclusion Stuff<GetDec<Automatic>> is too strong;
                                                                    // use Not<GetDec<Temp>>
    s_and->patterns = ( retval, cs_stuff );
    MakePatternPtr< TransformOf<InstanceIdentifier> > cs_id( &GetDeclaration::instance );
    cs_stuff->terminus = cs_id;
    TreePtr<Instance> cs_instance( new Automatic );
    cs_id->pattern = cs_instance;
    
    // replace with a new sub-compound, that declares a Temp, intialises it to the return value and returns it
    TreePtr<Compound> r_sub_comp( new Compound );
    TreePtr< Temporary > r_newvar( new Temporary );
    r_newvar->type = type;
    MakePatternPtr<BuildInstanceIdentifier> id("temp_retval");
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
       
    Configure( s_return, r_sub_comp );
}


ReturnViaTemp::ReturnViaTemp()
{
    MakePatternPtr<Scope> s_module, r_module;
    MakePatternPtr<Instance> func;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Function> cp;
    MakePatternPtr< NotMatch<Type> > return_type;
    MakePatternPtr<Void> sx_void;
    MakePatternPtr<Compound> s_body, r_body, lr_comp;
    MakePatternPtr< Star<Statement> > statements;
    MakePatternPtr< Star<Declaration> > locals;
    MakePatternPtr<InstanceIdentifier> func_id;
    MakePatternPtr<TypeIdentifier> module_id;
    MakePatternPtr< Star<Instance> > params;
    MakePatternPtr<Call> m_call;
    MakePatternPtr< Star<MapOperand> > m_operands;
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr<Assign> mr_assign, lr_assign;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("%s_return");
    MakePatternPtr<Return> ls_return, lr_return;
    MakePatternPtr<Expression> l_return_value;
    MakePatternPtr<CompoundExpression> mr_comp;
    MakePatternPtr< GreenGrass<Call> > ms_gg;
    MakePatternPtr< Overlay<Type> > overcp;
    MakePatternPtr< Overlay<Initialiser> > overi;
    
    MakePatternPtr< SlaveSearchReplace<Compound> > slavel( r_body, ls_return, lr_comp );
    ls_return->return_value = l_return_value; // note this also pre-restricts away Return<Uninitialised>
    lr_comp->statements = (lr_assign, lr_return);
    lr_assign->operands = (r_temp_id, l_return_value);
    lr_return->return_value = MakePatternPtr<Uninitialised>(); // means no return value given

    ms_gg->through = m_call;
    m_call->callee = func_id;
    m_call->operands = (m_operands);
    mr_comp->statements = (m_call, r_temp_id);
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( r_module, ms_gg, mr_comp );
    
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
    return_type->pattern = sx_void;
    overi->overlay = slavel;
    r_body->members = (locals);
    r_body->statements = (statements);
    overcp->overlay = MakePatternPtr<Void>();
    r_temp->initialiser = MakePatternPtr<Uninitialised>();
    r_temp->type = return_type;
    r_temp->identifier = r_temp_id;
    r_temp_id->sources = (func_id);
    
    Configure( s_module, slavem );  
}


struct TempReturnAddress : Temporary
{
    NODE_FUNCTIONS_FINAL
};


AddLinkAddress::AddLinkAddress()
{
    MakePatternPtr<Scope> s_module, r_module;
    MakePatternPtr< Star<Declaration> > decls;    
    MakePatternPtr< Star<Base> > bases;    
    MakePatternPtr<Temporary> r_retaddr;
    MakePatternPtr<BuildInstanceIdentifier> r_retaddr_id("%s_link");
    MakePatternPtr<Automatic> lr_retaddr;
    MakePatternPtr<BuildInstanceIdentifier> lr_retaddr_id("link");
    MakePatternPtr<TempReturnAddress> lr_temp_retaddr;
    MakePatternPtr<BuildInstanceIdentifier> lr_temp_retaddr_id("temp_link");
    MakePatternPtr< NotMatch<Declaration> > s_nm, ls_nm;
    MakePatternPtr< GreenGrass<Declaration> > gg;
    MakePatternPtr<Instance> l_inst;
    MakePatternPtr< Overlay<Compound> > l_over;
    MakePatternPtr<Compound> ls_comp, lr_comp;
    MakePatternPtr< Star<Declaration> > l_decls;
    MakePatternPtr< Star<Statement> > l_stmts, msx_stmts;
    MakePatternPtr<Assign> msx_assign;
    MakePatternPtr<Call> ms_call, mr_call;
    MakePatternPtr<Compound> mr_comp, msx_comp;
    MakePatternPtr<Label> mr_label;
    MakePatternPtr<BuildLabelIdentifier> mr_labelid("LINK");
    MakePatternPtr< MatchAll<Statement> > m_all;
    MakePatternPtr< AnyNode<Statement> > m_any; // TODO rename AnyNode -> Blob
    MakePatternPtr< NotMatch<Statement> > ms_not;
    MakePatternPtr< Overlay<Statement> > m_over;
    MakePatternPtr< Overlay<Function> > l_func_over;
    MakePatternPtr<Function> ls_func, lr_func;
    MakePatternPtr<TypeIdentifier> ident;
    MakePatternPtr<InstanceIdentifier> l_inst_id;
    MakePatternPtr<Return> ll_return;
    MakePatternPtr<Compound> llr_comp, llsx_comp;
    MakePatternPtr<Assign> llr_assign, llsx_assign;
    MakePatternPtr< MatchAll<Statement> > ll_all;
    MakePatternPtr< AnyNode<Statement> > ll_any;
    MakePatternPtr< NotMatch<Statement> > lls_not;
    MakePatternPtr< Overlay<Statement> > ll_over;
    MakePatternPtr< GreenGrass<Statement> > m_gg, ll_gg;
    MakePatternPtr<MapOperand> mr_operand;

    ll_gg->through = ll_return;
    ll_over->overlay = llr_comp;        
    //llr_comp->members = ();
    llr_comp->statements = (llr_assign, ll_return);
    llr_assign->operands = (lr_temp_retaddr_id, lr_retaddr_id);
   
    MakePatternPtr< SlaveSearchReplace<Compound> > slavell( lr_comp, ll_gg, llr_comp );   
   
    m_gg->through = ms_call;
    ms_call->operands = (MakePatternPtr< Star<MapOperand> >());
    ms_call->callee = l_inst_id;
    mr_comp->statements = (mr_call, mr_label);  
    mr_call->operands = (ms_call->operands, mr_operand);
    mr_call->callee = l_inst_id;
    mr_operand->identifier = lr_retaddr_id;
    mr_operand->value = mr_labelid;
    mr_label->identifier = mr_labelid;    
    
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( r_module, m_gg, mr_comp );
    
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
    Configure( s_module, slavem );  
}


ParamsViaTemps::ParamsViaTemps()
{
    MakePatternPtr<Scope> s_module, r_module;
    MakePatternPtr<Instance> s_func, r_func;
    MakePatternPtr< Star<Declaration> > decls;
    MakePatternPtr< Star<Base> > bases;
    MakePatternPtr<Function> s_cp, r_cp;
    MakePatternPtr<Type> return_type;
    MakePatternPtr<Compound> s_body, r_body, mr_comp;
    MakePatternPtr< Star<Statement> > statements;
    MakePatternPtr< Star<Declaration> > locals;
    MakePatternPtr<InstanceIdentifier> func_id, param_id;
    MakePatternPtr<TypeIdentifier> module_id;
    MakePatternPtr<Instance> s_param;
    MakePatternPtr< Star<Instance> > params;
    MakePatternPtr<Call> ms_call, mr_call;
    MakePatternPtr<MapOperand> ms_operand;
    MakePatternPtr< Star<MapOperand> > m_operands;
    MakePatternPtr<Automatic> r_param;
    MakePatternPtr<Type> param_type;
    MakePatternPtr<Temporary> r_temp;
    MakePatternPtr<Assign> mr_assign;
    MakePatternPtr<Expression> m_expr;
    MakePatternPtr<BuildInstanceIdentifier> r_temp_id("%s_%s");
    MakePatternPtr< Overlay<Declaration> > over;
    
    ms_call->callee = func_id;
    ms_call->operands = (m_operands, ms_operand);
    ms_operand->identifier = param_id;
    ms_operand->value = m_expr;
    mr_comp->statements = (mr_assign, mr_call);
    mr_assign->operands = (r_temp_id, m_expr);
    mr_call->callee = func_id;
    mr_call->operands = (m_operands);
    MakePatternPtr< SlaveSearchReplace<Scope> > slavem( r_module, ms_call, mr_comp );
    
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
    
    Configure( s_module, slavem );  
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
    MakePatternPtr<Instance> s_fi, r_fi, l_fi;
    MakePatternPtr<Thread> sx_thread;
    MakePatternPtr<Method> sx_method;
    MakePatternPtr< MatchAny<Type> > sx_any;
    MakePatternPtr< NotMatch<Type> > s_not;
    MakePatternPtr< MatchAll<Initialiser> > s_and;
    MakePatternPtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakePatternPtr< Star<Declaration> > top_decls;
    MakePatternPtr< Star<Statement> > top_pre;
    MakePatternPtr< Stuff<Initialiser> > stuff;
    MakePatternPtr< Stuff<Compound> > cs_stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr< Overlay<Declaration> > over;
    MakePatternPtr<Automatic> cs_instance, s_instance;
    MakePatternPtr<Field> r_index, r_instance;
    MakePatternPtr<Unsigned> r_index_type;
    MakePatternPtr<PostIncrement> r_inc;
    MakePatternPtr<PostDecrement> r_ret_dec;
    MakePatternPtr<InstanceIdentifier> s_identifier;
    MakePatternPtr<Array> r_array;
    MakePatternPtr<Return> ret;
    MakePatternPtr<Subscript> l_r_sub;
    MakePatternPtr< MatchAll<Node> > s_and3;
    MakePatternPtr<BuildInstanceIdentifier> r_index_identifier("%s_stack_index");
    MakePatternPtr<BuildInstanceIdentifier> r_identifier("%s_stack");
    MakePatternPtr< GreenGrass<Statement> > s_gg;
    MakePatternPtr<Assign> r_index_init;
    MakePatternPtr< Star<Declaration> > members;
    MakePatternPtr<Scope> s_module, r_module, ls_module, lr_module;
    MakePatternPtr< Star<Base> > bases, l_bases;
    MakePatternPtr<TypeIdentifier> module_id, l_module_id;
    MakePatternPtr<Compound> s_vcomp, r_vcomp;
    MakePatternPtr< Star<Declaration> > vdecls, l_members;
    MakePatternPtr< Star<Statement> > vstmts;
    MakePatternPtr<InstanceIdentifier> fi_id;

    // Sub-slave replace with a subscript into the array
    l_r_sub->operands = ( r_identifier, r_index_identifier );

    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave( r_vcomp, s_identifier, l_r_sub );

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
    
    MakePatternPtr< SlaveCompareReplace<Scope> > r_mid( r_module, ls_module, lr_module ); // stuff, stuff

    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave3( r_top_comp, s_gg, r_ret_comp );
    temp->statements = (r_slave3);
    
    // Master search - look for functions satisfying the construct limitation and get
    s_module->members = (over, members);
    r_module->members = (over, members, r_index);
    over->through = s_fi;
    over->overlay = r_fi;    
    s_fi->identifier = r_fi->identifier = fi_id;
    s_fi->type = r_fi->type = s_not;
    s_not->pattern = sx_any;
    sx_any->patterns = (sx_thread, sx_method); // Do not provide stacks for these because they do not recurse
    s_fi->initialiser = s_and;   
    r_fi->initialiser = temp;   
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

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

    Configure( s_module, r_mid );
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
    MakePatternPtr<Instance> fi;
    MakePatternPtr< Overlay<Initialiser> > oinit;
    MakePatternPtr<Callable> s_func;
    MakePatternPtr< MatchAll<Initialiser> > s_and;
    MakePatternPtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakePatternPtr< Star<Declaration> > top_decls;
    MakePatternPtr< Star<Statement> > top_pre;
    MakePatternPtr< Stuff<Statement> > stuff;
    MakePatternPtr< Stuff<Compound> > cs_stuff;
    MakePatternPtr< Overlay<Statement> > overlay;
    MakePatternPtr<Automatic> cs_instance, s_instance;
    MakePatternPtr<Static> r_index, r_instance;
    MakePatternPtr<Unsigned> r_index_type;
    MakePatternPtr<PostIncrement> r_inc;
    MakePatternPtr<PostDecrement> r_ret_dec;
    MakePatternPtr<InstanceIdentifier> s_identifier;
    MakePatternPtr<Array> r_array;
    MakePatternPtr<Return> ret;
    MakePatternPtr<Subscript> l_r_sub;
    MakePatternPtr< MatchAll<Node> > s_and3;
    MakePatternPtr<BuildInstanceIdentifier> r_index_identifier("%s_stack_index");
    MakePatternPtr<BuildInstanceIdentifier> r_identifier("%s_stack");
    MakePatternPtr< GreenGrass<Statement> > s_gg;

    // Master search - look for functions satisfying the construct limitation and get
    fi->identifier = MakePatternPtr<InstanceIdentifier>();
    fi->type = s_func;
    fi->initialiser = oinit;   
    oinit->through = s_and;
    s_top_comp->members = ( top_decls );
    s_top_comp->statements = ( top_pre );

    // Construct limitation - restrict master search to functions that contain an automatic variable
    cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

    // Master replace - insert index variable, inc and dec into function at top level
    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave( stuff, s_identifier, l_r_sub );
    MakePatternPtr< SlaveCompareReplace<Statement> > r_mid( r_top_comp, stuff, r_slave );
    MakePatternPtr< SlaveSearchReplace<Statement> > r_slave3( r_mid, s_gg, r_ret_comp );
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

    Configure( fi );
}

*/


MergeFunctions::MergeFunctions()
{
    MakePatternPtr<Scope> s_module, r_module;
    MakePatternPtr<Field> thread, s_func;
    MakePatternPtr<Thread> thread_type;
    MakePatternPtr<Callable> func_type;
    MakePatternPtr< Star<Declaration> > members, thread_decls;
    MakePatternPtr< Star<Statement> > thread_stmts;
    MakePatternPtr< Star<Base> > bases;    
    MakePatternPtr< Overlay<Compound> > thread_over;
    MakePatternPtr<Compound> s_thread_comp, r_thread_comp;
    MakePatternPtr<Call> s_call, ls_call;
    MakePatternPtr<InstanceIdentifier> func_id;
    MakePatternPtr<Label> r_label;
    MakePatternPtr< BuildLabelIdentifier > r_label_id("ENTER_%s");
    MakePatternPtr< MatchAll<Compound> > s_all;
    MakePatternPtr< Stuff<Compound> > s_stuff, func_stuff;
    MakePatternPtr<TypeIdentifier> module_id;
    MakePatternPtr<Goto> lr_goto, mr_goto;
    MakePatternPtr<Return> ms_return;    
    MakePatternPtr<TempReturnAddress> retaddr;
    MakePatternPtr<InstanceIdentifier> retaddr_id;
    
    mr_goto->destination = retaddr_id;
     
    MakePatternPtr< SlaveSearchReplace<Compound> > slavem( func_stuff, ms_return, mr_goto );        
    
    ls_call->callee = func_id;
    lr_goto->destination = r_label_id;
        
    MakePatternPtr< SlaveSearchReplace<Compound> > slavel( r_thread_comp, ls_call, lr_goto );    
    
    s_module->members = (members, thread, s_func);
    r_module->members = (members, thread);
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
         
    Configure( s_module, r_module );
}


