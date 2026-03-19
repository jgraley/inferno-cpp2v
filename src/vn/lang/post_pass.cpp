#include "post_pass.hpp"
#include "tree/misc.hpp"
#include "node/syntax.hpp"
#include "db/mutator.hpp"
#include "agents/embedded_scr_agent.hpp"
#include "agents/relocating_agent.hpp"

using namespace CPPTree;
using namespace VN;

PostPass::PostPass()
{
}


TreePtr<Node> PostPass::Run( TreePtr<Node> root )
{
	
	DData dd;
	dd.can_accept_code_unit = true;
	ProcessMutator( Mutator::CreateTreeRoot( &root ), dd );
	return root;
}


void PostPass::ProcessMutator( Mutator mutator, DData dd )
{	
	if( !dd.can_accept_code_unit )
	{
		if( auto cu = TreePtr<CPPTree::CodeUnit>(mutator.GetChildTreePtr()) )
		{
			ASSERT( cu->members.size() != 0 )("Unexpected empty CodeUnit"); // Genuine internal error
			ASSERT( cu->members.size() == 1 )("Unexpected multiple statement/decls\n")(cu->members); // TODO add location info
			(void)mutator.ExchangeChild(cu->members.front());			
		}
	}

	ProcessNode( mutator.GetChildTreePtr(), dd );
}


void PostPass::ProcessNode( TreePtr<Node> x, DData dd )
{
	ASSERT( x );

	if( Agent *a = Agent::TryAsAgent(x) )
	{
		if( dynamic_cast<ColocatingAgent *>(a) || // includes Stuff
			dynamic_cast<RelocatingAgent *>(a) )
			dd.can_accept_code_unit = false; 
	}
	else
	{
		// Cannot accept a code unit strictly-under ANY non-agent node including other code units
		dd.can_accept_code_unit = false;
	}	

	ProcessChildren( x, dd );
}


void PostPass::ProcessChildren( TreePtr<Node> x, DData dd )
{
	ASSERT( x );	

    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            ProcessSequence( x, x_seq, dd );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            ProcessCollection( x, x_col, dd );
        else if( TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(xe) )
            ProcessSingularItem( x, p_x_sing, dd );
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
