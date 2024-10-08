#include "teleport_agent.hpp"

#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"
#include "up/zone_commands.hpp"

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


TreePtr<Node> TeleportAgent::GetDomainExtraNode( const XTreeDatabase *db, XLink start_xlink, set<TreePtr<Node>> &deps ) const
{
	deps.clear();
	
	if( start_xlink == XLink::MMAX_Link )
		return TreePtr<Node>(); // MMAX at base never expands domain because then, all child patterns are also MMAX
	if( !IsPreRestrictionMatch(start_xlink) )
		return TreePtr<Node>(); // Failed pre-restriction so can't expand domain

	DepRep dep_rep;
	QueryReturnType tp_result;
	try
	{
		tp_result = RunTeleportQuery( db, &dep_rep, start_xlink );
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
		ASSERT( tp_result.first.GetChildX() == tp_result.second ); // Consistency
		ASSERT( deps.count( tp_result.first.GetChildX() ) > 0 ); // Result should be a dep
		return TreePtr<Node>(); // Don't bother Domain when there's an XLink
	}		

	return tp_result.second;               
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
}


bool TeleportAgent::IsExtenderChannelLess( const Extender &r ) const
{
	return GetExtenderChannelOrdinal() < r.GetExtenderChannelOrdinal();
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


unique_ptr<SymbolicResult> TeleportAgent::TeleportOperator::Evaluate( const EvalKit &kit,
                                                                      list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
	// Extract xlink from symbolic result
    ASSERT( op_results.size()==1 );            
    unique_ptr<SymbolicResult> start_result = OnlyElementOf(move(op_results));
    if( !start_result->IsDefinedAndUnique() )
        return make_unique<EmptyResult>();
    XLink start_xlink = start_result->GetOnlyXLink();
        
    // Apply the teleporting operation to the xlink. It may create new nodes
    // so it returns a TreePtr<Node> to avoid creating new xlink without base.
    QueryReturnType tp_result = agent->RunTeleportQuery( kit.x_tree_db, nullptr, start_xlink );

    // Teleporting operation can fail: if so call it a NaS
    if( !tp_result.second )
        return make_unique<EmptyResult>();        
        
    // If we got an XLink, just return it, don't bother DomainExtension
    if( tp_result.first ) // parent link was supplied
    {
         ASSERT( tp_result.first.GetChildX() == tp_result.second );
         return make_unique<UniqueResult>( tp_result.first );
	}

    // We are required to have already added the new node to the domain
    // during domain extension, so use the node to fetch the unbique XLink
    XLink unique_xlink = kit.x_tree_db->GetDEChannel(agent)->GetUniqueDomainExtension(start_xlink, tp_result.second);
    
    // Form a symbol result to return.       
    return make_unique<UniqueResult>( unique_xlink );
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


void TeleportAgent::DepRep::ReportTreeNode( TreePtr<Node> tree_ptr )
{
    deps.insert( tree_ptr );
}


set<TreePtr<Node>> TeleportAgent::DepRep::GetDeps() const
{
	return deps;
}


void TeleportAgent::DepRep::Clear()
{
	deps.clear();
}	
	
