#include "soft_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

deque<Agent *> SoftAgent::PatternQuery() const
{
	deque<Agent *> child_agents;
	deque< TreePtr<Node> > patterns = MyPatternQuery();
    FOREACH( const TreePtr<Node> p, patterns )
		child_agents.push_back( AsAgent(p) );
	return child_agents;
}


void SoftAgent::AgentConfigure( const Engine *e )
{
    AgentCommon::AgentConfigure( e ); // To main version of Configure
    MyConfigure();
}


bool SoftAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("%");

    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    bool result = MyCompare( x );
    return result;
}


TreePtr<Node> SoftAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("%");
    if( !GetCoupled() )
    {
        // Call the soft pattern impl 
        keynode = MyBuildReplace();
        DoKey( keynode );
    }
    // Note that the keynode could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keynode);   
}


deque< TreePtr<Node> > SoftAgent::MyPatternQuery() const
{
	return deque< TreePtr<Node> >();
}


// Soft nodes should override this to implement their comparison function
bool SoftAgent::MyCompare( const TreePtrInterface &x ) const
{
    // Seems reasonable to allow a soft node with no compare impl to just 
    // always match.
    return true;
}


TreePtr<Node> SoftAgent::MyBuildReplace()
{
    return TreePtr<Node>(); 
}


// Compare for child nodes in a normal context (i.e. in which the pattern must match
// for an overall match to be possible, and so can be used to key a coupling)
bool SoftAgent::NormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern ) const
{
    // Local because soft nodes can "manufacture" subtrees to compare - so no PointerIs under here
    RememberLocalLink( false, AsAgent(pattern), x );    
    return true;
}


// Compare for child nodes in an abnormal context (i.e. in which the pattern need not match
// for an overall match to be possible, and so cannot be used to key a coupling)
bool SoftAgent::AbnormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern ) const
{
    RememberLocalLink( true, AsAgent(pattern), x );    
    return true;
}


TreePtr<Node> *SoftAgent::GetContext() const
{
    return engine->GetOverallMaster()->pcontext;
}


TreePtr<Node> SoftAgent::GetPatternCoupled( TreePtr<Node> pattern )
{
    return AsAgent(pattern)->GetCoupled();
}


TreePtr<Node> SoftAgent::DoBuildReplace( TreePtr<Node> pattern ) 
{
    return AsAgent(pattern)->BuildReplace();
}
