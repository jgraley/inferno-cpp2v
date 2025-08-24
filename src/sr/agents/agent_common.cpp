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

using namespace SR;
using namespace SYM;

//---------------------------------- AgentCommon ------------------------------------    

AgentCommon::AgentCommon() :
    my_scr_engine(nullptr)
{
}


void AgentCommon::SCRConfigure( const SCREngine *e,
                                Phase phase_ )
{
    ASSERT(e);
    // Repeat configuration regarded as an error because it suggests I maybe don't
    // have a clue what should actaually be configing the agent. Plus general lifecycle 
    // rule enforcement.
    // Why no coupling across sibling embedded engines? Would need an ordering for keying
    // but ordering not defined on sibling embedded engines.
    // Stronger reason: siblings each hit multiple times with respect to parent
    // and not necessarily the same number of times, so there's no single
    // well defined key.
    ASSERT(!my_scr_engine)
          ("Detected repeat configuration of ")
          (*this)
          ("\nCould be result of coupling this node across sibling embedded engines - not allowed :(");
    my_scr_engine = e;
    
	pattern_query = GetPatternQuery();    
        
    phase = phase_;
    ASSERT( (int)phase != 0 );
    if( phase != IN_REPLACE_ONLY )
    {
        // We will need a conjecture, so that we can iterate through multiple 
        // potentially valid values for the abnormals and multiplicities.
        nlq_conjecture = make_shared<Conjecture>(this);            
    }
}


void AgentCommon::ConfigureCoupling( const Traceable *e,
                                     PatternLink keyer_plink_, 
                                     set<PatternLink> residual_plinks_ )
{  
    ASSERT(e);
    // Enforcing rule #149 - breaking that rule will cause the same base node to appear in
    // more than one subordinate and-rule engine, so that it will get configured more than once.
    // Also see #316
    ASSERT(!my_keyer_engine)("Detected repeat coupling configuration of ")(*this)
                                   ("\nCould be result of coupling abnormal links - not allowed :(\n")
                                   (my_keyer_engine)(" with keyer ")(keyer_plink_)("\n");                
    ASSERT(my_scr_engine)("Must call SCRConfigure() before ConfigureCoupling()");
    my_keyer_engine = e;
                                           
    if( keyer_plink_ )
    {
        ASSERT( keyer_plink_.GetChildAgent() == this )("Parent link supplied for different agent");
        //keyer_plink = keyer_plink_;
    }   
}
                                

void AgentCommon::AddResiduals( set<PatternLink> residual_plinks_ )
{

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
    return GetChildren();
}


shared_ptr<DecidedQuery> AgentCommon::CreateDecidedQuery() const
{
    ASSERT( my_scr_engine ); // check we have been configured
    
    return make_shared<DecidedQuery>( pattern_query );
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
    ASSERT( my_keyer_engine )(*this)(" has not been configured for couplings");
    
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


bool AgentCommon::IsNonTrivialPreRestriction(const TreePtrInterface *pptr) const
{
    // Note: we are using typeid on the tree pointer type, not the node type.
    // So we need an archetype tree pointer.
    return typeid( *pptr ) != typeid( *GetArchetypeTreePtr() );
}                                


bool AgentCommon::ShouldGenerateCategoryClause() const
{
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
    // node constructor. 
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
    return IsSubcategory( *x );
}


bool AgentCommon::IsPreRestrictionMatch( XLink x ) const
{
    return IsPreRestrictionMatch( x.GetChildTreePtr() );
}


void AgentCommon::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                            const SolutionMap *hypothesis_links,
                                            PatternLink keyer_plink,
                                            const XTreeDatabase *x_tree_db ) const
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
                      
                      
AgentCommon::QueryLambda AgentCommon::StartRegenerationQuery( const SolutionMap *hypothesis_links,
															  PatternLink keyer_plink,
                                                              const XTreeDatabase *x_tree_db,
                                                              bool use_DQ ) const
{
    ASSERT( nlq_conjecture )(phase);
    nlq_conjecture->Start();
    bool first = true;
    
    QueryLambda lambda = [=]()mutable->shared_ptr<DecidedQuery>
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
                                              
                                              
AgentCommon::QueryLambda AgentCommon::TestStartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                                  PatternLink keyer_plink,  
                                                                  const XTreeDatabase *x_tree_db ) const
{
    QueryLambda mut_lambda;
    QueryLambda ref_lambda;
    auto mut_hits = make_shared<int>(0);
    auto ref_hits = make_shared<int>(0);
    try
    {
        mut_lambda = StartRegenerationQuery( hypothesis_links, keyer_plink, x_tree_db, false );
    }
    catch( ::Mismatch &e ) 
    {
        try
        {
            Tracer::RAIIDisable silencer; // make ref algo be quiet            
            (void)StartRegenerationQuery( hypothesis_links, keyer_plink, x_tree_db, true );
            ASSERT(false)("MUT start threw ")(e)(" but ref didn't\n")
                         (*this)("\n")
                         ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *hypothesis_links))("\n")
                         ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *hypothesis_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *hypothesis_links))("\n");
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
        ref_lambda = StartRegenerationQuery( hypothesis_links, keyer_plink, x_tree_db, true );        
    }
    catch( ::Mismatch &e ) 
    {
        ASSERT(false)("Ref start threw ")(e)(" but MUT didn't\n")
                     (*this)("\n")
                     ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *hypothesis_links))("\n")
                     ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *hypothesis_links))("\n")
                     ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *hypothesis_links))("\n");
    }

    QueryLambda test_lambda = [=]()mutable->shared_ptr<DecidedQuery>
    {
        shared_ptr<SR::DecidedQuery> mut_query;
        shared_ptr<SR::DecidedQuery> ref_query;
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
                             ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *hypothesis_links))("\n")
                             ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *hypothesis_links))("\n")
                             ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *hypothesis_links))("\n");
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
                         ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *hypothesis_links))("\n")
                         ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *hypothesis_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *hypothesis_links))("\n");
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


void AgentCommon::ResetNLQConjecture()
{
    // Theoretically this should be done inside the lambda's destructor, but can't put code there
    nlq_conjecture->Reset();
}


const SCREngine *AgentCommon::GetMasterSCREngine() const
{
    return my_scr_engine;
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
      
    if( my_scr_engine->IsKeyedByAndRuleEngine(this) ) 
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


void AgentCommon::MaybeChildrenPlanOverlay( SCREngine *acting_engine,
                                            PatternLink me_plink, 
                                            PatternLink bottom_layer_plink )
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
    ASSERT(my_scr_engine)("Agent ")(*this)(" appears not to have been configured");
    ASSERT( phase != IN_COMPARE_ONLY )(*this)(" is configured for compare only");  
    
    XLink key_xlink;
    //FTRACE(my_scr_engine)("\n");
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


string AgentCommon::GetTrace() const
{
    return Traceable::GetName() + GetSerialString();
}


string AgentCommon::GetGraphId() const
{
    return GetSerialString();
}


string AgentCommon::GetPlanAsString() const
{
    list<KeyValuePair> plan_as_strings = 
    {
        { "my_scr_engine", 
          Trace(my_scr_engine) },
        { "my_keyer_engine", 
          Trace(my_keyer_engine) }
    };
    return Trace(plan_as_strings);
}


