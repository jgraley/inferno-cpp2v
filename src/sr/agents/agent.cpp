#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"
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
    ASSERT( node )("Called AsAgent(")(node)(") with null shared_ptr");
    Agent *agent = dynamic_cast<Agent *>(node.get());
    ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


const Agent *Agent::AsAgentConst( shared_ptr<const Node> node )
{
    ASSERT( node )("Called AsAgent(")(node)(") with null shared_ptr");
    const Agent *agent = dynamic_cast<const Agent *>(node.get());
    ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


//---------------------------------- AgentCommon ------------------------------------    

AgentCommon::AgentCommon() :
    master_scr_engine(nullptr)
{
}


void AgentCommon::AgentConfigure( Phase phase_, const SCREngine *e )
{
    // Repeat configuration regarded as an error because it suggests I maybe don't
    // have a clue what should actaually be configing the agent. Plus general lifecycle 
    // rule enforcement.
    // Why no coupling across sibling slaves? Would need an ordering for keying
    // but ordering not defined on sibling slaves.
    ASSERT(!master_scr_engine)("Detected repeat configuration of ")(*this)
                              ("\nCould be result of coupling this node across sibling slaves - not allowed :(");
    ASSERT(e);
    master_scr_engine = e;
    phase = phase_;
    
    if( phase != IN_REPLACE_ONLY )
    {
        pattern_query = GetPatternQuery();
        num_decisions = pattern_query->GetDecisions().size();

        // We will need a conjecture, so that we can iterate through multiple 
        // potentially valid values for the abnormals and multiplicities.
        nlq_conjecture = make_shared<Conjecture>(this);            
    }
}


shared_ptr<ContainerInterface> AgentCommon::GetVisibleChildren( Path v ) const
{
	// Normally all children should be visible 
   typedef ContainerFromIterator< FlattenNode_iterator, const Node * > FlattenNodePtr;
   return shared_ptr<ContainerInterface>( new FlattenNodePtr(this) );
    // Note: a pattern query should be just as good...
    // Incorrect! This gets the replace-side stuff as well; GetPatternQuery()
    // is only for search.
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


void AgentCommon::RunNormalLinkedQueryImpl( PatternLink base_plink,
                                            const SolutionMap *required_links,
                                            const TheKnowledge *knowledge ) const
{
    ASSERTFAIL();
}
    
    
bool AgentCommon::ImplHasNLQ() const
{    
    return false;
}

    
void AgentCommon::NLQFromDQ( PatternLink base_plink,
                             const SolutionMap *required_links,
                             const TheKnowledge *knowledge ) const
{    
    TRACE("common DNLQ: ")(*this)(" at ")(base_plink)("\n");
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
                                
    
void AgentCommon::RunNormalLinkedQuery( PatternLink base_plink,
                                        const SolutionMap *required_links,
                                        const TheKnowledge *knowledge ) const
{
    if( ImplHasNLQ() )    
        RunNormalLinkedQueryImpl( base_plink, required_links, knowledge );
    else
        NLQFromDQ( base_plink, required_links, knowledge );               
}                                            


void AgentCommon::RunCouplingQuery( multiset<XLink> candidate_links )
{    
    // This function establishes the policy for couplings in one place.
    // Today, it's SimpleCompare, via EquivalenceRelation, with MMAX excused. 
    // And it always will be: see #121; para starting at "No!!"
    // HOWEVER: it is now possible for agents to override this policy.

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
                                            XLink base_xlink,
                                            const SolutionMap *required_links,
                                            const TheKnowledge *knowledge ) const
{
}
    
    
void AgentCommon::RunRegenerationQuery( DecidedQueryAgentInterface &query,
                                        XLink base_xlink,
                                        const SolutionMap *required_links,
                                        const TheKnowledge *knowledge ) const
{
    // Admin stuff every RQ has to do
    query.last_activity = DecidedQueryCommon::QUERY;
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    query.Reset(); 

    if( base_xlink != XLink::MMAX_Link )
        this->RunRegenerationQueryImpl( query, base_xlink, required_links, knowledge );
}                             
                      
                      
AgentCommon::QueryLambda AgentCommon::StartRegenerationQuery( XLink base_xlink,
                                                              const SolutionMap *required_links,
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
                RunRegenerationQuery( *query, base_xlink, required_links, knowledge );       
                    
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
                                              
                                              
AgentCommon::QueryLambda AgentCommon::TestStartRegenerationQuery( XLink base_xlink,
                                                                  const SolutionMap *required_links,
                                                                  const TheKnowledge *knowledge ) const
{
    QueryLambda mut_lambda;
    QueryLambda ref_lambda;
    auto mut_hits = make_shared<int>(0);
    auto ref_hits = make_shared<int>(0);
    try
    {
        mut_lambda = StartRegenerationQuery( base_xlink, required_links, knowledge, false );
    }
    catch( ::Mismatch &e ) 
    {
        try
        {
            Tracer::RAIIEnable silencer( false ); // make ref algo be quiet            
            (void)StartRegenerationQuery( base_xlink, required_links, knowledge, true );
            ASSERT(false)("MUT start threw ")(e)(" but ref didn't\n")
                         (*this)(" at ")(base_xlink)("\n")
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
        ref_lambda = StartRegenerationQuery( base_xlink, required_links, knowledge, true );        
    }
    catch( ::Mismatch &e ) 
    {
        ASSERT(false)("Ref start threw ")(e)(" but MUT didn't\n")
                     (*this)(" at ")(base_xlink)("\n")
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
                             (*this)(" at ")(base_xlink)("\n")
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
                         (*this)(" at ")(base_xlink)("\n")
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


void AgentCommon::SetKey( CouplingKey keylink )
{
    ASSERT(keylink);
    if( phase != IN_COMPARE_ONLY )
        ASSERT(keylink.GetChildX()->IsFinal() )(*this)(" trying to key with non-final ")(keylink)("\n"); 
   
    if( !coupling_key )
    { 
        coupling_key = keylink;
    }
}


CouplingKey AgentCommon::GetKey()
{
    return coupling_key; 
}


void AgentCommon::Reset()
{
    coupling_key = LocatedLink();
}


void AgentCommon::KeyReplace( const CouplingKeysMap *coupling_keys )
{   
    SetKey( coupling_keys->at(this) );  
}


void AgentCommon::KeyForOverlay( Agent *from )
{
    // This function is called on nodes under the "overlay" branch of Overlay nodes.
    // Some special nodes will not know what to do...
    ASSERT(GetKey())(*this)(" cannot appear in a replace-only context");
}


TreePtr<Node> AgentCommon::BuildReplace()
{
    INDENT("B");
    
    ASSERT(this);
    ASSERT(master_scr_engine)("Agent ")(*this)(" appears not to have been configured");
    ASSERT( phase != IN_COMPARE_ONLY )(*this)(" is configured for compare only");
    
    // See if the pattern node is coupled to anything. The keynode that was passed
    // in is just a suggestion and will be overriden if we are keyed.   
    ASSERT( !GetKey() || GetKey().GetChildX()->IsFinal() )(*this)(" keyed with non-final ")(GetKey())("\n"); 
    
    TreePtr<Node> dest = BuildReplaceImpl();    
    ASSERT( dest );
    ASSERT( dest->IsFinal() )(*this)(" built non-final ")(*dest)("\n"); 
    
    return dest;
}


TreePtr<Node> AgentCommon::BuildReplaceImpl()
{
    ASSERT(GetKey())("Unkeyed search-only agent seen in replace context");
    return DuplicateSubtree(GetKey().GetChildX());   
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
    INDENT("D");
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


void DefaultMMAXAgent::RunNormalLinkedQueryImpl( PatternLink base_plink,
                                                 const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{
    if( required_links->at(base_plink) == XLink::MMAX_Link )
    {
        for( PatternLink plink : pattern_query->GetNormalLinks() ) 
        {
            if( required_links->count(plink) > 0 )
            {
                XLink req_xlink = required_links->at(plink);
                if( req_xlink != XLink::MMAX_Link )
                    throw MMAXPropagationMismatch();
            }
        }   
        return; // Done: all are MMAX
    }   
    
    RunNormalLinkedQueryMMed( base_plink, required_links, knowledge );
}

                               
void DefaultMMAXAgent::RunNormalLinkedQueryMMed( PatternLink base_plink,
                                                 const SolutionMap *required_links,
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


void PreRestrictedAgent::RunNormalLinkedQueryMMed( PatternLink base_plink,
                                                   const SolutionMap *required_links,
                                                   const TheKnowledge *knowledge ) const
{
    // Check pre-restriction if based
    if( required_links->count(base_plink) == 1 )
        if( !IsLocalMatch( required_links->at(base_plink).GetChildX().get() ) )
            throw PreRestrictionMismatch();
    
    RunNormalLinkedQueryPRed( base_plink, required_links, knowledge );
}

                               
void PreRestrictedAgent::RunNormalLinkedQueryPRed( PatternLink base_plink,
                                                   const SolutionMap *required_links,
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
/*
    map<XLink, XLink> extra;
    for( XLink base_xlink : xlinks )
    {
        if( base_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because all child patterns are also MMAX
        if( !IsLocalMatch( base_xlink.GetChildX().get() ) )
            continue; // Failed pre-restriction so can't expand domain
            
        try
        {
            map<PatternLink, XLink> tp_links = RunTeleportQuery( base_xlink );
                    
            for( pair<PatternLink, XLink> p : tp_links )
                p.second = master_scr_engine->UniquifyDomainExtension(p.second); // in-place
            
            ASSERT( tp_links.size() <= 1 ); // TODO this method should return a set<XLink> - caller doesn't need the plinks
            for( LocatedLink new_link : tp_links )
                extra[base_xlink] = (XLink)new_link;
        }
        catch( ::Mismatch & ) {}
    }
    return extra;
*/

void TeleportAgent::Reset()
{
    AgentCommon::Reset();
    cache.Reset();
}

//---------------------------------- BuilderAgent ------------------------------------    

shared_ptr<PatternQuery> BuilderAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void BuilderAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                        XLink base_xlink ) const
{
}
