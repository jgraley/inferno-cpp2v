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
    sr(NULL)
{
}


void AgentCommon::AgentConfigure( const CompareReplace *s )
{
    // Repeat configuration regarded as an error because it suggests I maybe don't
    // have a clue what should actaually be configing the agent. Plus general lifecycle 
    // rule enforcement.
    // Why no coupling across sibling slaves? Would need an ordering for keying
    // but ordering not defined on sibling slaves.
    ASSERT(!sr)("Detected repeat configuration of ")(*this)
               ("\nCould be result of coupling this node across sibling slaves - not allowed :(");
    ASSERT(s);
    sr = s;
}


bool AgentCommon::DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj )
{
    INDENT;
    ASSERT(sr)("Agent ")(*this)(" at appears not to have been configured, since sr is NULL");
    ASSERT( x ); // Target must not be NULL
    ASSERT(this);
    
    // Check whether the present node matches. Do this for all nodes: this will be the local
    // restriction for normal nodes and the pre-restriction for special nodes (based on
    // how IsLocalMatch() has been overridden.
    if( !IsLocalMatch(x.get()) )
    {
        return false;
    }
    
    bool match = DecidedCompareImpl( x, can_key, conj );
    if( !match )
        return false;
    
    if( TreePtr<Node> keynode = GetCoupled() )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
      
    // Note that if the DecidedCompareImpl() already keyed, then this does nothing.
    if( can_key )
    {
        DoKey( x );  
    }

    return true;
}


bool AgentCommon::DecidedCompareImpl( const TreePtrInterface &x,
                                      bool can_key,
                                      Conjecture &conj )
{
    ASSERTFAIL("Agents must overload one of DecidedCompare() or DecidedCompareImpl()");
}


bool AgentCommon::Compare( const TreePtrInterface &x,
                           bool can_key,
                           Conjecture *in_conj  ) 
{
    INDENT("C");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*this);
    TRACE(" can_key=%d \n", (int)can_key);
    //TRACE(**pcontext)(" @%p\n", pcontext);
    
    //if( can_key == false )
    //{
    //    ASSERT(in_conj);
        //return DecidedCompare( x, false, *in_conj );
   // }
        
    
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj;
    bool r;
    while(1)
    {
        // Try out the current conjecture. This will call HandlDecision() once for each decision;
        // HandleDecision() will return the current choice for that decision, if absent it will
        // add the decision and choose the first choice, if the decision reaches the end it
        // will remove the decision.
        r = true;

        // Only key if the keys are already set to KEYING (which is 
        // the initial value). Keys could be RESTRICTING if we're under
        // a SoftNot node, in which case we only want to restrict.
        if( can_key )
        {
            // Unkey 
            FOREACH( Agent *a, sr->my_agents )
                a->ResetKey();

            // Do a two-pass matching process: first get the keys...
            TRACE("doing KEYING pass....\n");
            conj.PrepareForDecidedCompare();
            r = DecidedCompare( x, true, conj );
            TRACE("KEYING pass result %d\n", r );
        }
               
        if( r )
        {
            // ...now restrict the search according to the couplings
            TRACE("doing RESTRICTING pass....\n");
            conj.PrepareForDecidedCompare();
            r = DecidedCompare( x, false, conj );
            TRACE("RESTRICTING pass result %d\n", r );
        }
        
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
    INDENT;
    ASSERT(this);
    ASSERT(sr)("Agent ")(*this)(" at appears not to have been configured, since sr is NULL");
    
    // See if the pattern node is coupled to anything. The keynode that was passed
    // in is just a suggestion and will be overriden if we are keyed.
    TreePtr<Node> keynode = GetCoupled();
    
    return BuildReplaceImpl( keynode );    
}


TreePtr<Node> AgentCommon::DuplicateNode( TreePtr<Node> source,
                                          bool force_dirty ) const
{
    INDENT;

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);

    bool source_dirty = sr->GetOverallMaster()->dirty_grass.find( source ) != sr->GetOverallMaster()->dirty_grass.end();
    if( force_dirty || // requested by caller
        source_dirty ) // source was dirty
    {
        //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
        sr->GetOverallMaster()->dirty_grass.insert( dest );
    }
    
    return dest;    
}                                                     


TreePtr<Node> AgentCommon::DuplicateSubtree( TreePtr<Node> source,
                                             TreePtr<Node> source_terminus,
                                             TreePtr<Node> dest_terminus ) const
{
    INDENT;
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


