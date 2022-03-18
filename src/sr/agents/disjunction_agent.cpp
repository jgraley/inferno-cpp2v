#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
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
           
    Over<BooleanExpression> main_expr;
    if( ReadArgs::split_disjunctions )
    {
        ASSERT( GetDisjuncts().size() == 2 )
              ("Got %d choices; to support more than 2 disjuncts, enable SplitDisjunctions; fewer than 2 not allowed", GetDisjuncts().size());
        main_expr = is_mmax_exprs.front() & is_keyer_exprs.back() | is_mmax_exprs.back() & is_keyer_exprs.front();
    }
    else
    {
#if 0
        main_expr = MakeOver<WideMainBoolOperator>( is_keyer_exprs, is_mmax_exprs );
#else
        main_expr = MakeOver<WideMainOperator>( keyer_expr, disjunct_exprs );
#endif
    }
    // Don't forget the pre-restriction, applies in non-MMAX-keyer case
    main_expr &= SymbolicPreRestriction() | keyer_expr==mmax_expr; 
    return main_expr;
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


DisjunctionAgent::WideMainOperator::WideMainOperator( shared_ptr<SYM::SymbolExpression> keyer_,
                                                    list<shared_ptr<SYM::SymbolExpression>> disjuncts_ ) :
    keyer( keyer_ ),
    disjuncts( disjuncts_ )
{    
}                                                


list<shared_ptr<SymbolExpression>> DisjunctionAgent::WideMainOperator::GetSymbolOperands() const
{
    list<shared_ptr<SymbolExpression>> l{ keyer };
    l = l + disjuncts;
    return l;
}


shared_ptr<BooleanResultInterface> DisjunctionAgent::WideMainOperator::Evaluate( const EvalKit &kit,
                                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()>=1 );        
    shared_ptr<SymbolResultInterface> keyer_result = op_results.front();
    list<shared_ptr<SymbolResultInterface>> disjunct_results = op_results;
    disjunct_results.pop_front();
    
    if( !keyer_result->IsDefinedAndUnique() )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    XLink keyer_xlink = keyer_result->GetAsXLink();

    int num_keyer_disjuncts = 0;
    int num_mmax_disjuncts = 0;
    int num_disjuncts = 0;
    for( shared_ptr<SymbolResultInterface> dr : disjunct_results )
    {
        if( !dr->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

        XLink xlink = dr->GetAsXLink(); 
        if( xlink != XLink::MMAX_Link && xlink != keyer_xlink )
            return make_shared<BooleanResult>( BooleanResult::DEFINED, false );

        num_keyer_disjuncts += (xlink == keyer_xlink);
        num_mmax_disjuncts += (xlink == XLink::MMAX_Link);
        num_disjuncts++;
    }

    bool ok_keyer_mmax = (num_keyer_disjuncts==num_disjuncts && 
                          num_mmax_disjuncts==num_disjuncts);
    bool ok_keyer_non_mmax = (num_keyer_disjuncts==1 && 
                              num_mmax_disjuncts==num_disjuncts-1);
                          
    return make_shared<BooleanResult>( BooleanResult::DEFINED, ok_keyer_mmax || ok_keyer_non_mmax );
}


string DisjunctionAgent::WideMainOperator::RenderNF() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> d : disjuncts )
        ls.push_back( RenderForMe(d) );
    return "DisjunctionWideMain(" + keyer->Render() + ": " + Join(ls, ", ") + ")"; 
}


Expression::Precedence DisjunctionAgent::WideMainOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}







DisjunctionAgent::WideMainBoolOperator::WideMainBoolOperator( list<shared_ptr<SYM::BooleanExpression>> is_keyer_disjuncts_,
                                                              list<shared_ptr<SYM::BooleanExpression>> is_mmax_disjuncts_ ) :
    num_disjuncts( is_keyer_disjuncts_.size() ),
    is_keyer_disjuncts( is_keyer_disjuncts_ ),
    is_mmax_disjuncts( is_mmax_disjuncts_ )
{    
}                                                


list<shared_ptr<BooleanExpression>> DisjunctionAgent::WideMainBoolOperator::GetBooleanOperands() const
{
    list<shared_ptr<BooleanExpression>> l;
    l = l + is_keyer_disjuncts;
    l = l + is_mmax_disjuncts;
    return l;
}


shared_ptr<BooleanResultInterface> DisjunctionAgent::WideMainBoolOperator::Evaluate( const EvalKit &kit,
                                                                                     const list<shared_ptr<BooleanResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size() == num_disjuncts*2 ); 
    typedef pair<shared_ptr<BooleanResultInterface>, shared_ptr<BooleanResultInterface>> ResultPair;
    list<ResultPair> zipped_results;
    list<shared_ptr<BooleanResultInterface>>::const_iterator keyer_it = op_results.begin(), mmax_it = op_results.begin();
    for( int i=0; i<num_disjuncts; i++ )
        mmax_it++; // move mit to the MMAX results    
    for( int i=0; i<num_disjuncts; i++ )    
        zipped_results.push_back( make_pair(*keyer_it++, *mmax_it++) );
    ASSERT( mmax_it == op_results.end() );
    
    int num_keyer_disjuncts = 0;
    int num_mmax_disjuncts = 0;
    for( const ResultPair &p : zipped_results )
    {
        if( !p.first->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
        if( !p.second->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
            
        if( !p.first->GetAsBool() && !p.second->GetAsBool() )
            return make_shared<BooleanResult>( BooleanResult::DEFINED, false );

        num_mmax_disjuncts += p.first->GetAsBool() ? 1 : 0;
        num_keyer_disjuncts += p.second->GetAsBool() ? 1 : 0;    
    }

    bool ok_keyer_mmax = (num_keyer_disjuncts==num_disjuncts && 
                          num_mmax_disjuncts==num_disjuncts);
    bool ok_keyer_non_mmax = (num_keyer_disjuncts==1 && 
                              num_mmax_disjuncts==num_disjuncts-1);
                          
    return make_shared<BooleanResult>( BooleanResult::DEFINED, ok_keyer_mmax || ok_keyer_non_mmax );
}


string DisjunctionAgent::WideMainBoolOperator::Render() const
{
    list<string> kls, mls;
    for( shared_ptr<Expression> d : is_keyer_disjuncts )
        kls.push_back( RenderForMe(d) );
    for( shared_ptr<Expression> d : is_mmax_disjuncts )
        mls.push_back( RenderForMe(d) );
    return "DisjunctionWideMainBool(" + Join(kls, ", ") + ": " + Join(mls, ", ") + ")"; 
}


Expression::Precedence DisjunctionAgent::WideMainBoolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}
