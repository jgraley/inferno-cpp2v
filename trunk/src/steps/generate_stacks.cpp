/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "sr/soft_patterns.hpp"

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
    MakeTreePtr<Subroutine> s_func;
    MakeTreePtr< MatchAll<Initialiser> > s_and;
    MakeTreePtr<Compound> s_top_comp, r_top_comp, r_ret_comp, temp;
    MakeTreePtr< Star<Declaration> > top_decls;
    MakeTreePtr< Star<Statement> > top_pre;
    MakeTreePtr< Stuff<Statement> > stuff;
    MakeTreePtr< Stuff<Compound> > cs_stuff;
    MakeTreePtr< Overlay<Statement> > overlay;
    MakeTreePtr<Automatic> cs_instance, s_instance;
    MakeTreePtr<Static> r_index, r_instance; // TODO Field
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

    SearchReplace::Configure( fi );
}


ExplicitiseReturn::ExplicitiseReturn()
{
    MakeTreePtr< Instance > fi;
    MakeTreePtr<Compound> s_comp, r_comp;        
    MakeTreePtr< Star<Statement> > pre;
    MakeTreePtr< Overlay<Compound> > over;
    MakeTreePtr< NotMatch<Statement> > sx_last;
    MakeTreePtr<Return> s_return, r_return;
    
    fi->type = MakeTreePtr<Subroutine>();
    fi->initialiser = over;
    s_comp->statements = (pre, sx_last);
    over->through = s_comp;
    sx_last->pattern = s_return;
    
    over->overlay = r_comp;
    r_comp->statements = (pre, sx_last, r_return);
    r_return->return_value = MakeTreePtr<Uninitialised>();
    
    SearchReplace::Configure( fi );
}    

