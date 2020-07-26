#ifndef NORMALITY_AGENT_WRAPPER_HPP
#define NORMALITY_AGENT_WRAPPER_HPP

#include "agent.hpp"
#include "engine.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

class NormalityAgentWrapper : public virtual Agent
{
	NormalityAgentWrapper( Agent *agent ); 
	void Configure( const Set<Agent *> &engine_agents, // Agents of the enclosing engine
	                const Set<Agent *> &master_agents, // Agents of the enclosing engine's master
	                const Engine *master );
	void SetMasterKeys( const CouplingMap &keys ); // Keys of the enclosing engine's master (call this at top of engine Compare())
    virtual PatternQueryResult PatternQuery() const;
    virtual void DecidedQuery( QueryAgentInterface &query,
                               const TreePtrInterface *px ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    shared_ptr<ContainerInterface> GetVisibleChildren() const;
    
    // Holder for info beyond what PatternQuery provides (the agent) for abnormal links.     
    struct AbnormalLink
    {
		AbnormalLink() : engine(false) {} // Engine is compare, not search
		
		// An engine instance to do the comparing within the context,
		Engine engine;
		// A set of terminal couplings which are couplings into the master's normal
        // context, and which terminate the abnormal context.		
        Set< Agent * > terminal_agents;
	};
    
    // The agent we are wrapping. Typically we will wrap all agents with >0 
    // abnormal links.
    Agent * const wrapped_agent;    
    
    // Additional info for abnormal links, in same order as Pattern/DecidedQuery
    list< shared_ptr<AbnormalLink> > abnormal_links;
    
    // Map of keys
    CouplingMap master_keys;
    shared_ptr<BooleanEvaluator> evaluator;
    
    // Specialisation of unique tree walk that will collect together our terminator agents.
	class TerminalFinder_iterator : public UniqueWalk::iterator
	{
	public:
		TerminalFinder_iterator( const Node *root, const Set<Agent *> &ea, const Set<Agent *> &ma, Set< Agent * > &ta ) : 
		    UniqueWalk::iterator(root),
		    const Set<Agent *> &engine_agents(ea),
		    const Set<Agent *> &master_agents(ma),
		    Set< Agent * > &terminal_agents(ta)
		{}        
		VisibleWalk_iterator() : UniqueWalk::iterator() {}
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	private:
	    const Set<Agent *> &engine_agents;
	    const Set<Agent *> &master_agents;
	    Set< Agent * > &terminal_agents;
		virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const;
	};
	typedef ContainerFromIterator< TerminalFinder_iterator, const Node *, const Set<Agent *> &, Set< Agent * > & > TerminalFinder;    
};

};

#endif
