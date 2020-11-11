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

//#define CHECK_LINKS_COMPARISON

using namespace SR;

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


AgentCommon::AgentCommon() :
    master_scr_engine(nullptr)
{
}


void AgentCommon::AgentConfigure( const SCREngine *e )
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
    
    pattern_query = GetPatternQuery();
    num_decisions = pattern_query->GetDecisions().size();
}


shared_ptr<ContainerInterface> AgentCommon::GetVisibleChildren() const
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
    
    
void AgentCommon::RunDecidedQuery( DecidedQueryAgentInterface &query,
                                   XLink x ) const
{
    query.last_activity = DecidedQueryCommon::QUERY;
   
    DecidedQueryAgentInterface::RAIIDecisionsCleanup cleanup(query);
    
    if( x == XLink::MMAX_Link )
    {
        query.Reset();
        // Magic Match Anything node: all normal children also match anything
        // This is just to keep normal-domain solver happy, so we 
        // only need normals. 
        for( PatternLink l : pattern_query->GetNormalLinks() )       
            query.RegisterNormalLink( PatternLink(this, l.GetPatternPtr()), x );
    }   
    else
    {
        RunDecidedQueryImpl( query, x );
    }
}                             


void AgentCommon::ResumeNormalLinkedQuery( Conjecture &conj,
                                           XLink x,
                                           const list<LocatedLink> &required_links ) const
{    
    while(1)
    {
        try
        {
            shared_ptr<DecidedQuery> query = conj.GetQuery(this);
            {
                Tracer::RAIIEnable silencer( false ); // make DQ be quiet
                RunDecidedQuery( *query, x );
            }
            
            // The query now has populated links, which should be full
            // (otherwise RunDecidedQuery() should have thrown). We loop 
            // over both and check that they refer to the same x nodes
            // we were passed. Mismatch will throw, same as in DQ.
            auto actual_links = query->GetNormalLinks();
            ASSERT( actual_links.size() == required_links.size() );
            // TRACE("Actual links   ")(actual_links)("\n");
            // TRACEC("Required links ")(required_links)("\n");
            
            bool match = true;
            auto alit = actual_links.begin();
            auto rlit = required_links.begin();
            for( ;
                 alit != actual_links.end() || rlit != required_links.end();
                 ++alit, ++rlit )
            {
                if( alit == actual_links.end() || rlit == required_links.end() )
                {
                    match = false;
                    break;
                }
                LocatedLink alink = *alit;
                LocatedLink rlink = *rlit;
                ASSERT( alink.GetChildAgent() == rlink.GetChildAgent() );                
                if( (XLink)alink == XLink::MMAX_Link )
                    continue;
                // Compare by location
#ifdef CHECK_LINKS_COMPARISON
                if( alink.GetChildX() == rlink.GetChildX() )
                {
                    if( !TreePtr<CPPTree::Identifier>::DynamicCast( alink.GetChildX() ) )
                    {
                        if( auto tp = dynamic_cast<const TransformOfAgent *>(this) )
                        {
                            ASSERT( (XLink)alink == (XLink)rlink )
                                  ("Found conflicting X links for ")(alink.GetChildAgent())("\n")
                                  ("Actual   ")(alink)("\n")
                                  ("Required ")(rlink)("\n")
                                  ("TransformOfAgent's cache ")(tp->cache.cache)("\n");
                        }
                        else
                        {
                            ASSERT( (XLink)alink == (XLink)rlink )
                                  ("Found conflicting X links for ")(alink.GetChildAgent())("\n")
                                  ("Actual   ")(alink)("\n")
                                  ("Required ")(rlink)("\n");
                        }
                    }
                }
                else
#else                
                if( (XLink)alink != (XLink)rlink )
#endif                
                {
                    match = false;
                    // TODO break?
                }                 
            }
            
            if( match )
                break; // Great, the normal links matched
        }
        catch( ::Mismatch & ) {}
        
        Tracer::RAIIEnable silencer( false ); // make conjecture be quiet
        // We will get here on a mismatch, whether detected by DQ or our
        // own comparison of the normal links. Permit the conjecture
        // to move to a new set of choices.
        if( !conj.Increment() )
            throw NormalLinksMismatch(); // Conjecture has run out of choices to try.
            
        // Conjecture would like us to try again with new choices
    }     
}                           


void AgentCommon::RunNormalLinkedQuery( shared_ptr<DecidedQuery> query,
                                        XLink x,
                                        const list<LocatedLink> &required_links ) const
{
    Conjecture conj(this, query);            
    conj.Start();
    
    // RunNormalLinkedQuery() only wants to determine whether there
    // is at least one match, so a single call suffices. To get all
    // the matches, call ResumeNormalLinkedQuery() directly with 
    // your own Conjecture object.
    ResumeNormalLinkedQuery( conj, x, required_links );
}


void AgentCommon::CouplingQuery( multiset<XLink> candidate_links )
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
            if( !equivalence_relation( previous_link.GetChildX(), 
                                       current_link.GetChildX() ) )
                throw CouplingMismatch();               
        }
        previous_link = current_link;
    }     
}


void AgentCommon::SetKey( CouplingKey keylink )
{
    ASSERT(keylink);
    if( !coupling_key )
    { 
        coupling_key = keylink;
    }
}


CouplingKey AgentCommon::GetKey()
{
    return coupling_key; 
}


void AgentCommon::ResetKey()
{
    coupling_key = LocatedLink();
}


void AgentCommon::KeyReplace( const CouplingKeysMap *coupling_keys )
{   
    SetKey( coupling_keys->at(this) );  
}


void AgentCommon::TrackingKey( Agent *from )
{
    // This function is called on nodes under the "overlay" branch of Overlay nodes.
    // Some special nodes will not know what to do...
    ASSERT(GetKey())(*this)(" cannot appear in a replace-only context");
}


TreePtr<Node> AgentCommon::BuildReplace()
{
    INDENT("B");
    ASSERT(this);
    ASSERT(master_scr_engine)("Agent ")(*this)(" at appears not to have been configured");
    
    // See if the pattern node is coupled to anything. The keynode that was passed
    // in is just a suggestion and will be overriden if we are keyed.
    CouplingKey keylink = GetKey();
    
    return BuildReplaceImpl( keylink );    
}


TreePtr<Node> AgentCommon::BuildReplaceImpl( CouplingKey keylink )
{
    ASSERT(keylink)("Unkeyed search-only agent seen in replace context");
    return DuplicateSubtree(keylink.GetChildX());   
}


TreePtr<Node> AgentCommon::DuplicateNode( TreePtr<Node> source,
                                          bool force_dirty ) const
{
    INDENT("D");

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
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
                TreePtr<Node> n = DuplicateSubtree( p, source_terminus, dest_terminus );
                //TRACE("inserting ")(*n)(" directly\n");
                dest_con->insert( n );
            }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_ptr)("\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("source should be non-nullptr");
            *dest_ptr = DuplicateSubtree( *keynode_ptr, source_terminus, dest_terminus );
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


set<XLink> AgentCommonDomainExtender::ExpandNormalDomain( XLink x )
{
    set<XLink> extras;
    Conjecture conj(this);            
    conj.Start();
    
    do
    {
        try
        {
            shared_ptr<DecidedQuery> query = conj.GetQuery(this);
            {
                Tracer::RAIIEnable silencer( false ); // make DQ be quiet
                RunDecidedQuery( *query, x );
            }
            
            for( LocatedLink link : query->GetNormalLinks() )
                extras.insert( (XLink)link );
        }
        catch( ::Mismatch & ) {}
    }
    while( conj.Increment() );
    
    return extras;
}
