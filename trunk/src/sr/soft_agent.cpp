#include "soft_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

void SoftAgentCommon::Configure( const CompareReplace *s, CouplingKeys *c )
{
    AgentCommon::Configure( s, c ); // To main version of Configure
    MyConfigure();
}

    
void SoftAgentCommon::KeyReplace()
{    
    TRACE("Keying replace pattern (via agent) ")(*this)("\n");

    // Call the soft pattern impl 
    TreePtr<Node> key = MyBuildReplace();
    if( key )           
    {            
        // Allow this to key a coupling. 
        coupling_keys->DoKey( key, this );
    } 
}


TreePtr<Node> SoftAgentCommon::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    TreePtr<Node> overlay = GetOverlayPattern(); // only strong modifiers use this
    if( overlay ) // Really two different kinds of pattern node
        return AsAgent(overlay)->BuildReplace( keynode ); // Strong modifier
    else
    {
        ASSERT(keynode)("Soft pattern seen in tree but it produced no keynode or overlay");
        ASSERT(sr);
        return DuplicateSubtree(keynode);   // Weak modifier
    }
}


// Called when coupled, dest is coupling key TODO route through "my" version
TreePtr<Node> SoftAgentCommon::GetOverlayPattern() 
{ 
    return TreePtr<Node>(); // default implementation for weak modifiers 
                            // so that couplings appear to override local functionality
}   


TreePtr<Node> SoftAgentCommon::MyBuildReplace()
{
    return TreePtr<Node>(); // default implementation for weak modifiers 
                            // so that couplings appear to override local functionality
}


// Compare for child nodes in a normal context (i.e. in which the pattern must match
// for an overall match to be possible, and so can be used to key a coupling)
bool SoftAgentCommon::NormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    ASSERT( current_conj )("Cannot call NormalCompare() from other than MyCompare()");
    return Agent::AsAgent(pattern)->DecidedCompare( x, current_can_key, *current_conj );
}


// Compare for child nodes in an abnormal context (i.e. in which the pattern need not match
// for an overall match to be possible, and so cannot be used to key a coupling)
bool SoftAgentCommon::AbnormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern )
{
    return Agent::AsAgent(pattern)->Compare( x, false ); 
}


TreePtr<Node> *SoftAgentCommon::GetContext()
{
    return sr->pcontext;
}


bool SoftAgentCommon::IsCanKey()
{
    return current_can_key;
}


TreePtr<Node> SoftAgentCommon::GetCoupled( TreePtr<Node> pattern )
{
    return coupling_keys->GetCoupled( Agent::AsAgent(pattern) );
}


CouplingKeys *SoftAgentCommon::GetCouplingKeys()
{
    return coupling_keys;
}


TreePtr<Node> SoftAgentCommon::DoBuildReplace( TreePtr<Node> pattern ) 
{
    return Agent::AsAgent(pattern)->BuildReplace( pattern );
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


bool SoftAgentSpecialKey::DecidedCompareImpl( const TreePtrInterface &x,
                                              bool can_key,
                                              Conjecture &conj )
{
    // Hand over to any soft search functionality in the search pattern node
    ASSERT( !current_conj )("DecidedCompare() recursion detected in soft node");
    current_can_key = can_key;
    current_conj = &conj;   
    shared_ptr<Key> special_key = MyCompare( x );
    current_conj = NULL;
    if( special_key )
    {
        coupling_keys->DoKey( special_key, this ); 
        return true;
    }
    else
        return false;
}


// Soft nodes should override this to implement their comparison function
bool SoftAgent::MyCompare( const TreePtrInterface &x )
{
    return true;
}
