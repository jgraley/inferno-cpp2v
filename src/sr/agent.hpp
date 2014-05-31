#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"
#include <set>
#include <boost/type_traits.hpp>
 
class Conjecture;
class SpecialBase;
class StuffBase;
class StarBase;
class SlaveBase;
class SearchContainerBase;
class SearchReplace;
 
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
								  TreePtr<Node> pattern,
								  bool can_key,
							      Conjecture &conj ) const = 0;
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> pattern, 
			 					         TreePtr<Node> keynode=TreePtr<Node>() ) const = 0;
	virtual void Configure( const CompareReplace *s, CouplingKeys *c ) = 0;
	static Agent *AsAgent( TreePtr<Node> node )
	{
		ASSERT( node )("Called AsAgent(")(node)(") with NULL TreePtr");
		Agent *agent = dynamic_cast<Agent *>(node.get());
		ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
		return agent;
	}
};



class NormalAgent : public virtual Agent
{
public:
	NormalAgent() :sr(NULL), coupling_keys(NULL) {}
    void ConfigureTreePtrThis( TreePtr<Node> tpt );										 
	void Configure( const CompareReplace *s, CouplingKeys *c );
private:
	const CompareReplace *sr;
	CouplingKeys *coupling_keys;
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
    		                     TreePtr<Node> pattern,
    		                     bool can_key,
    		                     Conjecture &conj ) const;
private:
    bool DecidedCompare( SequenceInterface &x,
    		             SequenceInterface &pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
    bool DecidedCompare( CollectionInterface &x,
    		             CollectionInterface &pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
    bool DecidedCompare( const TreePtrInterface &x,
    		             shared_ptr<SearchContainerBase> pattern,
    		             bool can_key,
    		             Conjecture &conj ) const;
public:	
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> pattern, 
								         TreePtr<Node> keynode=TreePtr<Node>() ) const;
private:	
	TreePtr<Node> BuildReplaceOverlay( TreePtr<Node> pattern, 
									    TreePtr<Node> keynode ) const; // under substitution if not NULL
    TreePtr<Node> BuildReplaceSlave( shared_ptr<SlaveBase> pattern, 
									  TreePtr<Node> keynode ) const;    
    TreePtr<Node> BuildReplaceNormal( TreePtr<Node> pattern ) const;
    TreePtr<Node> BuildReplaceKeyed( TreePtr<Node> pattern, 
									  TreePtr<Node> keynode ) const;
    TreePtr<Node> BuildReplaceStar( shared_ptr<StarBase> pattern, 
									 TreePtr<Node> keynode ) const;
									 
private:
    Sequence<Node> WalkContainerPattern( ContainerInterface &pattern,
                                         bool replacing ) const;
	// Internal node classes - NOTE these are not special nodes, and we use them like normal tree nodes
	// Note: only needed for Star pattern
    struct SubContainer : Node
    {
        NODE_FUNCTIONS
    };
    struct SubSequenceRange : SequenceInterface,
                              SubContainer
    {
    	NODE_FUNCTIONS_FINAL 

        SubSequenceRange() {}
    	shared_ptr<iterator_interface> my_begin;
    	shared_ptr<iterator_interface> my_end;
    	operator string() const { return GetName() + SSPrintf("@%p", this); }    
    public:
    	SubSequenceRange( iterator &b, iterator &e ) : my_begin(b.Clone()), my_end(e.Clone())
    	{    	    
    	}
	    virtual const iterator_interface &begin() { return *my_begin; }
        virtual const iterator_interface &end()   { return *my_end; }
        virtual void erase( iterator )  { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual void clear()                                { ASSERTFAIL("Cannot modify SubSequenceRange"); }    
        virtual void insert( const TreePtrInterface & )     { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual TreePtrInterface &operator[]( int i ) { ASSERTFAIL("TODO"); }  
        virtual void push_back( const TreePtrInterface &gx ){ ASSERTFAIL("Cannot modify SubSequenceRange"); }  
    };
    struct SubSequence : Sequence<Node>,
                         SubContainer
    {
    	NODE_FUNCTIONS_FINAL 
    	operator string() const { return GetName() + SSPrintf("@%p", this); }    
    };
    struct SubCollection : Collection<Node>,
                           SubContainer
    {
    	NODE_FUNCTIONS_FINAL
    	operator string() const { return GetName() + SSPrintf("@%p", this); } 
    };

};


#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  
template<typename NODE_TYPE>
class NormalAgentWrapper : public virtual NODE_TYPE,
                           public virtual NormalAgent
{
public:
    NormalAgentWrapper() : NODE_TYPE() {}

	template<typename CP0>
    NormalAgentWrapper(const CP0 &cp0) : NODE_TYPE(cp0) {}

	template<typename CP0, typename CP1>
    NormalAgentWrapper(const CP0 &cp0, const CP1 &cp1) : NODE_TYPE(cp0, cp1) {}

	template<typename CP0, typename CP1, typename CP2>
    NormalAgentWrapper(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : NODE_TYPE(cp0, cp1, cp2) {}
};


// Similar to MakeTreePtr<> (see node/specialise_oostd.hpp) but produces a TreePtr to NormalAgentWrapper<NODE_TYPE> rather
// than just NODE_TYPE when NODE_TYPE is not already a kind of Agent. 
template<typename NODE_TYPE>
class MakePatternPtr : public TreePtr<NODE_TYPE>
{
private:
    // Using the magic of Boost, find out at compile time whether the NODE_TYPE is already an Agent.	
	static const bool NODE_IS_AGENT = is_base_of<Agent, NODE_TYPE>::value;
public:	
	MakePatternPtr() : TreePtr<NODE_TYPE>( NODE_IS_AGENT ? 
	                                       new NODE_TYPE : // Already an agent, so behave like MakeTreePtr
										   new NormalAgentWrapper<NODE_TYPE> ) // not an agent, so wrap. Ultimately this will only happen to normal nodes
	{
	}
	template<typename CP0>
	MakePatternPtr(const CP0 &cp0) : TreePtr<NODE_TYPE>( NODE_IS_AGENT ?  
	                                                     new NODE_TYPE(cp0) :
	                                                     new NormalAgentWrapper<NODE_TYPE>(cp0) ) 
	{ 
	}
    template<typename CP0, typename CP1>
	MakePatternPtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<NODE_TYPE>(  NODE_IS_AGENT ?  
	                                                                      new NODE_TYPE(cp0, cp1) :
	                                                                      new NormalAgentWrapper<NODE_TYPE>(cp0, cp1) )
	{ 
	}
	template<typename CP0, typename CP1, typename CP2>
	MakePatternPtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<NODE_TYPE>( NODE_IS_AGENT ? 	 
	                                                                                     new NODE_TYPE(cp0, cp1, cp2) :
	                                                                                     new NormalAgentWrapper<NODE_TYPE>(cp0, cp1, cp2) )
	{ 
	}
	// Add more params as needed...
};

#endif
