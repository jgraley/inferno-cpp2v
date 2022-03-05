#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"

using namespace SR;
using namespace SYM;

void DisjunctionAgent::SCRConfigure( const SCREngine *e,
                                     Phase phase )
{
    AgentCommon::SCRConfigure(e, phase);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )
        options->insert( p );
}


shared_ptr<PatternQuery> DisjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterDecision(false); // Exclusive, please
    for( CollectionInterface::iterator pit = GetDisjuncts().begin(); pit != GetDisjuncts().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
    
    return pq;
}


SYM::Over<SYM::BooleanExpression> DisjunctionAgent::SymbolicNormalLinkedQueryImpl() const
{
    auto mmax_expr = MakeOver<SymbolConstant>(XLink::MMAX_Link);
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    
    list< shared_ptr<BooleanExpression> > is_mmax_exprs, is_keyer_exprs;
    list< shared_ptr<SymbolExpression> > disjunct_exprs;
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )           
    {
        PatternLink disjunct_plink(this, &p);
        auto disjunct_expr = MakeOver<SymbolVariable>(disjunct_plink);
        disjunct_exprs.push_back( disjunct_expr );
        is_mmax_exprs.push_back( disjunct_expr==mmax_expr );
        is_keyer_exprs.push_back( disjunct_expr==keyer_expr );
    }
          
    Over<BooleanExpression> non_mmax_case_expr;
    if( ReadArgs::split_disjunctions )
    {
        ASSERT( GetDisjuncts().size() == 2 )
              ("Got %d choices; to support more than 2 disjuncts, enable SplitDisjunctions; fewer than 2 not allowed", GetDisjuncts().size());
        // This is actually the only part that's hard with more than 2 disjuncts
        non_mmax_case_expr = is_mmax_exprs.front() & is_keyer_exprs.back() | is_mmax_exprs.back() & is_keyer_exprs.front();
    }
    else
    {
        non_mmax_case_expr = MakeOver<NonMMAXOperator>( keyer_expr, disjunct_exprs );
    }
        
    non_mmax_case_expr &= SymbolicPreRestriction(); // Don't forget the pre-restriction, applies in non-MMAX-keyer case
    
    return MakeOver<BooleanConditionalOperator>( keyer_expr == mmax_expr, 
                                                 MakeOver<AndOperator>( is_mmax_exprs ),
                                                 non_mmax_case_expr );
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
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )
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


DisjunctionAgent::NonMMAXOperator::NonMMAXOperator( shared_ptr<SYM::SymbolExpression> keyer_,
                                                    list<shared_ptr<SYM::SymbolExpression>> disjuncts_ ) :
    keyer( keyer_ ),
    disjuncts( disjuncts_ )
{    
}                                                


list<shared_ptr<SymbolExpression>> DisjunctionAgent::NonMMAXOperator::GetSymbolOperands() const
{
    list<shared_ptr<SymbolExpression>> l{ keyer };
    l = l + disjuncts;
    return l;
}


shared_ptr<BooleanResultInterface> DisjunctionAgent::NonMMAXOperator::Evaluate( const EvalKit &kit,
                                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()>=1 );        
    shared_ptr<SymbolResultInterface> keyer_result = op_results.front();
    list<shared_ptr<SymbolResultInterface>> disjunct_results = op_results;
    disjunct_results.pop_front();
    
    list<XLink> non_mmax_disjuncts;
    for( shared_ptr<SymbolResultInterface> dr : disjunct_results )
    {
        if( !dr->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
        XLink xlink = dr->GetAsXLink(); 
        if( xlink != XLink::MMAX_Link )
            non_mmax_disjuncts.push_back( xlink );
    }

    // Choose a checking strategy based on the number of non-MMAX residuals we saw. 
    // It should be 1.
    switch( non_mmax_disjuncts.size() )
    {
    case 0:
        // All were MMAX
        return make_shared<BooleanResult>( BooleanResult::DEFINED, false );
        break;        
        
    case 1:
        {
            // This is the correct number of non-MMAX. If we have a base, check against it.
            if( !keyer_result->IsDefinedAndUnique() )
                return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
            XLink keyer_xlink = keyer_result->GetAsXLink();

            XLink taken_option_x_link = OnlyElementOf(non_mmax_disjuncts);
            bool r = ( taken_option_x_link == keyer_xlink );
            return make_shared<BooleanResult>( BooleanResult::DEFINED, r );
            break;        
        }
        
    default: // 2 or more
        // It's never OK to have more than one non-MMAX (strict MMAX rules).
        return make_shared<BooleanResult>( BooleanResult::DEFINED, false );
        break;
    }
}


string DisjunctionAgent::NonMMAXOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> d : disjuncts )
        ls.push_back( RenderForMe(d) );
    return "NonMMAX(" + keyer->Render() + ": " + Join(ls, ", ") + ")"; 
}


Expression::Precedence DisjunctionAgent::NonMMAXOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}
