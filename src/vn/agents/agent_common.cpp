#include "agent_common.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"
#include "db/duplicate.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/rewriters.hpp"
#include "up/tree_update.hpp"

#include <stdexcept>

using namespace VN;
using namespace SYM;

//---------------------------------- AgentCommon ------------------------------------    

AgentCommon::AgentCommon()
{
}


void AgentCommon::SCRConfigure( Phase phase_ )
{
    phase = phase_;
    ASSERT( (int)phase != 0 );
}


list<PatternLink> AgentCommon::GetChildren() const
{
    // Normally all children should be visible 
    typedef ContainerFromIterator< FlattenNode_iterator, const Node * > FlattenNodePtr;
    auto con = make_shared<FlattenNodePtr>(this);
    // Note: a pattern query should be just as good...
    // Incorrect! This gets the replace-side stuff as well; GetPatternQuery()
    // is only for search.
    //FTRACE("\n");
    list<PatternLink> plinks;
    for( const TreePtrInterface &tpi : *con )
    {        
		//FTRACE("TPI=%p\n", &tpi);
        if( tpi )
            plinks.push_back( PatternLink(&tpi) );
    }
    return plinks;
}

    
list<PatternLink> AgentCommon::GetVisibleChildren( Path v ) const
{
	(void)v;
    return GetChildren();
}


shared_ptr<DecidedQuery> AgentCommon::CreateDecidedQuery() const
{    
    return make_shared<DecidedQuery>( GetPatternQuery() );
}
                                

Lazy<BooleanExpression> AgentCommon::SymbolicQuery( PatternLink keyer, const set<PatternLink> &residuals, bool coupling_only ) const 
{
    auto cq_expr = SymbolicCouplingQuery(keyer, residuals);
    if( coupling_only )
        return cq_expr;

    auto nlq_expr = SymbolicNormalLinkedQuery(keyer);
       return cq_expr & nlq_expr; 
}


Lazy<BooleanExpression> AgentCommon::SymbolicCouplingQuery(PatternLink keyer, const set<PatternLink> &residuals) const
{    
    // This class establishes the policy for couplings in one place.
    // And it always will be: see #121; para starting at "No!!"
	      
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer);    
    auto mmax_expr = MakeLazy<SymbolConstant>(XLink::MMAX);
    
    // Policy must apply for every residual
    auto expr = MakeLazy<BooleanConstant>(true);
    for( PatternLink residual_plink : residuals )    
    {
        auto residual_expr = MakeLazy<SymbolVariable>(residual_plink);
        
        // Policy: Accept SimpleCompare equivalence of current 
        // residual to keyer, or either one being MMAX
        expr &= MakeLazy<IsSimpleCompareEquivalentOperator>( keyer_expr, residual_expr ) |
                (keyer_expr == mmax_expr) | // See thought on #384
                (residual_expr == mmax_expr);
    }
    return expr;
}


bool AgentCommon::ShouldGenerateCategoryClause() const
{
	if( !GetArchetypeNode() )
		return false;
		
    // It could be argued that, from the CSP solver's point of
    // view, if we didn't need pre-restriction constraint, we would
    // still need a "type-correctness constraint", i.e. something
    // to stop the solver wasting time querying values that would lead
    // to non-type-safe trees. We don't get fails without it because
    // solver never finds them because they're not there due type-safety.
    // But if we're type Node, then the parent pointer(s) must also be
    // Node, and there's no need for any restriction.
    return typeid( *GetArchetypeNode() ) != typeid(Node);
    
    // Note about typeid(): if I go typeid( Node() ) I get the type of a 
    // Node constructor. 
}                                


SYM::Lazy<SYM::BooleanExpression> AgentCommon::SymbolicPreRestriction(PatternLink keyer_plink) const
{
    if( ShouldGenerateCategoryClause() )
    {
        auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
        return MakeLazy<IsInCategoryOperator>(GetArchetypeNode(), keyer_expr);
    }
    else
    {
        return MakeLazy<BooleanConstant>(true);
    }
}


bool AgentCommon::IsPreRestrictionMatch( TreePtr<Node> x ) const
{
    // Pre-restriction policy defined here. 
    return GetArchetypeNode()->IsSubcategory( *x );
}


bool AgentCommon::IsPreRestrictionMatch( XLink x ) const
{
    return IsPreRestrictionMatch( x.GetChildTreePtr() );
}


void AgentCommon::RunRegenerationQueryImpl( DecidedQueryAgentInterface &,
                                            const SolutionMap *,
                                            PatternLink,
                                            const XTreeDatabase * ) const
{
}
    
    
void AgentCommon::RunRegenerationQuery( DecidedQueryAgentInterface &query,
                                        const SolutionMap *hypothesis_links,
                                        PatternLink keyer_plink,
                                        const XTreeDatabase *x_tree_db ) const
{
    // Admin stuff every RQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    if( keyer_xlink != XLink::MMAX )
        this->RunRegenerationQueryImpl( query, hypothesis_links, keyer_plink, x_tree_db );
}                             
                      
                      
AgentCommon::QueryLambda AgentCommon::StartRegenerationQuery( const AndRuleEngine *acting_engine,
                                                              const SolutionMap *hypothesis_links,
															  PatternLink keyer_plink,
                                                              const XTreeDatabase *x_tree_db ) const
{
	shared_ptr<Conjecture> nlq_conjecture = acting_engine->GetNLQConjecture(this);
    ASSERT( nlq_conjecture )(phase);
    nlq_conjecture->Start();
    bool first = true;
    
    QueryLambda lambda = [=, this]()mutable->shared_ptr<DecidedQuery>
    { 
        shared_ptr<DecidedQuery> query;
        
        // The trouble with using a lambda is that you have to return a potential
        // hit to caller and then iterate _if_ you get called again. A coroutine
        // would make it possible to do this more cleanly. Related: #815
        if( !first )
        {
            TRACE("Trying conjecture increment\n");
            if( !nlq_conjecture->Increment() )
            {
				nlq_conjecture->Reset();
                throw NLQConjOutAfterHitMismatch(); // Conjecture has run out of choices to try.            
			}
        }

        while(1)
        {
            try
            {
                // Query the agent: our conj will be used for the iteration and
                // therefore our query will hold the result.
                query = nlq_conjecture->GetQuery(this);

                // which is held by nlq_conjecture. It is imperitive to call
                // nlq_conjecture->Reset() to destruct them before completing
                // the search so that final teardown doesn't orphan them. Note
                // this lambda is called repeatedley, see the `first` flag.
                // Related: #815
                RunRegenerationQuery( *query, hypothesis_links, keyer_plink, x_tree_db );       
                    
                TRACE("Got query from DNLQ ")(query->GetDecisions())("\n");
                    
                break; // Great, the normal links matched
            }
            catch( ::Mismatch &e ) 
            {
                // We will get here on a mismatch, whether detected by DQ or our
                // own comparison of the normal links. Permit the conjecture
                // to move to a new set of choices.
                if( !nlq_conjecture->Increment() )
                {
                    nlq_conjecture->Reset();
                    throw; // Conjecture has run out of choices to try.
                }
                // Conjecture would like us to try again with new choices
            }
        }     
        first = false;
        
        return query;
    };
    return lambda;
}   
                                              
                                              
AgentCommon::QueryLambda AgentCommon::TestStartRegenerationQuery( const AndRuleEngine *acting_engine,
                                                                  const SolutionMap *hypothesis_links,
                                                                  PatternLink keyer_plink,  
                                                                  const XTreeDatabase *x_tree_db ) const
{
	shared_ptr<PatternQuery> pq = GetPatternQuery();
    QueryLambda mut_lambda;
    QueryLambda ref_lambda;
    auto mut_hits = make_shared<int>(0);
    auto ref_hits = make_shared<int>(0);
    try
    {
        mut_lambda = StartRegenerationQuery( acting_engine, hypothesis_links, keyer_plink, x_tree_db );
    }
    catch( ::Mismatch &e ) 
    {
        try
        {
            Tracer::RAIIDisable silencer; // make ref algo be quiet            
            (void)StartRegenerationQuery( acting_engine, hypothesis_links, keyer_plink, x_tree_db );
            ASSERT(false)("MUT start threw ")(e)(" but ref didn't\n")
                         (*this)("\n")
                         ("Normal: ")(MapForPattern(pq->GetNormalLinks(), *hypothesis_links))("\n")
                         ("Abormal: ")(MapForPattern(pq->GetAbnormalLinks(), *hypothesis_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pq->GetMultiplicityLinks(), *hypothesis_links))("\n");
        }
        catch( ::Mismatch &e ) 
        {
            // Passed test: both threw Mismatch
            throw;
        }        
    }
    // FastStartNormalLinkedQuery() didn't throw
    try
    {
        Tracer::RAIIDisable silencer; // make ref algo be quiet             
        ref_lambda = StartRegenerationQuery( acting_engine, hypothesis_links, keyer_plink, x_tree_db );        
    }
    catch( ::Mismatch &e ) 
    {
        ASSERT(false)("Ref start threw ")(e)(" but MUT didn't\n")
                     (*this)("\n")
                     ("Normal: ")(MapForPattern(pq->GetNormalLinks(), *hypothesis_links))("\n")
                     ("Abormal: ")(MapForPattern(pq->GetAbnormalLinks(), *hypothesis_links))("\n")
                     ("Multiplicity: ")(MapForPattern(pq->GetMultiplicityLinks(), *hypothesis_links))("\n");
    }

    QueryLambda test_lambda = [=, this]()mutable->shared_ptr<DecidedQuery>
    {
        shared_ptr<VN::DecidedQuery> mut_query;
        shared_ptr<VN::DecidedQuery> ref_query;
        try 
        { 
            mut_query = mut_lambda(); 
            TRACE("MUT lambda hit #%d\n", (*mut_hits))
                 (*mut_query)("\n");

            (*mut_hits)++;
        }
        catch( ::Mismatch &e ) 
        {
            try 
            { 
                {
                    Tracer::RAIIDisable silencer; // make ref algo be quiet            
                    ref_query = ref_lambda(); 
                }
                TRACE("Ref lambda hit #%d\n", (*ref_hits))
                     (*ref_query)("\n");
                (*ref_hits)++;                
                ASSERT(false)("MUT lambda threw ")(e)(" but ref didn't\n")
                             ("MUT hits %d, ref hits %d\n", *mut_hits, *ref_hits)
                             (*this)("\n")
                             ("Normal: ")(MapForPattern(pq->GetNormalLinks(), *hypothesis_links))("\n")
                             ("Abormal: ")(MapForPattern(pq->GetAbnormalLinks(), *hypothesis_links))("\n")
                             ("Multiplicity: ")(MapForPattern(pq->GetMultiplicityLinks(), *hypothesis_links))("\n");
            }
            catch( ::Mismatch &e ) 
            {
                throw;
            }                
        }
        // Didn't throw
        try
        {
            {
                Tracer::RAIIDisable silencer; // make ref algo be quiet             
                ref_query = ref_lambda(); 
            }
            TRACE("Ref lambda hit #%d\n", (*ref_hits))
                 (*ref_query)("\n");

            (*ref_hits)++;                
        }
        catch( ::Mismatch &e ) 
        {
            ASSERT(false)("Ref lambda threw ")(e)(" but MUT didn't\n")
                         ("MUT hits %d, ref hits %d\n", *mut_hits, *ref_hits)
                         (*this)("\n")
                         ("Normal: ")(MapForPattern(pq->GetNormalLinks(), *hypothesis_links))("\n")
                         ("Abormal: ")(MapForPattern(pq->GetAbnormalLinks(), *hypothesis_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pq->GetMultiplicityLinks(), *hypothesis_links))("\n");
        }
            
        
        // Don't check normal links - NLQ implementations are not required to register them
        //DecidedQueryCommon::AssertMatchingLinks( mut_query->GetNormalLinks(), ref_query->GetNormalLinks() );

        // Now to check the links the two algos put in their query objects
        //DecidedQueryCommon::AssertMatchingNodes( mut_query->GetAbnormalNodes(), ref_query->GetAbnormalNodes() ); TODO this is a check on agents; bring back
        DecidedQueryCommon::AssertMatchingNodes( mut_query->GetMultiplicityNodes(), ref_query->GetMultiplicityNodes() );
        return mut_query;
    };
    
    return test_lambda;
}



void AgentCommon::Reset()
{
}


void AgentCommon::PlanOverlay( SCREngine *acting_engine,
                               PatternLink me_plink, 
                               PatternLink bottom_layer_plink )
{
    ASSERT( me_plink.GetChildAgent() == this );
    // This function is called on nodes in the "overlay" branch of Delta nodes.
    // Some special nodes will not know what to do...
      
    if( acting_engine->IsKeyedByAndRuleEngine(this) ) 
        return; // In search pattern and already keyed - we only overlay using replace-only nodes
                
    // This is why we call on over, passing in under. The test requires
    // that under be a non-strict subclass of over. Overlaying a super-category
    // over a subcategory means we simply update the singulars we know about
    // in over. Under is likely to be an X node and hence final, while
    // over can be StandaedAgent<some intermediate>.
    if( !IsSubcategory(*bottom_layer_plink.GetChildAgent()) ) 
        return; // Not compatible with pattern: recursion stops here
        
    // Under must be a standard agent
    if( !dynamic_cast<StandardAgent *>(bottom_layer_plink.GetChildAgent()) )
        return;
        
    // Remember the overlaying action that will be required
    acting_engine->SetOverlayBottomLayer( this, bottom_layer_plink );

    // Agent-specific actions
    MaybeChildrenPlanOverlay( acting_engine, me_plink, bottom_layer_plink );
}


void AgentCommon::MaybeChildrenPlanOverlay( SCREngine *,
                                            PatternLink, 
                                            PatternLink )
{
    // An empty function here implies leaf-termination of the overlay plan
}
                                  
                                  
TreePtr<Node> AgentCommon::BuildForBuildersAnalysis( PatternLink me_plink, const SCREngine *acting_engine )
{
    Agent::ReplaceKit kit { nullptr, nullptr };
    shared_ptr<Patch> layout = GenReplaceLayout(kit, me_plink, acting_engine);
    unique_ptr<FreeZone> zone = TreeUpdater::TransformToSingleFreeZone( layout );     
    return zone->GetBaseNode();
}


Agent::ReplacePatchPtr AgentCommon::GenReplaceLayout( const ReplaceKit &kit, 
                                                      PatternLink me_plink,
                                                      const SCREngine *acting_engine)
{
    INDENT("C");
    ASSERT( me_plink.GetChildAgent() == this );
    ASSERTTHIS();
    ASSERT( phase != IN_COMPARE_ONLY )(*this)(" is configured for compare only");  
    
    XLink key_xlink;
    ASSERT( acting_engine );
    if( acting_engine->IsKeyedBeforeReplace( this ) )
    {
        key_xlink = acting_engine->GetKey( this );
        ASSERT( key_xlink.GetChildTreePtr()->IsFinal() )
              (*this)(" keyed with non-final node ")(key_xlink)("\n"); 
    }

    ReplacePatchPtr patch = GenReplaceLayoutImpl( kit, me_plink, key_xlink, acting_engine );
      
    // Inform the update mechanism that, once it's done duplicating 
    // nodes etc, it should mark this position for this agent's origin.
    patch->AddOriginators( { me_plink } );
    
    return patch;
}


TreePtr<Node> AgentCommon::GetEmbeddedSearchPattern() const
{
	return nullptr;
}


TreePtr<Node> AgentCommon::GetEmbeddedReplacePattern() const
{
	return nullptr;
}


Agent::ReplacePatchPtr AgentCommon::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                          PatternLink me_plink, 
                                                          XLink key_xlink,
                                                          const SCREngine *acting_engine )
{
	(void)kit;
	(void)me_plink;
	(void)acting_engine;
	
    // Default replace behaviour to just use the X subtree we keyed to, so we need to be keyed
    ASSERT(key_xlink)("Agent ")(*this)(" in replace context is not keyed but needs to be");
    auto new_zone = TreeZone::CreateSubtree(key_xlink);
    return make_shared<TreePatch>( new_zone );        
} 


TreePtr<Node> AgentCommon::CloneNode() const
{
    INDENT("D");

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = Clone();
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
            
    return dest;
}


bool AgentCommon::IsNonTrivialPreRestriction(const TreePtrInterface *pptr) const
{
	if( IsFixedType() )
		return false;
		
	if( !GetArchetypeTreePtr() )
		return false;
		
    // Note: we are using typeid on the tree pointer type, not the node type.
    // So we need an archetype tree pointer.
    return typeid( *pptr ) != typeid( *GetArchetypeTreePtr() );
}                                


bool AgentCommon::IsFixedType() const
{
	return false; // "most" agents are templated on a node type
}


string AgentCommon::GetGraphId() const
{
    return GetSerialString();
}


string AgentCommon::GetPlanAsString() const
{
    list<KeyValuePair> plan_as_strings = 
    {
    };
    return Trace(plan_as_strings);
}


string AgentCommon::GetTrace() const
{
    return Traceable::GetName() + GetSerialString();
}
