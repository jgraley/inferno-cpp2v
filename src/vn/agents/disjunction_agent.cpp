#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"
#include "lang/render.hpp"

using namespace VN;
using namespace SYM;

void DisjunctionAgent::SCRConfigure( Phase phase )
{
    AgentCommon::SCRConfigure(phase);

    options = make_shared< Collection<Node> >();
    for( const TreePtrInterface &p : GetDisjuncts() )
        options->insert( p );
}


shared_ptr<PatternQuery> DisjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterDecision(false); // Exclusive, please
    for( const TreePtrInterface &p : GetDisjuncts() )                 
        pq->RegisterNormalLink( PatternLink(&p) );    
    
    return pq;
}


SYM::Lazy<SYM::BooleanExpression> DisjunctionAgent::SymbolicNormalLinkedQuery(PatternLink keyer_plink) const
{
    // TODO implement SymbolicAutolocatingQuery()?
    ASSERT( GetDisjuncts().size() == 2 )
          ("Got %d choices; to support more than 2 disjuncts, enable SplitDisjunctions; fewer than 2 not allowed", GetDisjuncts().size());

    auto mmax_expr = MakeLazy<SymbolConstant>(XLink::MMAX);
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    
    list< shared_ptr<BooleanExpression> > is_mmax_exprs, is_keyer_exprs;
    list< shared_ptr<SymbolExpression> > disjunct_exprs;
    for( const TreePtrInterface &p : GetDisjuncts() )           
    {
        PatternLink disjunct_plink(&p);
        auto disjunct_expr = MakeLazy<SymbolVariable>(disjunct_plink);
        disjunct_exprs.push_back( disjunct_expr );
        is_mmax_exprs.push_back( disjunct_expr==mmax_expr );
        is_keyer_exprs.push_back( disjunct_expr==keyer_expr );
    }
           
    Lazy<BooleanExpression> main_expr = (is_mmax_exprs.front() & is_keyer_exprs.back()) | 
                                        (is_mmax_exprs.back() & is_keyer_exprs.front());

    // Don't forget the pre-restriction, applies in non-MMAX-keyer case
    main_expr &= SymbolicPreRestriction(keyer_plink) | (keyer_expr==mmax_expr); 
    return main_expr;
}


Syntax::Production DisjunctionAgent::GetAgentProduction() const
{
	return Syntax::Production::VN_DISJUNCTION;
}


string DisjunctionAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;

	// Commutative and associative so don't boost productions
	list<string> ls;
	for( const TreePtrInterface &p : GetDisjuncts() )                 
		ls.push_back( kit.render( (TreePtr<Node>)p, Syntax::Production::VN_DISJUNCTION ) );

	return Join(ls, " ∨ ");
}    
    

Graphable::NodeBlock DisjunctionAgent::GetGraphBlockInfo() const
{
    // The Disjunction node appears as a diamond with a ∨ character inside it. The affected subtrees are 
    // on the right.
    // NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
    // src/tree/operator_db.txt  
    NodeBlock block;
    block.bold = true;
    block.title = "Disjunction";
    block.symbol = string("∨");
    block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = { { "patterns", 
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
        block.item_blocks.front().links.push_back( link );
    }

    return block;
}


bool DisjunctionAgent::IsNonTrivialPreRestriction(const TreePtrInterface *tpi) const
{
    if( !AgentCommon::IsNonTrivialPreRestriction(tpi) )
		return false;
				
	// Disjunction only needs to match one child, so if we want to avoid a prerestriction
	// we need all of them to restrict
	return !All( AreChildrenRestricting() );
} 


bool DisjunctionAgent::IsFixedType() const
{
	return All( AreChildrenFixedType() );
}
