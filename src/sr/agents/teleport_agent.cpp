#include "teleport_agent.hpp"

#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- TeleportAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> TeleportAgent::SymbolicNormalLinkedQueryPRed() const                                      
{             
    shared_ptr<PatternQuery> my_pq = GetPatternQuery();         
    PatternLink child_plink = OnlyElementOf( my_pq->GetNormalLinks() );
    
    SYM::Lazy<SYM::SymbolExpression> keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    SYM::Lazy<SYM::SymbolExpression> child_expr = MakeLazy<SymbolVariable>(child_plink);
    
    return MakeLazy<TeleportOperator>( this, keyer_expr ) == child_expr;
}                     


TreePtr<Node> TeleportAgent::GetDomainExtraNode( const XTreeDatabase *db, XLink keyer_xlink, set<XLink> &deps ) const
{
	deps.clear();
	
	if( keyer_xlink == XLink::MMAX_Link )
		return TreePtr<Node>(); // MMAX at base never expands domain because then, all child patterns are also MMAX
	if( !IsPreRestrictionMatch(keyer_xlink) )
		return TreePtr<Node>(); // Failed pre-restriction so can't expand domain

	DepRep dep_rep;
	TeleportResult tp_result;
	try
	{
		tp_result = RunTeleportQuery( db, &dep_rep, keyer_xlink );
	}
	catch( ::Mismatch & ) 
	{
		return TreePtr<Node>();
	}
	if( !tp_result.second )
		return TreePtr<Node>();       // NULL  

	deps = dep_rep.GetDeps();	
	
	if( tp_result.first ) // parent link was supplied
	{
		ASSERT( tp_result.first.GetChildX() == tp_result.second );    
		ASSERT( deps.count( tp_result.first ) > 0 );
		return TreePtr<Node>(); // Don't bother Domain when there's an XLink
	}		

	return tp_result.second;               
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
}


bool TeleportAgent::IsExtenderLess( const Extender &r ) const
{
	return GetExtenderOrdinal() < r.GetExtenderOrdinal();
}


TeleportAgent::TeleportOperator::TeleportOperator( const TeleportAgent *agent_,
                                                   shared_ptr<SymbolExpression> keyer_ ) :
    agent( agent_ ),
    keyer( keyer_ )
{    
}                                                


list<shared_ptr<SymbolExpression>> TeleportAgent::TeleportOperator::GetSymbolOperands() const
{
    return { keyer };
}


unique_ptr<SymbolResultInterface> TeleportAgent::TeleportOperator::Evaluate( const EvalKit &kit,
                                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
	// Extract xlink from symbolic result
    ASSERT( op_results.size()==1 );            
    unique_ptr<SymbolResultInterface> keyer_result = OnlyElementOf(move(op_results));
    if( !keyer_result->IsDefinedAndUnique() )
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    XLink keyer_xlink = keyer_result->GetOnlyXLink();
        
    // Apply the teleporting operation to the xlink. It may create new nodes
    // so it returns a TreePtr<Node> to avoid creating new xlink without base.
    TeleportResult tp_result = agent->RunTeleportQuery( kit.x_tree_db, nullptr, keyer_xlink );

    // Teleporting operation can fail: if so call it a NaS
    if( !tp_result.second )
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );        
        
    // If we got an XLink, just return it, don't bother Domain
    if( tp_result.first ) // parent link was supplied
    {
         ASSERT( tp_result.first.GetChildX() == tp_result.second );
         return make_unique<SymbolResult>( tp_result.first );
	}

    // We are required to have already added the new node to the domain
    // during domain extension, so use the node to fetch the unbique XLink
    XLink unique_xlink = kit.x_tree_db->GetUniqueDomainExtension(agent, tp_result.second);
    
    // Form a symbol result to return.       
    return make_unique<SymbolResult>( unique_xlink );
}


Orderable::Diff TeleportAgent::TeleportOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                                      OrderProperty order_property ) const 
{
    auto &r = GET_THAT_REFERENCE(right);
    // Agents aren't comparable, so value of operator is identiy of agent
    return Node::Compare3WayIdentity( *agent->GetPatternPtr(), *r.agent->GetPatternPtr() );
}  


string TeleportAgent::TeleportOperator::Render() const
{
    return agent->GetName() + "(" + keyer->Render() + ")"; 
}


Expression::Precedence TeleportAgent::TeleportOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}


const TeleportAgent *TeleportAgent::TeleportOperator::GetAgent() const
{
	return agent;
}


void TeleportAgent::DepRep::ReportTreeNode( const TreePtrInterface *p_tree_ptr )
{
	if( depth==0 )
    {
        XLink xlink( (TreePtr<Node>)*p_tree_ptr, p_tree_ptr );
        deps.insert( xlink );
    }
}


void TeleportAgent::DepRep::EnterTreeTransformation( Transformation *tx )
{
    depth++;
}


void TeleportAgent::DepRep::ExitTreeTransformation()
{
    depth--;
}


set<XLink> TeleportAgent::DepRep::GetDeps() const
{
	return deps;
}


void TeleportAgent::DepRep::Clear()
{
	deps.clear();
}	
	
