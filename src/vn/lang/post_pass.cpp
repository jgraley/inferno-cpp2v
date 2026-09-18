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

static TreePtr<Node> MakeStandardAgentFromTypeID(const type_info &ti)
{
#define NODE(NS, NAME) \
	if( ti == typeid(NS::NAME) ) \
		return MakeTreeNode<StandardAgentWrapper<NS::NAME>>(); \
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
	TreePtr<Node> old_x = mutator.GetChildTreePtr();
	
	bool needs_standard_agent = !Agent::TryAsAgent(old_x);
	if( needs_standard_agent )
	{
		const type_info &old_x_ti = typeid(old_x);
		TreePtr<Node> new_x = MakeStandardAgentFromTypeID(old_x_ti);
		(void)new_x;
		//(void)mutator.ExchangeChild(cu->members.front());			
	}
	
	ProcessNode( old_x, old_x, dd );
}


void PostPass::ProcessNode( TreePtr<Node> old_x, TreePtr<Node> new_x, DData dd )
{
	ASSERT( old_x );	
	ASSERT( new_x );	
	ProcessChildren( old_x, new_x, dd );
}


void PostPass::ProcessChildren( TreePtr<Node> old_x, TreePtr<Node> new_x, DData dd )
{
	ASSERT( old_x );	
	ASSERT( new_x );
	
    vector< Itemiser::Element * > old_x_items = old_x->Itemise();
    vector< Itemiser::Element * > new_x_items = new_x->Itemise();
    
  /*  Agent *a = Agent::TryAsAgent(x);
    if( a && !dynamic_cast<StandardAgent *>(a) )
    {
		shared_ptr<PatternQuery> pq = a->GetPatternQuery();
		list<PatternLink> al = pq->GetAllLinks();
		FTRACE(x)(" itemises to ")(x_items)("\n and pqs to ")(al)("\n");
	}
    */
    for( pair<Itemiser::Element *, Itemiser::Element *> p : Zip(old_x_items, new_x_items) )
    {
        if( auto *old_x_seq = dynamic_cast<SequenceInterface *>(p.first) )
        {
			auto *new_x_seq = dynamic_cast<SequenceInterface *>(p.second);
			ASSERT( new_x_seq );
            ProcessSequence( new_x, old_x_seq, new_x_seq, dd );
		}
        else if( auto *old_x_col = dynamic_cast<CollectionInterface *>(p.first) )
        {
			auto *new_x_col = dynamic_cast<CollectionInterface *>(p.second);
			ASSERT( new_x_col );
            ProcessCollection( new_x, old_x_col, new_x_col, dd );
		}
        else if( TreePtrInterface *old_p_x_sing = dynamic_cast<TreePtrInterface *>(p.first) )
        {
			auto *new_p_x_sing = dynamic_cast<TreePtrInterface *>(p.second);
			ASSERT( new_p_x_sing );
            ProcessSingularItem( new_x, old_p_x_sing, new_p_x_sing, dd );
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void PostPass::ProcessSingularItem( TreePtr<Node> new_x, TreePtrInterface *old_p_x_sing, TreePtrInterface *new_p_x_sing, DData dd )
{
	ASSERT( old_p_x_sing==new_p_x_sing );
	if( *new_p_x_sing ) // Permitting NULL because patterns
		ProcessMutator( Mutator::CreateTreeSingular( new_x, new_p_x_sing ), dd );
}


void PostPass::ProcessSequence( TreePtr<Node> new_x, SequenceInterface *old_x_seq, SequenceInterface *new_x_seq, DData dd )
{
 	ASSERT( old_x_seq==new_x_seq );
    for( SequenceInterface::iterator it = old_x_seq->begin();
		 it != old_x_seq->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Sequence, which isn't allowed even for patterns");
		ProcessMutator( Mutator::CreateTreeContainer( new_x, new_x_seq, it ), dd );
    }
}


void PostPass::ProcessCollection( TreePtr<Node> new_x, CollectionInterface *old_x_col, CollectionInterface *new_x_col, DData dd )
{
 	ASSERT( old_x_col==new_x_col );
    for( CollectionInterface::iterator it = old_x_col->begin();
		 it != old_x_col->end();
		 ++it )
    {
		ASSERT((TreePtr<Node>)*it)("Got NULL in a Collection, which isn't allowed even for patterns");		
		ProcessMutator( Mutator::CreateTreeContainer( new_x, new_x_col, it ), dd );
	} 
}
