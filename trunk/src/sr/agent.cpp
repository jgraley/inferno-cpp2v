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
    current_query(IDLE)
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


PatternLinks AgentCommon::PatternQuery() const
{
    ASSERT(this);
    ASSERT(engine)("Agent ")(*this)(" at appears not to have been configured");
	ASSERT( current_query == IDLE );
	current_query = PATTERN;

    // choices are read by the impl; links are updated by the impl
    pattern_links.clear();
        
    // Determine how agent links to other agents
    PatternQueryImpl();
        
    // Note that if the DecidedCompareImpl() already keyed, then this does nothing.
    current_query = IDLE;
    return pattern_links;
}


Links AgentCommon::DecidedQuery( const TreePtrInterface &x,
                                 deque<ContainerInterface::iterator> ch ) const
{
    ASSERT(this);
    ASSERT(engine)("Agent ")(*this)(" at appears not to have been configured");
	ASSERT( current_query == IDLE );
	current_query = DECIDED;

    // choices are read by the impl; links are updated by the impl
    links.clear();
    choices = ch;
    
    // Do the agent-specific local checks (x versus characteristics of the present agent)
    // Also takes notes of how child agents link to children of x (depending on conjecture)
    links.local_match = DecidedQueryImpl( x );
        
    // Note that if the DecidedCompareImpl() already keyed, then this does nothing.
    current_query = IDLE;
    return links;
}


shared_ptr<ContainerInterface> AgentCommon::GetVisibleChildren() const
{
	// Normally all children should be visible 
   typedef ContainerFromIterator< FlattenNode_iterator, const Node * > FlattenNodePtr;
   return shared_ptr<ContainerInterface>( new FlattenNodePtr(this) );
    // Note: a pattern query should be just as good...
}

    
void AgentCommon::DoKey( TreePtr<Node> x )
{
    ASSERT(x);
    if( !coupling_key )
    {
        coupling_key = x;
    }
}


TreePtr<Node> AgentCommon::GetCoupled()
{
    return coupling_key; 
}


void AgentCommon::ResetKey()
{
    coupling_key = TreePtr<Node>();
}


void AgentCommon::RememberLink( bool abnormal, Agent *a ) const
{
	ASSERT( current_query==PATTERN );
    PatternLinks::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    TRACE("Remembering link %d ", pattern_links.links.size())(*a)(abnormal?" abnormal":" normal")("\n");
    pattern_links.links.push_back( l );
}


void AgentCommon::RememberLink( bool abnormal, Agent *a, const TreePtrInterface &x ) const
{
	ASSERT( current_query==DECIDED );
    Links::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    l.px = &x;
    l.local_x = TreePtr<Node>();
    TRACE("Remembering link %d ", links.links.size())(*a)(" -> ")(*x)(abnormal?" abnormal":" normal")("\n");
    links.links.push_back( l );
}


void AgentCommon::RememberLink( const Links::Link &l ) const
{
    links.links.push_back( l );
}

    
void AgentCommon::RememberLink( const PatternLinks::Link &l ) const
{
    pattern_links.links.push_back( l );
}

    
void AgentCommon::RememberLocalLink( bool abnormal, Agent *a, TreePtr<Node> x ) const
{
	ASSERT( current_query==DECIDED );
    ASSERT(x);
    Links::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    l.px = NULL;    
    l.local_x = x;
    TRACE("Remembering local link %d ", links.links.size())(*a)(" -> ")(*x)(abnormal?" abnormal":" normal")("\n");
    links.links.push_back( l );
}


void AgentCommon::RememberEvaluator( shared_ptr<BooleanEvaluator> e ) const
{
	ASSERT( current_query!=IDLE );
	if( current_query==PATTERN )
	{
		ASSERT( !pattern_links.evaluator ); // should not register more than one
		pattern_links.evaluator = e;
	}
	else
	{
		ASSERT( !links.evaluator ); // should not register more than one
	    links.evaluator = e;
	}
}	


ContainerInterface::iterator AgentCommon::HandleDecision( ContainerInterface::iterator begin,
                                                          ContainerInterface::iterator end ) const
{
    ASSERT( begin != end )("no empty decisions");
    ContainerInterface::iterator it;
    if( choices.empty() )
    {
        it = begin; // No choice was given to us so assume first one
    }
    else
    {
        it = choices.front(); // Use and consume the choice that was given to us
        ASSERT( it != end );
        choices.pop_front();
    }
    
    Conjecture::Range r;
    r.begin = begin;
    r.end = end;
    links.decisions.push_back(r); // Report the range back
        
    return it;
}


ContainerInterface::iterator AgentCommon::RememberDecisionLink( bool abnormal, 
																Agent *a, 
																ContainerInterface::iterator begin,
																ContainerInterface::iterator end ) const
{
	ASSERT( current_query==DECIDED );
    ASSERT( begin != end )("no empty decisions");
    ContainerInterface::iterator it;
    if( choices.empty() )
    {
        it = begin; // No choice was given to us so assume first one
    }
    else
    {
        it = choices.front();  // Use and consume the choice that was given to us
        ASSERT( it != end );
        choices.pop_front();
    }
    
    Links::Link l;
    l.abnormal = abnormal;
    l.agent = a;
    l.px = &(*it); // do not simplify! we want a simple pointer, not an iterator TODO or do we
    l.local_x = TreePtr<Node>();
    TRACE("Remembering decision link %d ", links.links.size())(*a)(" -> ")(**it)(abnormal?" abnormal":" normal")("\n");

    Conjecture::Range r;
    r.begin = begin;
    r.end = end;

    // Put it all in links TODO tie these together in the links struct
    links.links.push_back( l );    
    links.decisions.push_back(r); 
    
    return it; // Note: we have to have the iterator even when a coupling push has occurred, since
               // we should have checked that the pushed back node is actually in the container 
               // (find() etc gets us an iterator)
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
        
    return false; // equal
}


void AgentCommon::KeyReplace( const TreePtrInterface &x,
                              deque<ContainerInterface::iterator> choices )
{   
    DoKey(x);  
}


void AgentCommon::TrackingKey( Agent *from )
{
    // This function is called on nodes under the "overlay" branch of Overlay nodes.
    // Some special nodes will not know what to do...
    ASSERT(GetCoupled())(*this)(" cannot appear in a replace-only context");
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
                //TRACE("inserting ")(*n)(" directly\n");
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



