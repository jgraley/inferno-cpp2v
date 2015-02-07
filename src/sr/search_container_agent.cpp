#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

bool SearchContainerAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                               bool can_key,
                                               Conjecture &conj )
{
    INDENT;
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( x );
    
    // Get choice from conjecture about where we are in the walk
    ContainerInterface::iterator thistime = conj.HandleDecision( pwx->begin(), pwx->end() );
    if( thistime == (ContainerInterface::iterator)(pwx->end()) )
        return false; // ran out of choices

    // Try out comparison at this position
    TRACE("Trying terminus ")(**thistime);
    bool r = Agent::AsAgent(terminus)->DecidedCompare( *thistime, can_key, conj );
    if( !r )
        return false;
        
    if( TreePtr<Node> keynode = coupling_keys->GetCoupled( this ) )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
    
    // If we got this far, do the couplings
    if( can_key )
    {
        shared_ptr<TerminusKey> key( new TerminusKey );
        key->root = x;
        key->terminus = *thistime;
        shared_ptr<Key> sckey( key );
        TRACE("Matched, so keying search container type ")(*this)(" for ")(*x);
        coupling_keys->DoKey( sckey, this );    
    }
    return r;
}


TreePtr<Node> SearchContainerAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    // SearchContainer.
    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
    // so come out of substitution. Done as tail recursion so that we already duplicated
    // the terminus key, and can just overlay the terminus replace pattern.
    shared_ptr<Key> key = coupling_keys->GetKey( this );
    shared_ptr<TerminusKey> stuff_key = dynamic_pointer_cast<TerminusKey>(key);
    ASSERT( stuff_key );
    ASSERT( stuff_key->agent );
    TerminusBase *replace_stuff = dynamic_cast<TerminusBase *>( stuff_key->agent );
    ASSERT( replace_stuff );
    ASSERT( replace_stuff->terminus );
    TRACE( "Stuff node: Duplicating at terminus first: keynode=")(*(replace_stuff->terminus))
                                                        (", term=")(*(stuff_key->terminus))("\n");
    TreePtr<Node> term = AsAgent(replace_stuff->terminus)->BuildReplace( stuff_key->terminus );
    TRACE( "Stuff node: Substituting stuff");
    return sr->DuplicateSubtree(keynode, stuff_key->terminus, term);   
}


StuffAgent::StuffAgent() : 
    recurse_comparer( new CompareReplace ),
    one_level(false)
{
}


shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( TreePtr<Node> x )
{
    // TODO cache this?
    Filter *rf = NULL;
    if( recurse_restriction )
    {
        ASSERT( recurse_comparer->compare_pattern )("Stuff node in slave must be initialised before slave\n");     
        rf = recurse_comparer;
    }
    
    if( one_level )
        return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
    else
        return shared_ptr<ContainerInterface>( new Walk( x, NULL, rf ) );
}


shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( TreePtr<Node> x )
{ 
    TRACE("FlattenNodeing an AnyNode at ")(*x)(": { ");
    FlattenNode f( x );
    FOREACH( TreePtr<Node> pn, f )
        {TRACE(*pn)(" ");}
    TRACE("}\n");
        
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}

