#include "soft_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

void SoftAgent::AgentConfigure( const Engine *e )
{
    AgentCommon::AgentConfigure( e ); // To main version of Configure
    MyConfigure();
}


bool SoftAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                    bool can_key,
                                    Conjecture &conj )
{
    INDENT("%");

    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    ASSERT( !current_conj )("DecidedCompare() recursion detected in soft node");
    current_can_key = can_key;
    current_conj = &conj;   
    bool result = MyCompare( x );
    current_conj = NULL;
    return result;
}

    
void SoftAgent::KeyReplace()
{    
    INDENT("%");
    TRACE("Keying replace pattern (via agent) ")(*this)("\n");

    if( !GetKey() )
    {
        // Call the soft pattern impl 
        TreePtr<Node> key = MyBuildReplace();
        if( key )           
        {            
            // Allow this to key a coupling. 
            DoKey( key );
        } 
    }
}


TreePtr<Node> SoftAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("%");
    // Note that the keynode could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keynode);   
}


// Soft nodes should override this to implement their comparison function
bool SoftAgent::MyCompare( const TreePtrInterface &x )
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
bool SoftAgent::NormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    ASSERT( current_conj )("Cannot call NormalCompare() from other than MyCompare()");
    // Local because soft nodes can "manufacture" subtrees to compare - so no PointerIs under here
    RememberLocalLink( false, AsAgent(pattern), x );    
    return true;
}


// Compare for child nodes in an abnormal context (i.e. in which the pattern need not match
// for an overall match to be possible, and so cannot be used to key a coupling)
bool SoftAgent::AbnormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    ASSERT( current_conj )("Cannot call AbnormalCompare() from other than MyCompare()");
    RememberLocalLink( true, AsAgent(pattern), x );    
    return true;
}


TreePtr<Node> *SoftAgent::GetContext()
{
    return engine->GetOverallMaster()->pcontext;
}


bool SoftAgent::IsCanKey()
{
    return current_can_key;
}


TreePtr<Node> SoftAgent::GetCoupled( TreePtr<Node> pattern )
{
    return AsAgent(pattern)->GetCoupled();
}


TreePtr<Node> SoftAgent::DoBuildReplace( TreePtr<Node> pattern ) 
{
    return AsAgent(pattern)->BuildReplace();
}
