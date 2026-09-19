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
		return MakeTreeNode< StandardAgentWrapper<NS::NAME> >(*tx); \
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
	TreePtr<Node> x = mutator.GetChildTreePtr();
	if( reached.contains(x) )
		return;
		
	Agent *agent = Agent::TryAsAgent(x);
	if( changes.contains(x) )
	{	
		// Coupled node we've changed before: don't recurse into it, just swap it in
		TreePtr<Node> new_x = changes.at(x);
		TRACE(x)(" -> ")(new_x)(" (revisit)\n");
		(void)mutator.ExchangeChild(new_x);		
	}
	else if( !agent )
	{		
		TreePtr<Node> new_x = CloneToStandardAgent(x);
		TRACE(x)(" -> ")(new_x)("\n");
		(void)mutator.ExchangeChild(new_x);	
		// Copy children from old to new
		ProcessNode( new_x, dd );	
		
		// In case a coupling that we'll reach again
		changes[x] = new_x;
		reached.insert( new_x );
	}
	else
	{
		ASSERT( !dynamic_cast<StandardAgent *>(agent) )("Parsing produced a standard agent ")(x);
		TRACE(x)("\n");
		ProcessNode( x, dd );
		reached.insert( x );
	}
}


void PostPass::ProcessNode( TreePtr<Node> x, DData dd )
{	
	ASSERT( x );	
	ProcessChildren( x, dd );
}


void PostPass::ProcessChildren( TreePtr<Node> x, DData dd )
{
	ASSERT( x );
	
    vector< Itemiser::Element * > x_items = x->Itemise();
    
    for( Itemiser::Element *item : x_items )
    {
        if( auto *x_seq = dynamic_cast<SequenceInterface *>(item) )
        {
            ProcessSequence( x, x_seq, dd );
		}
        else if( auto *x_col = dynamic_cast<CollectionInterface *>(item) )
        {
            ProcessCollection( x, x_col, dd );
		}
        else if( TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(item) )
        {
            ProcessSingularItem( x, p_x_sing, dd );
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void PostPass::ProcessSingularItem( TreePtr<Node> x, TreePtrInterface *p_x_sing, DData dd )
{	
	if( *p_x_sing ) // Permitting NULL because patterns
		ProcessMutator( Mutator::CreateTreeSingular( x, p_x_sing ), dd );
}


void PostPass::ProcessSequence( TreePtr<Node> x, SequenceInterface *x_seq, DData dd )
{ 
    for( SequenceInterface::iterator it = x_seq->begin();
		 it != x_seq->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Sequence, which isn't allowed even for patterns");
		ProcessMutator( Mutator::CreateTreeContainer( x, x_seq, it ), dd );
    }
}


void PostPass::ProcessCollection( TreePtr<Node> x, CollectionInterface *x_col, DData dd )
{
    for( CollectionInterface::iterator it = x_col->begin();
		 it != x_col->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Collection, which isn't allowed even for patterns");		
		ProcessMutator( Mutator::CreateTreeContainer( x, x_col, it ), dd );
	} 
}
