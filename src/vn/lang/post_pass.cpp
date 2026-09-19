#include "post_pass.hpp"
#include "tree/misc.hpp"
#include "db/mutator.hpp"
#include "agents/embedded_scr_agent.hpp"
#include "agents/relocating_agent.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "tree/localtree.hpp"

using namespace CPPTree;
using namespace VN;

static TreePtr<Node> CloneToStandardAgent(TreePtr<Node> x)
{
	const type_info &ti = typeid(*x);
#define NODE(NS, NAME) \
	if( ti == typeid(NS::NAME) ) \
	{ \
		auto tx = TreePtr<NS::NAME>::DynamicCast(x); \
		TreePtr<Node> ax = MakeTreeNode< StandardAgentWrapper<NS::NAME> >(*tx); \
		return ax; \
	} \
	else
#include "tree/node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "tree/operator_data.inc"
#undef NODE
		
		// By design we should have a case for every value of the node enum
		ASSERT(false)("Could not find node for type info: ")(Traceable::CPPFilt(ti.name()));  // be the last else clause
		
	ASSERTFAIL();
}


PostPass::PostPass()
{
}


TreePtr<Node> PostPass::Run( TreePtr<Node> root )
{
	
	DData dd;
	ProcessMutator( Mutator::CreateTreeRoot( &root ), dd );
	return root;
}


void PostPass::ProcessMutator( Mutator mutator, DData dd )
{	
	INDENT("M");
	TreePtr<Node> old_x = mutator.GetChildTreePtr();
	
	bool needs_standard_agent = !Agent::TryAsAgent(old_x);
	if( changes.contains(old_x) )
	{	
		// Coupled node we've reached before: don't recurse into it, just swap it in
		TreePtr<Node> new_x = changes.at(old_x);
		TRACE(old_x)(" -> ")(new_x)(" (revisit)\n");
		(void)mutator.ExchangeChild(new_x);		
	}
	else if( needs_standard_agent )
	{		
		TreePtr<Node> new_x = CloneToStandardAgent(old_x);
		TRACE(old_x)(" -> ")(new_x)("\n");
		(void)mutator.ExchangeChild(new_x);	
		// Copy children from old to new
		ProcessNode( new_x, dd );	
		
		// In case a coupling that we'll reach again
		changes[old_x] = new_x;
	}
	else
	{
		// Alias old and new prevents any copying
		TRACE(old_x)("\n");
		ProcessNode( old_x, dd );
	}
}


void PostPass::ProcessNode( TreePtr<Node> new_x, DData dd )
{	
	ASSERT( new_x );	
	ProcessChildren( new_x, dd );
}


void PostPass::ProcessChildren( TreePtr<Node> new_x, DData dd )
{
	ASSERT( new_x );
	
    vector< Itemiser::Element * > x_items = new_x->Itemise();
    
    for( Itemiser::Element *item : x_items )
    {
        if( auto *x_seq = dynamic_cast<SequenceInterface *>(item) )
        {
            ProcessSequence( new_x, x_seq, dd );
		}
        else if( auto *x_col = dynamic_cast<CollectionInterface *>(item) )
        {
            ProcessCollection( new_x, x_col, dd );
		}
        else if( TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(item) )
        {
            ProcessSingularItem( new_x, p_x_sing, dd );
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void PostPass::ProcessSingularItem( TreePtr<Node> new_x, TreePtrInterface *new_p_x_sing, DData dd )
{	
	if( *new_p_x_sing ) // Permitting NULL because patterns
		ProcessMutator( Mutator::CreateTreeSingular( new_x, new_p_x_sing ), dd );
}


void PostPass::ProcessSequence( TreePtr<Node> new_x, SequenceInterface *new_x_seq, DData dd )
{ 
    for( SequenceInterface::iterator it = new_x_seq->begin();
		 it != new_x_seq->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Sequence, which isn't allowed even for patterns");
		ProcessMutator( Mutator::CreateTreeContainer( new_x, new_x_seq, it ), dd );
    }
}


void PostPass::ProcessCollection( TreePtr<Node> new_x, CollectionInterface *new_x_col, DData dd )
{
    for( CollectionInterface::iterator it = new_x_col->begin();
		 it != new_x_col->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Collection, which isn't allowed even for patterns");		
		ProcessMutator( Mutator::CreateTreeContainer( new_x, new_x_col, it ), dd );
	} 
}
