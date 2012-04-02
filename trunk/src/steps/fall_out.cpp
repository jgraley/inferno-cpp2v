
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
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakeTreePtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (func_id, ls_label->identifier);
    l_lmap->identifier = r_lmap_id;
    l_lmap->initialiser = l_make;
    l_make->operands = (l_existing, l_minsert);
    l_existing->pattern = l_mnot;
    l_mnot->pattern = ls_label_id;
    l_minsert->insert = ls_label_id;
    l_label->identifier = ls_label_id;
    l_stuff->terminus = l_label;

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

