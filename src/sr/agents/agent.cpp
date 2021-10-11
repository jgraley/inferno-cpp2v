#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agent.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"
#include "coupling.hpp"
// Temporary
#include "tree/cpptree.hpp"
#include "transform_of_agent.hpp"

#include <stdexcept>

using namespace SR;

//---------------------------------- Agent ------------------------------------    

// C++11 fix
Agent& Agent::operator=(Agent& other)
{
    (void)Node::operator=(other);
    return *this;
}


Agent *Agent::AsAgent( shared_ptr<Node> node )
{
    Agent *agent = TryAsAgent(node);
    ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


Agent *Agent::TryAsAgent( shared_ptr<Node> node )
{
    ASSERT( node )("Called TryAsAgent(")(node)(") with null shared_ptr");
    return dynamic_cast<Agent *>(node.get());
}


const Agent *Agent::AsAgentConst( shared_ptr<const Node> node )
{
    const Agent *agent = TryAsAgentConst(node);
    ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


const Agent *Agent::TryAsAgentConst( shared_ptr<const Node> node )
{
    ASSERT( node )("Called TryAsAgent(")(node)(") with null shared_ptr");
    return dynamic_cast<const Agent *>(node.get());
}


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


void AgentCommon::AndRuleConfigure( const AndRuleEngine *e,
                                    PatternLink base_plink_, 
                                    set<PatternLink> coupled_plinks_ )
{  
    ASSERT(e);
    // Enforcing rule #149 - breaking that rule will cause the same root node to appear in
    // more than one subordinate and-rule engine, so that it will get configured more than once.
    // Also see #316
    ASSERT(!master_and_rule_engine)("Detected repeat configuration of ")(*this)
                                   ("\nCould be result of coupling abnormal links - not allowed :(");
    ASSERT(master_scr_engine)("Must call SCRConfigure() before AndRuleConfigure()");
    master_and_rule_engine = e;
                                           
    if( base_plink_ )
    {
        ASSERT( base_plink_.GetChildAgent() == this )("Parent link supplied for different agent");
        base_plink = base_plink_;
    }
    
    for( PatternLink plink : coupled_plinks_ )
    {
        ASSERT( plink );
        ASSERT( plink.GetChildAgent() == this )("Parent link supplied for different agent");
        InsertSolo( coupled_plinks, plink );
    }

    // It works with a set, but if we lose this ordering hints from 
    // Colocated agent NLQs become less useful and CSP solver slows right
    // down.
    if( phase != IN_REPLACE_ONLY && base_plink )
    {
        base_and_normal_plinks.clear();
        base_and_normal_plinks.push_back( base_plink );
        for( PatternLink plink : pattern_query->GetNormalLinks() )
            base_and_normal_plinks.push_back( plink );
    }
}
                                

void AgentCommon::AddResiduals( set<PatternLink> coupled_plinks_ )
{
    for( PatternLink plink : coupled_plinks_ )
    {
        ASSERT( plink );
        ASSERT( plink.GetChildAgent() == this )("Parent link supplied for different agent");
        InsertSolo( coupled_plinks, plink );
    }
}


list<PatternLink> AgentCommon::GetVisibleChildren( Path v ) const
{
	// Normally all children should be visible 
    typedef ContainerFromIterator< FlattenNode_iterator, const Node * > FlattenNodePtr;
    auto c = shared_ptr<ContainerInterface>( new FlattenNodePtr(this) );
    // Note: a pattern query should be just as good...
    // Incorrect! This gets the replace-side stuff as well; GetPatternQuery()
    // is only for search.
    
    list<PatternLink> plinks;
    FOREACH( const TreePtrInterface &tpi, *c )
    {        
        if( tpi )
            plinks.push_back( PatternLink(this, &tpi) );
    }
    return plinks;
}

    
shared_ptr<DecidedQuery> AgentCommon::CreateDecidedQuery() const
{
    ASSERT( master_scr_engine ); // check we have been configured
    
    return make_shared<DecidedQuery>( pattern_query );
}
    
    
void AgentCommon::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                       XLink base_xlink ) const
{
    ASSERTFAIL();
}                                       


void AgentCommon::RunDecidedQuery( DecidedQueryAgentInterface &query,
                                   XLink base_xlink ) const
{
    // Admin stuff every DQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;   
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    RunDecidedQueryImpl( query, base_xlink );
}                             


bool AgentCommon::ImplHasNLQ() const
{    
    return false;
}

    
bool AgentCommon::NLQRequiresBase() const
{
    return true;
}                                         


void AgentCommon::RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                            const TheKnowledge *knowledge ) const
{
    ASSERTFAIL();
}
    
    
void AgentCommon::NLQFromDQ( const SolutionMap *required_links,
                             const TheKnowledge *knowledge ) const
{    
    TRACE("common DNLQ: ")(*this)(" at ")(base_plink)("\n");
    
    // Can't do baseless query using DQ
    ASSERT( NLQRequiresBase() ); // Agent shouldn't advertise
    ASSERT( required_links->count(base_plink) ); // Solver shouldn't try
    
    auto query = CreateDecidedQuery();
    RunDecidedQueryImpl( *query, required_links->at(base_plink) );
    
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
        if(required_links->count(plink)==0)
            continue; // partial query support
            
        LocatedLink rlink( plink, required_links->at(plink) );
        ASSERT( alink.GetChildAgent() == rlink.GetChildAgent() );                
        if( (XLink)alink == XLink::MMAX_Link )
            continue; // only happens when agent pushes out MMAX, as with DisjunctionAgent
            
        // Compare by location
        if( (XLink)alink != (XLink)rlink ) 
        {
            NLQFromDQLinkMismatch e; // value of links mismatches
#ifdef HINTS_IN_EXCEPTIONS
            e.hint = alink;
#endif            
            throw e;       
        }                          
    }            
}                           
                                
    
void AgentCommon::RunNormalLinkedQuery( const SolutionMap *required_links,
                                        const TheKnowledge *knowledge ) const
{
    if( ImplHasNLQ() )    
        RunNormalLinkedQueryImpl( required_links, knowledge );
    else
        NLQFromDQ( required_links, knowledge );               
}                                            


void AgentCommon::RunCouplingQuery( const SolutionMap *required_links, multiset<XLink> candidate_links )
{    
    // This function establishes the policy for couplings in one place.
    // Today, it's SimpleCompare, via EquivalenceRelation, with MMAX excused. 
    // And it always will be: see #121; para starting at "No!!"
    // HOWEVER: it is now possible for agents to override this policy.

    if( ReadArgs::new_feature )
    {
        multiset<XLink> my_candidate_links;    
        my_candidate_links.insert( required_links->at(base_plink) ); // insert base xlink (keyer)
        for( PatternLink coupled_plink : coupled_plinks )
            if( required_links->count(coupled_plink) ) // could be partial query
                my_candidate_links.insert( required_links->at(coupled_plink) ); // insert coupled x links if required (residuals)

        ASSERT( candidate_links.size() == my_candidate_links.size() )(*this)
              ("\ncandidate_links:\n")(candidate_links)
              ("\nmy_candidate_links:\n")(my_candidate_links)
              ("\nrequired_links:\n")(required_links)
              ("\ncoupled_plinks:\n")(coupled_plinks)("\n");
    }
    
    // Note: having combined keyer and residuals into a single multimap,
    // we proceed with a symmetrical algorithm.

    // We will always accept MMAX links, so ignore them
    candidate_links.erase(XLink::MMAX_Link);

    // Check remaining links against each other. EquivalenceRelation is
    // transitive, so it's enough just to daisy-chain the checks.
    XLink previous_link;
    for( XLink current_link : candidate_links )
    {
        if( previous_link )
        {
            CompareResult cr = equivalence_relation.Compare( previous_link, 
                                                             current_link );
            if( cr != EQUAL )
                throw CouplingMismatch();               
        }
        previous_link = current_link;
    }     
}


void AgentCommon::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                            const SolutionMap *required_links,
                                            const TheKnowledge *knowledge ) const
{
}
    
    
void AgentCommon::RunRegenerationQuery( DecidedQueryAgentInterface &query,
                                        const SolutionMap *required_links,
                                        const TheKnowledge *knowledge ) const
{
    // Admin stuff every RQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    XLink base_xlink = required_links->at(base_plink);
    if( base_xlink != XLink::MMAX_Link )
        this->RunRegenerationQueryImpl( query, required_links, knowledge );
}                             
                      
                      
AgentCommon::QueryLambda AgentCommon::StartRegenerationQuery( const SolutionMap *required_links,
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
                RunRegenerationQuery( *query, required_links, knowledge );       
                    
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
                                              
                                              
AgentCommon::QueryLambda AgentCommon::TestStartRegenerationQuery( const SolutionMap *required_links,
                                                                  const TheKnowledge *knowledge ) const
{
    QueryLambda mut_lambda;
    QueryLambda ref_lambda;
    auto mut_hits = make_shared<int>(0);
    auto ref_hits = make_shared<int>(0);
    try
    {
        mut_lambda = StartRegenerationQuery( required_links, knowledge, false );
    }
    catch( ::Mismatch &e ) 
    {
        try
        {
            Tracer::RAIIEnable silencer( false ); // make ref algo be quiet            
            (void)StartRegenerationQuery( required_links, knowledge, true );
            ASSERT(false)("MUT start threw ")(e)(" but ref didn't\n")
                         (*this)("\n")
                         ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *required_links))("\n")
                         ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *required_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *required_links))("\n");
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
        Tracer::RAIIEnable silencer( false ); // make ref algo be quiet             
        ref_lambda = StartRegenerationQuery( required_links, knowledge, true );        
    }
    catch( ::Mismatch &e ) 
    {
        ASSERT(false)("Ref start threw ")(e)(" but MUT didn't\n")
                     (*this)("\n")
                     ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *required_links))("\n")
                     ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *required_links))("\n")
                     ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *required_links))("\n");
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
                    Tracer::RAIIEnable silencer( false ); // make ref algo be quiet            
                    ref_query = ref_lambda(); 
                }
                TRACE("Ref lambda hit #%d\n", (*ref_hits))
                     (*ref_query)("\n");
                (*ref_hits)++;                
                ASSERT(false)("MUT lambda threw ")(e)(" but ref didn't\n")
                             ("MUT hits %d, ref hits %d\n", *mut_hits, *ref_hits)
                             (*this)("\n")
                             ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *required_links))("\n")
                             ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *required_links))("\n")
                             ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *required_links))("\n");
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
                Tracer::RAIIEnable silencer( false ); // make ref algo be quiet             
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
                         ("Normal: ")(MapForPattern(pattern_query->GetNormalLinks(), *required_links))("\n")
                         ("Abormal: ")(MapForPattern(pattern_query->GetAbnormalLinks(), *required_links))("\n")
                         ("Multiplicity: ")(MapForPattern(pattern_query->GetMultiplicityLinks(), *required_links))("\n");
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


void AgentCommon::SetKey( CouplingKey key )
{
    ASSERT( master_scr_engine );
    ASSERT(key);
    if( phase != IN_COMPARE_ONLY )
        ASSERT( key.IsFinal() )(*this)(" trying to key with non-final ")(key)("\n"); 
    master_scr_engine->SetReplaceKey( this, key );
}


CouplingKey AgentCommon::GetKey()
{
    ASSERT( master_scr_engine );
    return master_scr_engine->GetReplaceKey( this );
}


PatternLink AgentCommon::GetKeyerPatternLink()
{
    ASSERT( master_and_rule_engine )(*this)(" has not been configured by any AndRuleEngine");
    ASSERT( base_plink )(*this)(" has no base_plink, engine=")(master_and_rule_engine)("\n");
    
    return base_plink;
}


void AgentCommon::Reset()
{
}


void AgentCommon::KeyForOverlay( Agent *from )
{
    // This function is called on nodes under the "overlay" branch of Overlay nodes.
    // Some special nodes will not know what to do...
    //ASSERT(GetKey())(*this)(" cannot appear in a replace-only context");
}


TreePtr<Node> AgentCommon::BuildReplace()
{
    INDENT("B");
    
    ASSERT(this);
    ASSERT(master_scr_engine)("Agent ")(*this)(" appears not to have been configured");
    ASSERT( phase != IN_COMPARE_ONLY )(*this)(" is configured for compare only");
    ASSERT( !GetKey() || GetKey().IsFinal() )(*this)(" keyed with non-final node ")(GetKey())("\n"); 
    
    TreePtr<Node> dest;
    if( GetKey() )
        dest = BuildReplaceImpl(GetKey().GetKeyXNode(KEY_CONSUMER_5));
    else 
        dest = BuildReplaceImpl(nullptr);    
    ASSERT( dest );
    ASSERT( dest->IsFinal() )(*this)(" built non-final ")(*dest)("\n"); 
    
    return dest;
}


TreePtr<Node> AgentCommon::BuildReplaceImpl( TreePtr<Node> keynode )
{
    ASSERT(keynode)("Unkeyed search-only agent seen in replace context");
    return DuplicateSubtree(keynode);   
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
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_ptr)("\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("source should be non-nullptr");
            *dest_ptr = DuplicateSubtree( (TreePtr<Node>)*keynode_ptr, source_terminus, dest_terminus );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
}


string AgentCommon::GetTrace() const
{
    string s = Traceable::GetName() + GetSerialString();
    switch( phase )
    {
    case IN_COMPARE_ONLY:
        s += "/C";
        break;
    case IN_COMPARE_AND_REPLACE:
        s += "/CR";
        break;
    case IN_REPLACE_ONLY:
        s += "/R";
        break;
    }
    return s;
}


string AgentCommon::GetGraphId() const
{
	return GetSerialString();
}

//---------------------------------- DefaultMMAXAgent ------------------------------------    

void DefaultMMAXAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink base_xlink ) const
{
    if( base_xlink == XLink::MMAX_Link )
    {
        // Magic Match Anything node: all normal children also match anything
        // This is just to keep normal-domain solver happy, so we 
        // only need normals. 
        for( PatternLink plink : pattern_query->GetNormalLinks() )       
            query.RegisterNormalLink( plink, base_xlink );
    }   
    else
    {
        RunDecidedQueryMMed( query, base_xlink );
    }
}


void DefaultMMAXAgent::RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{
    XLink base_xlink;
    if( required_links->count(base_plink) > 0 )
        base_xlink = required_links->at(base_plink);

    // Baseless or MMAX query strategy: hand-rolled
    if( !base_xlink || base_xlink == XLink::MMAX_Link )
    {
        bool saw_non_mmax = false;
        for( PatternLink plink : pattern_query->GetNormalLinks() ) 
        {
            if( required_links->count(plink) > 0 )
            {
                XLink req_xlink = required_links->at(plink);
                if( req_xlink != XLink::MMAX_Link )
                    saw_non_mmax = true;                    
            }
        }   
        
        if( !saw_non_mmax )
            return; // Done: all are MMAX
        
        if( base_xlink == XLink::MMAX_Link )
            throw MMAXPropagationMismatch(); // Mismatch: there are non-MMAX but base is MMAX
    }   
    
    ASSERT( base_xlink != XLink::MMAX_Link );
    RunNormalLinkedQueryMMed( required_links, knowledge );
}

                               
void DefaultMMAXAgent::RunNormalLinkedQueryMMed( const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const                                      
{                      
    ASSERTFAIL("Please implement RunNormalLinkedQueryMMed()\n");
}                     

//---------------------------------- PreRestrictedAgent ------------------------------------    

void PreRestrictedAgent::RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                              XLink base_xlink ) const
{
    // Check pre-restriction
    if( !IsLocalMatch( base_xlink.GetChildX().get() ) )
        throw PreRestrictionMismatch();
            
    RunDecidedQueryPRed( query, base_xlink );
}


void PreRestrictedAgent::RunNormalLinkedQueryMMed( const SolutionMap *required_links,
                                                   const TheKnowledge *knowledge ) const
{
    // Baseless query strategy: don't check pre-restriction
    bool based = (required_links->count(base_plink) == 1);
    if( based )
    { 
        // Check pre-restriction
        if( !IsLocalMatch( required_links->at(base_plink).GetChildX().get() ) )
            throw PreRestrictionMismatch();
    }
    
    RunNormalLinkedQueryPRed( required_links, knowledge );
}

                               
void PreRestrictedAgent::RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                                   const TheKnowledge *knowledge ) const                                      
{                      
    ASSERTFAIL("Please implement RunNormalLinkedQueryPRed()\n");
}                     

//---------------------------------- TeleportAgent ------------------------------------    

void TeleportAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink base_xlink ) const
{
    INDENT("T");
    
    auto op = [&](XLink base_xlink) -> map<PatternLink, XLink>
    {
        map<PatternLink, XLink> tp_links = RunTeleportQuery( base_xlink );
        
        // We will uniquify the link against the domain and then cache it against base_xlink
        
        for( pair<PatternLink, XLink> p : tp_links )
            p.second = master_scr_engine->UniquifyDomainExtension(p.second); // in-place
                   
        return tp_links;
    };
    
    map<PatternLink, XLink> cached_links = cache( base_xlink, op );
    for( LocatedLink cached_link : cached_links )
    {   
        ASSERT( cached_link );
        query.RegisterNormalLink( (PatternLink)cached_link, (XLink)cached_link );
    }    
}                                    


set<XLink> TeleportAgent::ExpandNormalDomain( const unordered_set<XLink> &base_xlinks )
{
    set<XLink> extra_xlinks;
    for( XLink base_xlink : base_xlinks )
    {
        if( base_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because all child patterns are also MMAX
        if( !IsLocalMatch( base_xlink.GetChildX().get() ) )
            continue; // Failed pre-restriction so can't expand domain

        try
        {
            shared_ptr<DecidedQuery> query = CreateDecidedQuery();
            RunDecidedQueryPRed( *query, base_xlink );
           
            for( LocatedLink extra_link : query->GetNormalLinks() )
                extra_xlinks.insert( (XLink)extra_link );
        }
        catch( ::Mismatch & ) {}
    }
    return extra_xlinks;
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
    cache.Reset();
}

//---------------------------------- SearchLeafAgent ------------------------------------    

shared_ptr<PatternQuery> SearchLeafAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void SearchLeafAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                           XLink base_xlink ) const
{
}

//---------------------------------- SpecialBase ------------------------------------    

bool SpecialBase::IsNonTrivialPreRestriction(const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>((TreePtr<Node>)*ptr) )   // is to a special node
        {
            if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchitype()) ) )    // pre-restrictor is nontrivial
            {
                return true;
            }
        }
    }
    return false;
}
