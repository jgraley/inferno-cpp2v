#ifndef NORMAL_AGENT_HPP
#define NORMAL_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include <set>
#include <boost/type_traits.hpp>
 
namespace SR
{ 

/// Agent implementation used for all normal pattern nodes    
class NormalAgent : public virtual AgentCommon
{
public:
    virtual deque<Agent *> PatternQuery() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
private:
    bool DecidedQuerySequence( SequenceInterface &x,
    	                       SequenceInterface &pattern ) const;
    bool DecidedQueryCollection( CollectionInterface &x,
    		                     CollectionInterface &pattern ) const;
public:
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
private:	
	TreePtr<Node> BuildReplaceOverlay( TreePtr<Node> keynode ); // under substitution if not NULL
    TreePtr<Node> BuildReplaceNormal();
};


#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  
/// Wrapper for normal pattern nodes that combines a NormalAgent with the specified pattern node
template<typename NODE_TYPE>
class NormalAgentWrapper : public virtual NODE_TYPE,
                           public virtual NormalAgent
{
public:
    NormalAgentWrapper() : NODE_TYPE() {}

	template<typename CP0>
    NormalAgentWrapper(const CP0 &cp0) : 
        NODE_TYPE(cp0) {}

	template<typename CP0, typename CP1>
    NormalAgentWrapper(const CP0 &cp0, const CP1 &cp1) : 
        NODE_TYPE(cp0, cp1) {}

    template<typename CP0, typename CP1, typename CP2>
    NormalAgentWrapper(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : 
        NODE_TYPE(cp0, cp1, cp2) {}
    
    template<typename CP0, typename CP1, typename CP2, typename CP3>
    NormalAgentWrapper(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2, const CP2 &cp3) : 
        NODE_TYPE(cp0, cp1, cp2, cp3) {}
    
    virtual string GetTypeName() const // used by parse, render etc
    {
        // Want to get rid of the NormalAgentWrapper<...> around the name. The main
        // GetName() works via RTTI so no amount of casting of "this" will stop the full
        // final node's name being returned. So we have to actually make a temp in the
        // node type and use that.
        return NODE_TYPE().GetTypeName();
    }
};


/// Helper that does the constructing of pattern nodes when they are not already agents
template< bool IS_AGENT, typename NODE_TYPE >
class MakePatternPtrHelper  
{
public:    
    static inline TreePtr<NODE_TYPE> Make()
    {
        return new NormalAgentWrapper<NODE_TYPE>;
    }    
    template<typename CP0>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0)
    {
        return new NormalAgentWrapper<NODE_TYPE>(cp0);
    }    
    template<typename CP0, typename CP1>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1)
    {
        return new NormalAgentWrapper<NODE_TYPE>(cp0, cp1);
    }    
    template<typename CP0, typename CP1, typename CP2>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2)
    {
        return new NormalAgentWrapper<NODE_TYPE>(cp0, cp1, cp2);
    }    
    template<typename CP0, typename CP1, typename CP2, typename CP3>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2, const CP3 &cp3)
    {
        return new NormalAgentWrapper<NODE_TYPE>(cp0, cp1, cp2, cp3);
    }    
};


/// Helper that does the constructing of pattern nodes that are already agents
template<typename NODE_TYPE>
class MakePatternPtrHelper<true, NODE_TYPE> // NODE_TYPE is an agent, so behave like MakeTreePtr
{
public:
    static inline TreePtr<NODE_TYPE> Make()
    {
        return new NODE_TYPE;
    }    
    template<typename CP0>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0)
    {
        return new NODE_TYPE(cp0);
    }    
    template<typename CP0, typename CP1>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1)
    {
        return new NODE_TYPE(cp0, cp1);
    }    
    template<typename CP0, typename CP1, typename CP2>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2)
    {
        return new NODE_TYPE(cp0, cp1, cp2);
    }    
    template<typename CP0, typename CP1, typename CP2, typename CP3>
    static inline TreePtr<NODE_TYPE> Make(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2, const CP3 &cp3)
    {
        return new NODE_TYPE(cp0, cp1, cp2, cp3);
    }    
};


// Similar to MakeTreePtr<> (see node/specialise_oostd.hpp) but produces a TreePtr to NormalAgentWrapper<NODE_TYPE> rather
// than just NODE_TYPE when NODE_TYPE is not already a kind of Agent. 
/// Utility for constructing nodes that are to be used in patterns
template<typename NODE_TYPE>
class MakePatternPtr : public TreePtr<NODE_TYPE>
{
private:
    // Using the magic of Boost, find out at compile time whether the NODE_TYPE is already an Agent.	
	typedef MakePatternPtrHelper<is_base_of<Agent, NODE_TYPE>::value, NODE_TYPE> Maker;
public:	
	inline MakePatternPtr() : TreePtr<NODE_TYPE>( Maker::Make() )
	{
        TRACE("MakePatternPtr made a ")(*this)("\n");
	}
	template<typename CP0>
	inline MakePatternPtr(const CP0 &cp0) : 
	    TreePtr<NODE_TYPE>( Maker::Make(cp0) ) 
	{ 
        TRACE("MakePatternPtr made a ")(*this)("\n");
	}
    template<typename CP0, typename CP1>
	inline MakePatternPtr(const CP0 &cp0, const CP1 &cp1) : 
	    TreePtr<NODE_TYPE>(  Maker::Make(cp0, cp1) )
	{ 
        TRACE("MakePatternPtr made a ")(*this)("\n");
	}
    template<typename CP0, typename CP1, typename CP2>
    inline MakePatternPtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : 
        TreePtr<NODE_TYPE>( Maker::Make(cp0, cp1, cp2) )
    { 
        TRACE("MakePatternPtr made a ")(*this)("\n");
    }
    template<typename CP0, typename CP1, typename CP2, typename CP3>
    inline MakePatternPtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2, const CP3 &cp3) : 
        TreePtr<NODE_TYPE>( Maker::Make(cp0, cp1, cp2, cp3) )
    { 
        TRACE("MakePatternPtr made a ")(*this)("\n");
    }
	// Add more params as needed...
};

};

#endif
