#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"
#include "up/tree_update.hpp"

using namespace SR;
using namespace SYM;

void DisjunctionAgent::SCRConfigure( const SCREngine *e,
                                     Phase phase )
{
    AgentCommon::SCRConfigure(e, phase);

    options = make_shared< Collection<Node> >();
    for( const TreePtrInterface &p : GetDisjuncts() )
        options->insert( p );
}


shared_ptr<PatternQuery> DisjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterDecision(false); // Exclusive, please
    for( const TreePtrInterface &p : GetDisjuncts() )                 
	    pq->RegisterNormalLink( PatternLink(this, &p) );    
    
    return pq;
}


SYM::Lazy<SYM::BooleanExpression> DisjunctionAgent::SymbolicNormalLinkedQuery() const
{
    ASSERT( GetDisjuncts().size() == 2 )
          ("Got %d choices; to support more than 2 disjuncts, enable SplitDisjunctions; fewer than 2 not allowed", GetDisjuncts().size());

    auto mmax_expr = MakeLazy<SymbolConstant>(XLink::MMAX_Link);
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    
    list< shared_ptr<BooleanExpression> > is_mmax_exprs, is_keyer_exprs;
    list< shared_ptr<SymbolExpression> > disjunct_exprs;
    for( const TreePtrInterface &p : GetDisjuncts() )           
    {
        PatternLink disjunct_plink(this, &p);
        auto disjunct_expr = MakeLazy<SymbolVariable>(disjunct_plink);
        disjunct_exprs.push_back( disjunct_expr );
        is_mmax_exprs.push_back( disjunct_expr==mmax_expr );
        is_keyer_exprs.push_back( disjunct_expr==keyer_expr );
    }
           
    Lazy<BooleanExpression> main_expr = is_mmax_exprs.front() & is_keyer_exprs.back() | is_mmax_exprs.back() & is_keyer_exprs.front();

    // Don't forget the pre-restriction, applies in non-MMAX-keyer case
    main_expr &= SymbolicPreRestriction() | keyer_expr==mmax_expr; 
    return main_expr;
}


Agent::CommandPtr DisjunctionAgent::GenerateCommandImpl( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink key_xlink )
{
    // Conjuction and disjunction are ambiguous because there are 
    // multiple conjuncts/disjuncts
    ASSERT(key_xlink)("Unkeyed boolean agent seen in replace context");
    auto new_zone = TreeZone::CreateSubtree( key_xlink );
    auto commands = make_unique<CommandSequence>();
	commands->Add( make_unique<DuplicateTreeZoneCommand>( new_zone ) );
	return commands;
}
                                                 

Graphable::Block DisjunctionAgent::GetGraphBlockInfo() const
{
	// The Disjunction node appears as a diamond with a ∨ character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
    block.title = "Disjunction";
	block.symbol = string("∨");
	block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    for( const TreePtrInterface &p : GetDisjuncts() )
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
