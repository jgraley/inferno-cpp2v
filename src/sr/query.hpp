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


class DecidedQueryResult 
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
    
    // There is a "random access" in IncrementAgent()
    typedef vector<Range> Ranges;

    // There is a "random access" in DecidedQueryResult::AddDecision()
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
        bool is_decision;
        Range decision;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
        
    void clear()
    {
        blocks.clear();
        decision_count = 0;
        evaluator = shared_ptr<BooleanEvaluator>();        
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
    int GetDecisionCount() const { return decision_count; }
    
protected:
    list<Block> blocks; 
    shared_ptr<BooleanEvaluator> evaluator;
    bool local_match = true;
    int decision_count = 0;
};


class AgentQuery : public DecidedQueryResult
{
public:   
    const Choices *GetChoices() { return &choices; }
    const Ranges *GetDecisions() { return &decisions; }
    void InvalidateBack();
    void SetBackChoice( ContainerInterface::iterator newc );
    void PushBackChoice( ContainerInterface::iterator newc );
    
private: friend class Agent; 
    void SetDQR( const DecidedQueryResult &dqr );

    Choices choices;
    Ranges decisions;
};


bool operator<(const DecidedQueryResult::Block &l0, const DecidedQueryResult::Block &l1);
};
#endif