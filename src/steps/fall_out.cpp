
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


struct NestedBase : CompareReplace::SoftSearchPatternSpecialKey,
                    TerminusBase
{
    virtual TreePtr<Node> Advance( TreePtr<Node> n ) = 0;
    virtual shared_ptr<Key> DecidedCompare( const CompareReplace *sr,
                                 const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj )
    {
        INDENT;
        // Keep advancing until we get NULL, and remember the last non-null position
        TreePtr<Node> xt = x;
        int i = 0;
        while( TreePtr<Node> tt = Advance(xt) )
        {
            xt = tt;
            i++;
        } 
                
        // Compare the last position with the terminus pattern
        bool r = sr->DecidedCompare( xt, TreePtr<Node>(terminus), can_key, conj );
        
        // Compare the depth with the supplied pattern if present
        if( r && depth )
        {
            TreePtr<Node> cur_depth( new SpecificInteger(i) );
            r = sr->DecidedCompare( cur_depth, TreePtr<Node>(depth), can_key, conj );
        }
        
        if( r )
        {
            // Ensure the replace can terminate and overlay
            shared_ptr<TerminusKey> k( new TerminusKey );
            k->root = x;
            k->terminus = xt;
            return k;
        }
        else
        {
            return shared_ptr<Key>();
        }
    }    
    TreePtr<Integer> depth;
};

// Recurse through a number of nested Array nodes, but only by going through
// the "element" member, not the "size" member. So this will get you from the type
// of an instance to the type of the eventual element in a nested array decl.
struct NestedArray : NestedBase, Special<Type>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n )
    {
        if( TreePtr<Array> a = dynamic_pointer_cast<Array>(n) )            
            return a->element;
        else
            return TreePtr<Node>();
    }
};

// Recurse through a number of Subscript nodes, but only going through
// the base, not the index. Thus we seek the instance that contains the 
// data we strarted with.
struct NestedSubscript : NestedBase, Special<Expression>
{
    SPECIAL_NODE_FUNCTIONS
    virtual TreePtr<Node> Advance( TreePtr<Node> n )
    {
        if( TreePtr<Subscript> s = dynamic_pointer_cast<Subscript>(n) )            
            return s->operands[0]; // the base, not the index
        else
            return TreePtr<Node>();
    }
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
    MakeTreePtr<Pointer> r_pointer;
    MakeTreePtr<Void> r_void;
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
    r_lmap->constancy = MakeTreePtr<NonConst>();        
//    r_lmap->virt = MakeTreePtr<NonVirtual>();
  //  r_lmap->access = MakeTreePtr<Private>();    
    r_array->element = r_pointer;
    r_array->size = MakeTreePtr<Uninitialised>();
    r_pointer->destination = r_void;
    //r_make->operands = ()
    
    Configure( module, slavel );    
}


LabelVarsToEnum::LabelVarsToEnum()
{
    MakeTreePtr< MatchAll<Scope> > s_all;
    MakeTreePtr< MatchAll<Statement> > sx_all;
    MakeTreePtr<Scope> scope, l_scope;
    MakeTreePtr<Instance> var;
    MakeTreePtr<Pointer> s_ptr;
    MakeTreePtr<Void> s_void;
    MakeTreePtr<InstanceIdentifier> var_id;
    MakeTreePtr< Stuff<Scope> > s_stuff, sx_stuff;
    MakeTreePtr<Assign> s_assign, sx_assign, l_assign;
    MakeTreePtr<Subscript> s_sub, sx_sub, ls_sub, mr_sub, msx_sub;
    MakeTreePtr<InstanceIdentifier> array_id; // TODO arrange for this to be const, since if the array itself can change this step is broken
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
    MakeTreePtr<NestedSubscript> nested_subscript, nested_subscript2, nested_subscript3;
    MakeTreePtr<NestedSubscript> l_nested_subscript, m_nested_subscript, m_nested_subscript2;
    MakeTreePtr<Instance> msx_inst;
    MakeTreePtr<Integer> depth;
   
    l_assign->operands = (l_nested_subscript, l_over);
    l_nested_subscript->terminus = var_id;    
    l_nested_subscript->depth = depth;
    l_over->through = ls_sub;
    ls_sub->operands = (array_id, l_index);
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
    msx_sub->operands = (array_id, m_nested_subscript);
    mr_sub->operands = (array_id, m_nested_subscript);
    
    MakeTreePtr< SlaveSearchReplace<Scope> > slavem( slavel, ms_all, ms_anynode );   
   
    s_all->patterns = (scope, s_stuff, sx_not1);
    scope->members = (var, MakeTreePtr< Star<Declaration> >());
    var->type = nested_array;
    nested_array->terminus = over;
    nested_array->depth = depth;
    over->through = s_ptr;    
    var->identifier = var_id;
    s_ptr->destination = s_void;
    s_stuff->terminus = s_assign;
    s_assign->operands = (nested_subscript, s_sub);
    nested_subscript->terminus = var_id;
    nested_subscript->depth = depth;
    s_sub->operands = (array_id, s_index);
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
    sx_sub->operands = (array_id, MakeTreePtr<Expression>() );
    over->overlay = type;
    
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



