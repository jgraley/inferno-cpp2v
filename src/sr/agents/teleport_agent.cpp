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

//---------------------------------- TeleportAgent::Dependencies ------------------------------------    

void TeleportAgent::Dependencies::AddDep( XLink dep )
{
	deps.insert( dep );
}		


void TeleportAgent::Dependencies::AddChainTo( shared_ptr<Dependencies> chain )
{
	chains.insert( chain );
}


void TeleportAgent::Dependencies::CopyAllFrom( const Dependencies &other )
{
    for( XLink d : other.deps )    
		AddDep(d);
    for( shared_ptr<Dependencies> c : other.chains )    
		AddChainTo(c);
}


set<XLink> TeleportAgent::Dependencies::GetAll() const
{
	set<XLink> all_deps = deps;
    for( shared_ptr<Dependencies> c : chains )    
	{
		set<XLink> c_deps = c->GetAll();
		for( XLink d : c_deps )   
			all_deps.insert(d);
	}	
	
	return all_deps;
}


void TeleportAgent::Dependencies::Clear()
{
	deps.clear();
}	


//---------------------------------- QueryReturnType ------------------------------------    

TeleportAgent::QueryReturnType::QueryReturnType() :
	de_info({ nullptr, {} }),
	base_xlink()
{
}
	
	
TeleportAgent::QueryReturnType::QueryReturnType( XLink base_xlink_ ) :
	de_info({ nullptr, {} }),
	base_xlink( base_xlink_ )
{
}
	
	
TeleportAgent::QueryReturnType::QueryReturnType( TreePtr<Node> induced_base_node, const set<XLink> &deps ) :
	de_info{ induced_base_node, deps },
	base_xlink()
{
}


TeleportAgent::QueryReturnType::QueryReturnType( TreePtr<Node> induced_base_node, const Dependencies &deps ) :
    QueryReturnType( induced_base_node, deps.GetAll() )
{
}   


bool TeleportAgent::QueryReturnType::IsValid() const
{
	return de_info.induced_base_node || base_xlink;
}


bool TeleportAgent::QueryReturnType::IsXTree() const
{
	return !!base_xlink;
}

	
bool TeleportAgent::QueryReturnType::IsInduced() const
{
	return !!de_info.induced_base_node;
}

	
DomainExtension::Extender::Info TeleportAgent::QueryReturnType::TryGetDEInfo() const
{
	return de_info;
}


XLink TeleportAgent::QueryReturnType::GetBaseXLink() const
{
	ASSERTS( IsValid() );
	ASSERTS( IsXTree() );
	return base_xlink;
}


//---------------------------------- TeleportAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> TeleportAgent::SymbolicNormalLinkedQueryPRed() const                                      
{             
    shared_ptr<PatternQuery> my_pq = GetPatternQuery();         
    PatternLink child_plink = OnlyElementOf( my_pq->GetNormalLinks() );
    
    SYM::Lazy<SYM::SymbolExpression> keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    SYM::Lazy<SYM::SymbolExpression> child_expr = MakeLazy<SymbolVariable>(child_plink);
    
    return MakeLazy<TeleportOperator>( this, keyer_expr ) == child_expr;
}                     


DomainExtension::Extender::Info TeleportAgent::GetDomainExtension( const XTreeDatabase *db, XLink stimulus_xlink ) const
{
	
	if( stimulus_xlink == XLink::MMAX_Link )
		return DomainExtension::Extender::Info(); // MMAX at base never expands domain because then, all child patterns are also MMAX
	if( !IsPreRestrictionMatch(stimulus_xlink) )
		return DomainExtension::Extender::Info(); // Failed pre-restriction so can't expand domain

	QueryReturnType tq_result;
	try
	{
		tq_result = RunTeleportQuery( db, stimulus_xlink );
	}
	catch( ::Mismatch & ) 
	{
		return DomainExtension::Extender::Info();
	}
	
	return tq_result.TryGetDEInfo();
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
    unique_ptr<SymbolicResult> stimulus_linkset = OnlyElementOf(move(op_results));
    if( !stimulus_linkset->IsDefinedAndUnique() )
        return make_unique<EmptyResult>();
    XLink stimulus_xlink = stimulus_linkset->GetOnlyXLink();
        
    // Apply the teleporting operation to the xlink. It may create new nodes
    // so it returns a TreePtr<Node> to avoid creating new xlink without base.
    // TODO can't we fish this out of the database? or do both and compare?
    QueryReturnType tq_result = agent->RunTeleportQuery( kit.x_tree_db, stimulus_xlink );

    // Teleporting operation can fail: if so call it a NaS
    if( tq_result.IsInduced() )
    {
	    // We are required to have already added the new node to the domain
		// during domain extension, so use the node to fetch the unique XLink
		XLink unique_xlink = kit.x_tree_db->GetDEChannel(agent)->GetUniqueDomainExtension(stimulus_xlink, tq_result.TryGetDEInfo().induced_base_node);
    
		// Form a symbol result to return.       
		return make_unique<UniqueResult>( unique_xlink );
	}
    else if( tq_result.IsXTree() ) // parent link was supplied
    {
	    // If we got an XLink, just return it, don't bother DomainExtension
        return make_unique<UniqueResult>( tq_result.GetBaseXLink() );
	}
	else // invalid
	{
        return make_unique<EmptyResult>();        
	}
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

	
