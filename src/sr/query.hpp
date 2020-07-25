#ifndef QUERY_HPP
#define QUERY_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "boolean_evaluator.hpp"
#include <vector>
#include <boost/type_traits.hpp>
  
namespace SR
{ 
class Agent;

class PatternQueryResult 
{
public:    
    struct Block 
    {
        bool abnormal;
        Agent *agent;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
        
    void clear()
    {
		blocks.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
    void AddLink( bool abnormal, Agent *a );
    void AddEvaluator( shared_ptr<BooleanEvaluator> e );
    
    const list<Block> *GetBlocks() const { return &blocks; } // pointer returned because the blocks contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }

private:
    list<Block> blocks;
    shared_ptr<BooleanEvaluator> evaluator;
};


class AgentQuery 
{
public:    
    struct Range
    {
        ContainerInterface::iterator begin;
        ContainerInterface::iterator end;    
        bool inclusive; // If true, include "end" as a possible choice
        std::shared_ptr<ContainerInterface> container; // Only needed if the container is not part of the x tree
        bool operator==(const Range &o) const // Only required for an ASSERT
        {
			return begin==o.begin && end==o.end && inclusive==o.inclusive && container==o.container;
		}
    };
    
    // There is a "random access" in Conjecture::IncrementAgent() not sure if true any more
    typedef vector<Range> Ranges;

    // There is a "random access" in AddDecision() not sure if true any more
    typedef vector<ContainerInterface::iterator> Choices; 

    struct Block 
    {
        const TreePtrInterface *GetPX() const
        {
			if( px )
				return px; // linked x is in input tree
			else
				return &local_x; // linked x is local, kept alive by local_x    
		}	

        bool is_link;
        bool abnormal;
        Agent *agent;
        const TreePtrInterface *px;
        TreePtr<Node> local_x;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
    
    AgentQuery() :
        next_decision( decisions.begin() ), // will be end()
        next_choice( choices.begin() ) // will be end()
    {
    }
    void AddLink( bool abnormal, Agent *a, const TreePtrInterface *px ); 
    void AddLocalLink( bool abnormal, Agent *a, TreePtr<Node> x ); 
    void AddEvaluator( shared_ptr<BooleanEvaluator> e ); 
    ContainerInterface::iterator AddDecision( ContainerInterface::iterator begin,
                                              ContainerInterface::iterator end,
                                              bool inclusive,
                                              const Choices &choices,
                                              std::shared_ptr<ContainerInterface> container=nullptr );
    void AddLocalMatch( bool local_match );
                                                  
    const list<Block> *GetBlocks() const { return &blocks; } // pointer returned because the blocks contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }
    bool IsLocalMatch() { return local_match; }
      
    const Choices *GetChoices() { return &choices; }
    const Ranges *GetDecisions() { return &decisions; }
    void InvalidateBack();
    void SetBackChoice( ContainerInterface::iterator newc );
    void PushBackChoice( ContainerInterface::iterator newc );    
    void PopulateDecisions();
    ContainerInterface::iterator AddDecision( ContainerInterface::iterator begin,
                                              ContainerInterface::iterator end,
                                              bool inclusive,
                                              std::shared_ptr<ContainerInterface> container=nullptr );
    ContainerInterface::iterator AddDecision( const Range &d );
    ContainerInterface::iterator AddDecision( std::shared_ptr<ContainerInterface> container, bool inclusive );
    //ContainerInterface::iterator AddDecision( const ContainerInterface &container, bool inclusive );
    bool IsAlreadyGotNextOldDecision();
    const Range &GetNextOldDecision();
    ContainerInterface::iterator AddNextOldDecision();
    void Reset();
    
private:
    list<Block> blocks; 
    shared_ptr<BooleanEvaluator> evaluator;
    bool local_match = true;
    Choices choices;
    Ranges decisions;
    
    Ranges::iterator next_decision;
    Choices::iterator next_choice;
};


bool operator<(const AgentQuery::Block &l0, const AgentQuery::Block &l1);
};
#endif