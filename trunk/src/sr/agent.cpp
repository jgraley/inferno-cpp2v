#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

using namespace SR;

Agent *Agent::AsAgent( TreePtr<Node> node )
{
    ASSERT( node )("Called AsAgent(")(node)(") with NULL TreePtr");
    Agent *agent = dynamic_cast<Agent *>(node.get());
    ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


AgentCommon::AgentCommon() :
    engine(NULL),
    pconj(NULL)
{
}


void AgentCommon::AgentConfigure( const Engine *e )
{
    // Repeat configuration regarded as an error because it suggests I maybe don't
    // have a clue what should actaually be configing the agent. Plus general lifecycle 
    // rule enforcement.
    // Why no coupling across sibling slaves? Would need an ordering for keying
    // but ordering not defined on sibling slaves.
    ASSERT(!engine)("Detected repeat configuration of ")(*this)
                   ("\nCould be result of coupling this node across sibling slaves - not allowed :(");
    ASSERT(e);
    engine = e;
}


bool AgentCommon::DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj )
{
    INDENT(" ");
    ASSERT(engine)("Agent ")(*this)(" at appears not to have been configured");
    ASSERT( &x ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( x ); // Target must not be NULL
    ASSERT(this);
    
    // Do the agent-specific local checks (x versus characteristics of the present agent)
    // Also takes notes of how child agents link to children of x (depending on conjecture)
    ClearLinks();    
    pconj = &conj;
        
    // Run the compare implementation with couplings check/update
    bool match = DecidedCompareCoupled( x, can_key );

    pconj = NULL;
    
    // Follow up on any links that were noted by the agent impl
    if( match )
        match = DecidedCompareLinks( can_key, conj );
      
    return match;
}


bool AgentCommon::DecidedCompareCoupled( const TreePtrInterface &x,
                                         bool can_key )
{
    // If the agent is coupled already, check for a coupling match
    if( TreePtr<Node> keynode = GetCoupled() )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }

    bool match = DecidedCompareImpl( x, can_key );
    if( !match )
        return false;
    
    // Note that if the DecidedCompareImpl() already keyed, then this does nothing.
    if( can_key )
    {
        DoKey( x );  
    }
    
    return true;
}

    
bool AgentCommon::AbnormalCompare( const TreePtrInterface &x, 
                                   bool can_key ) 
{
    INDENT("A");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*this);

    // Only run during "restricting" pass
    if( can_key )
        return true;

    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj;
    bool r;
    while(1)
    {
        conj.PrepareForDecidedCompare();
        r = DecidedCompare( x, false, conj );
        
        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
            break; // Success
            
        if( !conj.Increment() )
            break; // Failure            
    }
    return r;
}


void AgentCommon::DoKey( TreePtr<Node> x )
{
    ASSERT(x);
    shared_ptr<Key> key( new Key );
    key->root = x;
    DoKey( key );
}


void AgentCommon::DoKey( shared_ptr<Key> key )
{
    if( !coupling_key )
    {
        coupling_key = key;
    }
}


TreePtr<Node> AgentCommon::GetCoupled()
{
    shared_ptr<Key> k = GetKey();
    if( k )
        return k->root;
    else
        return TreePtr<Node>(); 
}


shared_ptr<AgentCommon::Key> AgentCommon::GetKey()
{
    return coupling_key;
}


void AgentCommon::ResetKey()
{
    coupling_key = shared_ptr<Key>();
}


void AgentCommon::ClearLinks()
{
    links.clear();
}


void AgentCommon::RememberLink( bool abnormal, Agent *a, const TreePtrInterface &x )
{
    Links::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    l.px = &x;
    l.local_x = TreePtr<Node>();
    l.invert = false;
    links.links.insert( l );
}


void AgentCommon::RememberInvertedLink( Agent *a, const TreePtrInterface &x )
{
    Links::Link l;
    l.abnormal = true; // always
    l.agent = a;
    l.px = &x;
    l.local_x = TreePtr<Node>();
    l.invert = true;
    links.links.insert( l );
}


void AgentCommon::RememberLocalLink( bool abnormal, Agent *a, TreePtr<Node> x )
{
    ASSERT(x);
    Links::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    l.px = NULL;    
    l.local_x = x;
    l.invert = false;
    links.links.insert( l );
}


ContainerInterface::iterator AgentCommon::HandleDecision( ContainerInterface::iterator begin,
                                                          ContainerInterface::iterator end )
{
    ASSERT( pconj )("AgentCommon::HandleDecision() called outside of DecidedCompareImpl()");
    return pconj->HandleDecision( begin, end );
}


bool SR::operator<(const SR::Links::Link &l0, const SR::Links::Link &l1)
{
    if( l0.abnormal != l1.abnormal )
        return (int)l0.abnormal < (int)l1.abnormal;
    if( l0.agent != l1.agent )
        return l0.agent < l1.agent;
    if( l0.px != l1.px )
        return l0.px < l1.px;    
    if( l0.local_x != l1.local_x )
        return l0.local_x < l1.local_x;    
    if( l0.invert != l1.invert )
        return (int)l0.invert < (int)l1.invert;
    
    return false; // equal
}


bool AgentCommon::DecidedCompareLinks( bool can_key,
                                       Conjecture &conj )
{
    // Follow up on any links that were noted by the agent impl
    FOREACH( const Links::Link &l, links.links )
    {
        bool r;
        const TreePtrInterface *px;
        if( l.px )
            px = l.px;
        else
            px = &(l.local_x);        
        if( l.abnormal )
            r = l.agent->AbnormalCompare(*px, can_key);
        else
            r = l.agent->DecidedCompare(*px, can_key, conj);
        if( l.invert )
            r = !r || can_key; // only apply during restricting pass 
        if( !r )
            return false;
    }
    return true;
}


void AgentCommon::KeyReplace()
{    
}


void AgentCommon::TrackingKey( Agent *from )
{
    // This function is called on nodes under the "overlay" branch of Overlay nodes.
    // Some special nodes will not know what to do...
    ASSERT(GetKey())(*this)(" cannot appear in a replace-only context");
}


TreePtr<Node> AgentCommon::BuildReplace()
{
    INDENT(" ");
    ASSERT(this);
    ASSERT(engine)("Agent ")(*this)(" at appears not to have been configured");
    
    // See if the pattern node is coupled to anything. The keynode that was passed
    // in is just a suggestion and will be overriden if we are keyed.
    TreePtr<Node> keynode = GetCoupled();
    
    return BuildReplaceImpl( keynode );    
}


TreePtr<Node> AgentCommon::BuildReplaceImpl( TreePtr<Node> keynode )
{
    ASSERT(keynode)("Unkeyed search-only agent seen in replace context");
    return DuplicateSubtree(keynode);   
}


TreePtr<Node> AgentCommon::DuplicateNode( TreePtr<Node> source,
                                          bool force_dirty ) const
{
    INDENT(" ");

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);

    bool source_dirty = engine->GetOverallMaster()->dirty_grass.find( source ) != engine->GetOverallMaster()->dirty_grass.end();
    if( force_dirty || // requested by caller
        source_dirty ) // source was dirty
    {
        //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
        engine->GetOverallMaster()->dirty_grass.insert( dest );
    }
    
    return dest;    
}                                                     


TreePtr<Node> AgentCommon::DuplicateSubtree( TreePtr<Node> source,
                                             TreePtr<Node> source_terminus,
                                             TreePtr<Node> dest_terminus ) const
{
    INDENT(" ");
    ASSERT( source );
    if( source_terminus )
        ASSERT( dest_terminus );
     // Under substitution, we should be duplicating a subtree of the input
    // program, which should not contain any special nodes
    ASSERT( !(dynamic_pointer_cast<SpecialBase>(source)) )
          ("Cannot duplicate special node ")(*source);
    
    // If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copes of source terminus (directly, without duplicating).
    if( source_terminus && source == source_terminus ) 
        return dest_terminus;

    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( source, false );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > keynode_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Duplicating %d members source=", dest_memb.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
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
                ASSERT( p ); // present simplified scheme disallows NULL
                //TRACE("Duplicating ")(*p)("\n");
                TreePtr<Node> n = DuplicateSubtree( p, source_terminus, dest_terminus );
                //TRACE("Normal element, inserting ")(*n)(" directly\n");
                dest_con->insert( n );
            }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_ptr)("\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("source should be non-NULL");
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


