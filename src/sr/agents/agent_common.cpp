#include "agent_common.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/comparison_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/rewriters.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- AgentCommon ------------------------------------    

AgentCommon::AgentCommon() :
    master_scr_engine(nullptr)
{
}


void AgentCommon::SCRConfigure( const SCREngine *e,
                                Phase phase_ )
{
    ASSERT(e);
    // Repeat configuration regarded as an error because it suggests I maybe don't
    // have a clue what should actaually be configing the agent. Plus general lifecycle 
    // rule enforcement.
    // Why no coupling across sibling slaves? Would need an ordering for keying
    // but ordering not defined on sibling slaves.
    // Stronger reason: siblings each hit multiple times with respect to parent
    // and not necessarily the same number of times, so there's no single
    // well defined key.
    ASSERT(!master_scr_engine)("Detected repeat configuration of ")(*this)
                              ("\nCould be result of coupling this node across sibling slaves - not allowed :(");
    master_scr_engine = e;
    phase = phase_;

	ASSERT( (int)phase != 0 );
    
    if( phase != IN_REPLACE_ONLY )
    {
        pattern_query = GetPatternQuery();
        num_decisions = pattern_query->GetDecisions().size();

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
    // Enforcing rule #149 - breaking that rule will cause the same root node to appear in
    // more than one subordinate and-rule engine, so that it will get configured more than once.
    // Also see #316
    ASSERT(!coupling_master_engine)("Detected repeat coupling configuration of ")(*this)
                                   ("\nCould be result of coupling abnormal links - not allowed :(\n")
                                   (coupling_master_engine)(" with keyer ")(keyer_plink)("\n")
                                   (e)(" with keyer ")(keyer_plink_);                                   
    ASSERT(master_scr_engine)("Must call SCRConfigure() before ConfigureCoupling()");
    coupling_master_engine = e;
                                           
    if( keyer_plink_ )
    {
        ASSERT( keyer_plink_.GetChildAgent() == this )("Parent link supplied for different agent");
        keyer_plink = keyer_plink_;
    }
    
    for( PatternLink plink : residual_plinks_ )
    {
        ASSERT( plink );
        ASSERT( plink.GetChildAgent() == this )("Parent link supplied for different agent");
        InsertSolo( residual_plinks, plink );
    }

    // It works with a set, but if we lose this ordering hints from 
    // Colocated agent NLQs become less useful and CSP solver slows right
    // down.
    if( phase != IN_REPLACE_ONLY && keyer_plink )
    {
        keyer_and_normal_plinks.clear();
        keyer_and_normal_plinks.push_back( keyer_plink );
        for( PatternLink plink : pattern_query->GetNormalLinks() )
            keyer_and_normal_plinks.push_back( plink );
    }
}
                                

void AgentCommon::AddResiduals( set<PatternLink> residual_plinks_ )
{
    for( PatternLink plink : residual_plinks_ )
    {
        ASSERT( plink );
        ASSERT( plink.GetChildAgent() == this )("Parent link supplied for different agent");
        InsertSolo( residual_plinks, plink );
    }
}


list<PatternLink> AgentCommon::GetChildren() const
{
	// Normally all children should be visible 
    typedef ContainerFromIterator< FlattenNode_iterator, const Node * > FlattenNodePtr;
    auto con = shared_ptr<ContainerInterface>( new FlattenNodePtr(this) );
    // Note: a pattern query should be just as good...
    // Incorrect! This gets the replace-side stuff as well; GetPatternQuery()
    // is only for search.
    
    list<PatternLink> plinks;
    FOREACH( const TreePtrInterface &tpi, *con )
    {        
        if( tpi )
            plinks.push_back( PatternLink(this, &tpi) );
    }
    return plinks;
}

    
list<PatternLink> AgentCommon::GetVisibleChildren( Path v ) const
{
    return GetChildren();
}


shared_ptr<DecidedQuery> AgentCommon::CreateDecidedQuery() const
{
    ASSERT( master_scr_engine ); // check we have been configured
    
    return make_shared<DecidedQuery>( pattern_query );
}
    
    
void AgentCommon::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                       XLink keyer_xlink ) const
{
    ASSERTFAIL();
}                                       


void AgentCommon::RunDecidedQuery( DecidedQueryAgentInterface &query,
                                   XLink keyer_xlink ) const
{
    // Admin stuff every DQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;   
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    RunDecidedQueryImpl( query, keyer_xlink );
}                             


bool AgentCommon::ImplHasNLQ() const
{    
    return false;
}


void AgentCommon::RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                            const TheKnowledge *knowledge ) const
{
    ASSERTFAIL();
}
    
    
void AgentCommon::NLQFromDQ( const SolutionMap *hypothesis_links,
                             const TheKnowledge *knowledge ) const
{    
    TRACE("common DNLQ: ")(*this)(" at ")(keyer_plink)("\n");
    
    
    // Can't do baseless query using DQ
    if( hypothesis_links->count(keyer_plink)==0 )
        return;
    
    auto query = CreateDecidedQuery();
    RunDecidedQueryImpl( *query, hypothesis_links->at(keyer_plink) );
    
    // The query now has populated links, which should be full
    // (otherwise RunDecidedQuery() should have thrown). We loop 
    // over both and check that they refer to the same x nodes
    // we were passed. Mismatch will throw, same as in DQ.
    auto actual_links = query->GetNormalLinks();
    TRACE("Actual   ")(actual_links)("\n");
    ASSERT( actual_links.size() == pattern_query->GetNormalLinks().size() );
    
    for( LocatedLink alink : actual_links )
    {
        auto plink = (PatternLink)alink;
        if(hypothesis_links->count(plink)==0)
            continue; // partial query support
            
        LocatedLink rlink( plink, hypothesis_links->at(plink) );
        ASSERT( alink.GetChildAgent() == rlink.GetChildAgent() );                
        if( (XLink)alink == XLink::MMAX_Link )
            continue; // only happens when agent pushes out MMAX, as with DisjunctionAgent
            
        // Compare by location
        if( (XLink)alink != (XLink)rlink ) 
            throw NLQFromDQLinkMismatch();               
    }            
}                           
                                
    
void AgentCommon::RunCouplingQuery( const SolutionMap *hypothesis_links ) const
{    
    // This function establishes the policy for couplings in one place.
    // Today, it's SimpleCompare, via EquivalenceRelation, with MMAX excused. 
    // And it always will be: see #121; para starting at "No!!"
    // HOWEVER: it is now possible for agents to override this policy.
    
    ASSERT( hypothesis_links );
    // Without keyer, don't bother to check anything
    if( hypothesis_links->count(keyer_plink) == 0 )
        return;
    XLink keyer = hypothesis_links->at(keyer_plink);
    
    for( PatternLink residual_plink : residual_plinks )
    {
        if( hypothesis_links->count(residual_plink) )
        {
            XLink residual = hypothesis_links->at(residual_plink);
            if( residual == XLink::MMAX_Link || keyer == XLink::MMAX_Link)
                continue; 
                    
            CompareResult cr = equivalence_relation.Compare( keyer, 
                                                             residual );
            if( cr != EQUAL )
                throw CouplingMismatch();               
        }
    }     
}


Over<BooleanExpression> AgentCommon::SymbolicQuery( bool coupling_only ) const
{
	auto cq_lazy = SymbolicCouplingQuery();
    if( coupling_only )
        return cq_lazy;

    auto nlq_lazy = SymbolicNormalLinkedQuery();
   	return cq_lazy & nlq_lazy; // Over-style symbolic expression
}


Over<BooleanExpression> AgentCommon::SymbolicNormalLinkedQuery() const
{
    if( ImplHasNLQ() )    
        return SymbolicNormalLinkedQueryImpl();

	// The keyer and normal children
	set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
	auto nlq_lambda = [this](const Expression::EvalKit &kit)
	{
		NLQFromDQ( kit.hypothesis_links,
                   kit.knowledge ); // throws on mismatch   
	};
	return MakeOver<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQFromDQ()");	             
}


Over<BooleanExpression> AgentCommon::SymbolicNormalLinkedQueryImpl() const
{
	// The keyer and normal children
	set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
	auto nlq_lambda = [this](const Expression::EvalKit &kit)
	{
		RunNormalLinkedQueryImpl( kit.hypothesis_links,
							      kit.knowledge ); // throws on mismatch   
	};
	return MakeOver<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQImpl()");	
}


Over<BooleanExpression> AgentCommon::SymbolicCouplingQuery() const
{
    ASSERT( coupling_master_engine )(*this)(" has not been configured for couplings");
	
    auto expr = MakeOver<BooleanConstant>(true);
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    auto mmax_expr = MakeOver<SymbolConstant>(SR::XLink::MMAX_Link);
    for( PatternLink residual_plink : residual_plinks )
    {
        auto residual_expr = MakeOver<SymbolVariable>(residual_plink);
        expr &= ( MakeOver<EquivalentOperator>( list< shared_ptr<SymbolExpression> >({keyer_expr, residual_expr}) ) |
                  keyer_expr == mmax_expr | // See thought on #384
                  residual_expr == mmax_expr );
    }
    return expr;
}


SYM::Over<SYM::BooleanExpression> AgentCommon::SymbolicPreRestriction() const
{
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
	return MakeOver<KindOfOperator>(this, keyer_expr);
}


void AgentCommon::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                            const SolutionMap *hypothesis_links,
                                            const TheKnowledge *knowledge ) const
{
}
    
    
void AgentCommon::RunRegenerationQuery( DecidedQueryAgentInterface &query,
                                        const SolutionMap *hypothesis_links,
                                        const TheKnowledge *knowledge ) const
{
    // Admin stuff every RQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    if( keyer_xlink != XLink::MMAX_Link )
        this->RunRegenerationQueryImpl( query, hypothesis_links, knowledge );
}                             
                      
                      
AgentCommon::QueryLambda AgentCommon::StartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                              const TheKnowledge *knowledge,
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
        // would make it possible to do this more cleanly.
        if( !first )
        {
            TRACE("Trying conjecture increment\n");
            if( !nlq_conjecture->Increment() )
                throw NLQConjOutAfterHitMismatch(); // Conjecture has run out of choices to try.            
        }

        while(1)
        {
            try
            {
                // Query the agent: our conj will be used for the iteration and
                // therefore our query will hold the result 
                query = nlq_conjecture->GetQuery(this);
                RunRegenerationQuery( *query, hypothesis_links, knowledge );       
                    
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
                                                                  const TheKnowledge *knowledge ) const
{
    QueryLambda mut_lambda;
    QueryLambda ref_lambda;
    auto mut_hits = make_shared<int>(0);
    auto ref_hits = make_shared<int>(0);
    try
    {
        mut_lambda = StartRegenerationQuery( hypothesis_links, knowledge, false );
    }
    catch( ::Mismatch &e ) 
    {
        try
        {
            Tracer::RAIIDisable silencer; // make ref algo be quiet            
            (void)StartRegenerationQuery( hypothesis_links, knowledge, true );
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
        ref_lambda = StartRegenerationQuery( hypothesis_links, knowledge, true );        
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
        DecidedQueryCommon::AssertMatchingLinks( mut_query->GetAbnormalLinks(), ref_query->GetAbnormalLinks() );
        DecidedQueryCommon::AssertMatchingLinks( mut_query->GetMultiplicityLinks(), ref_query->GetMultiplicityLinks() );
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
    return master_scr_engine;
}      


PatternLink AgentCommon::GetKeyerPatternLink() const
{
    ASSERT( coupling_master_engine )(*this)(" has not been configured for couplings");
    ASSERT( keyer_plink )(*this)(" has no keyer_plink, engine=")(coupling_master_engine)("\n");
    
    return keyer_plink;
}


set<PatternLink> AgentCommon::GetResidualPatternLinks() const
{
    ASSERT( coupling_master_engine )(*this)(" has not been configured for couplings");
    
    return residual_plinks;
}                                  


void AgentCommon::Reset()
{
}


void AgentCommon::PlanOverlay( PatternLink me_plink, 
                               PatternLink under_plink )
{
    ASSERT( me_plink.GetChildAgent() == this );
    // This function is called on nodes in the "overlay" branch of Delta nodes.
    // Some special nodes will not know what to do...
      
    if( master_scr_engine->IsKeyedByAndRuleEngine(this) ) 
        return; // In search pattern and already keyed - we only overlay using replace-only nodes
                
    // This is why we call on over, passing in under. The test requires
    // that under be a non-strict subclass of over. Overlaying a super-class
    // over a subclass means we simply update the singulars we know about
    // in over. Under is likely to be an X node and hence final while
    // over can be StandaedAgent<some intermediate>.
    if( !IsLocalMatch(under_plink.GetChildAgent()) ) 
        return; // Not compatible with pattern: recursion stops here
        
    // Under must be a standard agent
    if( !dynamic_cast<StandardAgent *>(under_plink.GetChildAgent()) )
        return;
        
    // Remember the overlaying action that will be required
    overlay_under_plink = under_plink;

    // Agent-specific actions
    PlanOverlayImpl( me_plink, under_plink );
}


void AgentCommon::PlanOverlayImpl( PatternLink me_plink, 
                                   PatternLink under_plink )
{
    // An empty function here implies leaf-termination of the overlay process
}


bool AgentCommon::ReplaceKeyerQuery( PatternLink me_plink, 
                                     set<PatternLink> keyer_plinks )
{
    return false;
}
                                  

TreePtr<Node> AgentCommon::BuildReplace( PatternLink me_plink )
{
    INDENT("B");
    ASSERT( me_plink.GetChildAgent() == this );

    ASSERT(this);
    ASSERT(master_scr_engine)("Agent ")(*this)(" appears not to have been configured");
    ASSERT( phase != IN_COMPARE_ONLY )(*this)(" is configured for compare only");

    TreePtr<Node> keynode = master_scr_engine->GetReplaceKey( keyer_plink );
    ASSERT( !keynode || keynode->IsFinal() )(*this)(" keyed with non-final node ")(keynode)("\n"); 
    
    TreePtr<Node> dest = BuildReplaceImpl(me_plink, keynode);
   
    ASSERT( dest );
    ASSERT( dest->IsFinal() )(*this)(" built non-final ")(*dest)("\n"); 
    
    return dest;
}


TreePtr<Node> AgentCommon::BuildReplaceImpl( PatternLink me_plink, 
                                             TreePtr<Node> key_node )
{
    ASSERT(key_node)("Unkeyed search-only agent seen in replace context");
    return DuplicateSubtree(key_node);   
}


TreePtr<Node> AgentCommon::DuplicateNode( TreePtr<Node> source,
                                          bool force_dirty ) const
{
    INDENT("D");

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
    ASSERT(dest);

    bool source_dirty = master_scr_engine->GetOverallMaster()->dirty_grass.find( source ) != master_scr_engine->GetOverallMaster()->dirty_grass.end();
    if( force_dirty || // requested by caller
        source_dirty ) // source was dirty
    {
        //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
        master_scr_engine->GetOverallMaster()->dirty_grass.insert( dest );
    }
    
    return dest;    
}                                                     


TreePtr<Node> AgentCommon::DuplicateSubtree( TreePtr<Node> source,
                                             TreePtr<Node> source_terminus,
                                             TreePtr<Node> dest_terminus ) const
{
    ASSERT( source );
    if( source_terminus )
        ASSERT( dest_terminus );

     // Under substitution, we should be duplicating a subtree of the input
    // program, which should not contain any special nodes
    ASSERT( !(dynamic_pointer_cast<SpecialBase>(source)) )
          ("Cannot duplicate special node ")(*source);
    
    // If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copies of source terminus (directly, without duplicating).
    if( source_terminus && source == source_terminus ) 
    {
        TRACE("Reached source terminus ")(source_terminus)
             (" and substituting ")(dest_terminus)("\n");
        return dest_terminus;
    }

    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( source, false );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > keynode_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Duplicating %d members source=", dest_memb.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        //TRACE("Duplicating member %d\n", i );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);

            dest_con->clear();

            //TRACE("Duplicating container size %d\n", keynode_con->size() );
            FOREACH( const TreePtrInterface &p, *keynode_con )
            {
                ASSERT( p ); // present simplified scheme disallows nullptr
                //TRACE("Duplicating ")(*p)("\n");
                TreePtr<Node> n = DuplicateSubtree( (TreePtr<Node>)p, source_terminus, dest_terminus );
                //TRACE("inserting ")(*n)(" directly\n");
                dest_con->insert( n );
            }
        }            
        else if( TreePtrInterface *keynode_singular = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_singular )("source should be non-nullptr");
            *dest_singular = DuplicateSubtree( (TreePtr<Node>)*keynode_singular, source_terminus, dest_terminus );
            ASSERT( *dest_singular );
            ASSERT( TreePtr<Node>(*dest_singular)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
}


bool AgentCommon::IsPreRestrictionMatch( XLink x ) const
{
    // Pre-restriction policy defined here. Would like to be able to 
    // export an archetype and implement the policy centrally or in
    // the symbolic stuff.
    return IsLocalMatch( x.GetChildX().get() );
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
        { "master_scr_engine", 
          Trace(master_scr_engine) },
        { "coupling_master_engine", 
          Trace(coupling_master_engine) },
        //{ "pattern_query", 
        //  Trace(pattern_query) }, // TODO should be traceable?
        { "keyer_plink", 
          Trace(keyer_plink) },
        { "residual_plinks", 
          Trace(residual_plinks) },
        { "overlay_under_plink", 
          Trace(overlay_under_plink) },
        { "keyer_and_normal_plinks", 
          Trace(keyer_and_normal_plinks) },
        { "SymbolicCouplingQuery()", 
          coupling_master_engine ? Trace(SymbolicCouplingQuery()) : string("<not configured for couplings>") },
        { "SymbolicNormalLinkedQuery()", 
          pattern_query ? Trace(SymbolicNormalLinkedQuery()) : string("<no pattern_query>") }
    };
    return Trace(plan_as_strings);
}