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

struct ReturnAddressTemp : Field
{
	NODE_FUNCTIONS_FINAL
};

struct ReturnAddress : Automatic
{
	NODE_FUNCTIONS_FINAL
};


AddReturnAddress::AddReturnAddress()
{
    MakeTreePtr<Module> s_module, r_module;
    MakeTreePtr< Star<Declaration> > decls;    
    MakeTreePtr< Star<Base> > bases;    
    MakeTreePtr<ReturnAddressTemp> r_retaddr;
    MakeTreePtr<BuildInstanceIdentifier> r_retaddr_id("return_address_temp");
    MakeTreePtr<ReturnAddress> lr_retaddr;
    MakeTreePtr<BuildInstanceIdentifier> lr_retaddr_id("return_address");
    MakeTreePtr<Pointer> r_ptr, lr_ptr;
    MakeTreePtr< NotMatch<Declaration> > s_nm, ls_nm;
    MakeTreePtr<Instance> l_inst;
    MakeTreePtr< Overlay<Compound> > l_over;
    MakeTreePtr<Compound> ls_comp, lr_comp;
    MakeTreePtr< Star<Declaration> > l_decls;
    MakeTreePtr< Star<Statement> > l_stmts, llsx_stmts;
    MakeTreePtr<Assign> lr_assign, llsx_assign;
    MakeTreePtr<Call> ll_call;
    MakeTreePtr<Compound> llr_comp, llsx_comp;
    MakeTreePtr< TransformOf<Expression> > ll_typeof( &TypeOf::instance );
    MakeTreePtr<Label> llr_label;
    MakeTreePtr<BuildLabelIdentifier> llr_labelid("RETURN");
    MakeTreePtr<Assign> llr_assign;
    MakeTreePtr< MatchAll<Statement> > ll_all;
    MakeTreePtr< AnyNode<Statement> > ll_any; // TODO rename AnyNode -> Blob
    MakeTreePtr< NotMatch<Statement> > lls_not;
    MakeTreePtr< Overlay<Statement> > ll_over;
    MakeTreePtr<Function> l_func;
   
    ll_all->patterns = (ll_any, lls_not);
    ll_any->terminus = ll_over;
    ll_over->through = ll_call;
    ll_call->callee = ll_typeof;
    ll_typeof->pattern = l_func;
    //lls_call->operands = ();
    lls_not->pattern = llsx_comp;
    llsx_comp->statements = (llsx_assign, llsx_stmts);
    llsx_assign->operands = (r_retaddr_id, llr_labelid);
    ll_over->overlay = llr_comp;
    llr_comp->statements = (llr_assign, ll_call, llr_label);  
    llr_assign->operands = (r_retaddr_id, llr_labelid);
    llr_label->identifier = llr_labelid;    
    
    MakeTreePtr< SlaveSearchReplace<Module> > l_slave( r_module, ll_all, ll_all );
    
    l_inst->type = l_func;
    //l_func->members = ();
    l_func->return_type = MakeTreePtr<Void>();
    l_inst->initialiser = l_over;
    l_over->through = ls_comp;
    ls_comp->members = (l_decls);
    ls_comp->statements = (l_stmts);
    l_over->overlay = lr_comp;
    lr_comp->members = (l_decls, lr_retaddr);
    l_decls->pattern = ls_nm;
    ls_nm->pattern = MakeTreePtr<ReturnAddress>();    
    lr_retaddr->identifier = lr_retaddr_id;
    lr_retaddr->type = lr_ptr;
    lr_ptr->destination = MakeTreePtr<Void>();
    lr_retaddr->initialiser = MakeTreePtr<Uninitialised>();
    lr_comp->statements = (lr_assign, l_stmts);
    lr_assign->operands = (lr_retaddr_id, r_retaddr_id);
    
    MakeTreePtr< SlaveSearchReplace<Module> > slave( l_slave, l_inst, l_inst );
    
    s_module->members = (decls);
    s_module->bases = (bases);
    decls->pattern = s_nm;
    s_nm->pattern = MakeTreePtr<ReturnAddressTemp>();    
    r_module->members = (decls, r_retaddr);
    r_module->bases = (bases);
    r_retaddr->identifier = r_retaddr_id;
    r_retaddr->type = r_ptr;
    r_ptr->destination = MakeTreePtr<Void>();
    r_retaddr->initialiser = MakeTreePtr<Uninitialised>();
    r_retaddr->virt = MakeTreePtr<NonVirtual>();
    r_retaddr->access = MakeTreePtr<Private>();
    r_retaddr->constancy = MakeTreePtr<Const>();
    
    Configure( s_module, slave );  
}


UseTempForReturn::UseTempForReturn()
{
    // search for return statement in a compound (TODO don't think we need the outer compound)
	TreePtr<Return> s_return( new Return );
	TreePtr< MatchAll<Expression> > s_and( new MatchAll<Expression> );
	s_return->return_value = s_and;
    MakeTreePtr< TransformOf<Expression> > retval( &TypeOf::instance );
	MakeTreePtr<Type> type;
	retval->pattern = type;
    
    // Restrict the search to returns that have an automatic variable under them
    TreePtr< Stuff<Expression> > cs_stuff( new Stuff<Expression> );
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
    MakeTreePtr<Thread> s_func;
    MakeTreePtr< MatchAll<Initialiser> > s_and;
    MakeTreePtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakeTreePtr< Star<Declaration> > top_decls;
    MakeTreePtr< Star<Statement> > top_pre;
    MakeTreePtr< Stuff<Statement> > stuff;
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
    //r_index->initialiser = MakeTreePtr<SpecificInteger>(0);
    r_index_init->operands = (r_index_identifier, MakeTreePtr<SpecificInteger>(0));
    r_index->virt = MakeTreePtr<NonVirtual>();
    r_index->access = MakeTreePtr<Private>();

    // top-level statements
    r_inc->operands = ( r_index_identifier );
    r_top_comp->statements = ( r_index_init, r_inc, top_pre );

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
    r_instance->virt = MakeTreePtr<NonVirtual>();
    r_instance->access = MakeTreePtr<Private>();
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

ExtractFields::ExtractFields()
{
    MakeTreePtr<Module> s_rec, r_rec;
    MakeTreePtr< Star<Declaration> > decls, vdecls;
    MakeTreePtr< Star<Statement> > vstmts;
    MakeTreePtr<Field> fn, var;
    MakeTreePtr<Callable> ft;
    MakeTreePtr< Stuff<Initialiser> > stuff;
    MakeTreePtr<Compound> s_comp, r_comp;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< Star<Base> > bases;
    
    s_rec->members = (decls, fn);
    s_rec->bases = bases;
    fn->type = ft;
    fn->initialiser = stuff;
    // TODO recurse restriction for locally declared classes
    stuff->terminus = over;
    over->through = s_comp;
    s_comp->members = (vdecls, var);
    s_comp->statements = (vstmts);
     
    r_rec->members = (decls, fn, var);
    r_rec->bases = bases;
    over->overlay = r_comp;
    r_comp->members = (vdecls);
    r_comp->statements = (vstmts);
    
    Configure( s_rec, r_rec );
}


