#include "conjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "up/commands.hpp"

using namespace SR;


shared_ptr<PatternQuery> ConjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    for( const TreePtrInterface &p : GetConjuncts() )                 
	    pq->RegisterNormalLink( PatternLink(this, &p) );
        
    return pq;
}


Agent::CommandPtr ConjunctionAgent::GenerateCommandImpl( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink key_xlink )
{
    // Conjuction and disjunction are ambiguous because there are 
    // multiple conjuncts/disjuncts
    ASSERT(key_xlink)("Unkeyed boolean agent seen in replace context");
    auto new_zone = make_unique<TreeZone>(TreeZone::CreateSubtree( key_xlink ));
	return make_unique<PopulateZoneCommand>( move(new_zone) );
}


Graphable::Block ConjunctionAgent::GetGraphBlockInfo() const
{
	// The Conjunction node appears as a diamond with a ∧ character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
    block.title = "Conjunction";
	block.symbol = string("∧");
	block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    for( const TreePtrInterface &p : GetConjuncts() )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &p );
        block.sub_blocks.front().links.push_back( link );
    }
    return block;
}
