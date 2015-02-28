#include "soft_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

void SoftAgent::AgentConfigure( const CompareReplace *s, CouplingKeys *c )
{
    AgentCommon::AgentConfigure( s, c ); // To main version of Configure
    MyConfigure();
}


bool SoftAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                    bool can_key,
                                    Conjecture &conj )
{
    ASSERT( !current_conj )("DecidedCompare() recursion detected in soft node");
    current_can_key = can_key;
    current_conj = &conj;   
    bool result = MyCompare( x );
    current_conj = NULL;
    return result;
}

    
void SoftAgent::KeyReplace()
{    
    TRACE("Keying replace pattern (via agent) ")(*this)("\n");

    // Call the soft pattern impl 
    TreePtr<Node> key = MyBuildReplace();
    if( key )           
    {            
        // Allow this to key a coupling. 
        DoKey( key );
    } 
}


TreePtr<Node> SoftAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERT(keynode)("Soft pattern seen in tree but it produced no keynode or overlay");
    ASSERT(sr);
    return DuplicateSubtree(keynode);   
}


// Soft nodes should override this to implement their comparison function
bool SoftAgent::MyCompare( const TreePtrInterface &x )
{
    return true;
}


TreePtr<Node> SoftAgent::MyBuildReplace()
{
    return TreePtr<Node>(); // default implementation for weak modifiers 
                            // so that couplings appear to override local functionality
}


// Compare for child nodes in a normal context (i.e. in which the pattern must match
// for an overall match to be possible, and so can be used to key a coupling)
bool SoftAgent::NormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    ASSERT( current_conj )("Cannot call NormalCompare() from other than MyCompare()");
    return AsAgent(pattern)->DecidedCompare( x, current_can_key, *current_conj );
}


// Compare for child nodes in an abnormal context (i.e. in which the pattern need not match
// for an overall match to be possible, and so cannot be used to key a coupling)
bool SoftAgent::AbnormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    if( current_can_key )
        return true; // Since we must not key, skip the keying phase, letting all the normal nodes key themselves
    else
        return AsAgent(pattern)->Compare( x, false, current_conj ); 
}


TreePtr<Node> *SoftAgent::GetContext()
{
    return sr->pcontext;
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
