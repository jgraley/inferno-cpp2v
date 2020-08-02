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
    struct Link 
    {
        Agent *agent;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
        
    typedef list<Link> Links;       
        
    void clear()
    {
		normal_links.clear();
		abnormal_links.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
    void AddNormalLink( Agent *a );
    void AddAbnormalLink( Agent *a );
    void AddEvaluator( shared_ptr<BooleanEvaluator> e );
    
    const Links *GetNormalLinks() const { return &normal_links; } // pointer returned because the links contain the local links
    const Links *GetAbnormalLinks() const { return &abnormal_links; } // pointer returned because the links contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }

private:
    Links normal_links; 
    Links abnormal_links; 
    shared_ptr<BooleanEvaluator> evaluator;
};


class QueryCommonInterface
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
    
    struct Link 
    {
        const TreePtrInterface *GetPX() const
        {
			if( px )
				return px; // linked x is in input tree
			else
				return &local_x; // linked x is local, kept alive by local_x    
		}	

        Agent *agent;
        const TreePtrInterface *px;
        TreePtr<Node> local_x;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
    
    // There is a "random access" in Conjecture::FillMissingChoicesWithBegin()
    typedef vector<Range> Ranges;
    typedef vector<ContainerInterface::iterator> Choices; 
    typedef list<Link> Links;
    
    virtual const Choices *GetChoices() = 0;
};


class QueryAgentInterface : virtual public QueryCommonInterface
{
public:
    virtual void Reset() = 0;

    virtual ContainerInterface::iterator AddDecision( const Range &d ) = 0; 
    virtual ContainerInterface::iterator AddDecision( ContainerInterface::iterator begin,
                                                      ContainerInterface::iterator end,
                                                      bool inclusive,
                                                      std::shared_ptr<ContainerInterface> container=nullptr ) = 0; 
    virtual ContainerInterface::iterator AddDecision( std::shared_ptr<ContainerInterface> container, bool inclusive ) = 0; 
    //virtual ContainerInterface::iterator AddDecision( const ContainerInterface &container, bool inclusive );
    virtual bool IsAlreadyGotNextOldDecision() = 0; 
    virtual const Range &GetNextOldDecision() = 0; 
    virtual ContainerInterface::iterator AddNextOldDecision() = 0; 

    virtual void AddNormalLink( Agent *a, const TreePtrInterface *px ) = 0; 
    virtual void AddAbnormalLink( Agent *a, const TreePtrInterface *px ) = 0; 
    virtual void AddLocalNormalLink( Agent *a, TreePtr<Node> x ) = 0; 
    virtual void AddLocalAbnormalLink( Agent *a, TreePtr<Node> x ) = 0; 
    virtual void AddEvaluator( shared_ptr<BooleanEvaluator> e ) = 0; 
};


class QueryClientInterface : virtual public QueryCommonInterface
{
public:
    virtual const Links *GetNormalLinks() const = 0; // pointer returned because the links contain the local links
    virtual const Links *GetAbnormalLinks() const = 0; // pointer returned because the links contain the local links
    virtual shared_ptr<BooleanEvaluator> GetEvaluator() const = 0;
    
    virtual const Ranges *GetDecisions() = 0;
    virtual void InvalidateBack() = 0;
    virtual void SetBackChoice( ContainerInterface::iterator newc ) = 0;
    virtual void PushBackChoice( ContainerInterface::iterator newc ) = 0;    
};


class AgentQuery : virtual public QueryClientInterface,
                   virtual public QueryAgentInterface
{
public:    
    AgentQuery() :
        next_decision( decisions.begin() ), // will be end()
        next_choice( choices.begin() ) // will be end()
    {
    }
    void Reset();

    ContainerInterface::iterator AddDecision( const Range &d );
    ContainerInterface::iterator AddDecision( ContainerInterface::iterator begin,
                                              ContainerInterface::iterator end,
                                              bool inclusive,
                                              std::shared_ptr<ContainerInterface> container=nullptr );
    ContainerInterface::iterator AddDecision( std::shared_ptr<ContainerInterface> container, bool inclusive );
    //ContainerInterface::iterator AddDecision( const ContainerInterface &container, bool inclusive );
    bool IsAlreadyGotNextOldDecision();
    const Range &GetNextOldDecision();
    ContainerInterface::iterator AddNextOldDecision();

    void AddNormalLink( Agent *a, const TreePtrInterface *px ); 
    void AddAbnormalLink( Agent *a, const TreePtrInterface *px ); 
    void AddLocalNormalLink( Agent *a, TreePtr<Node> x ); 
    void AddLocalAbnormalLink( Agent *a, TreePtr<Node> x ); 
    void AddEvaluator( shared_ptr<BooleanEvaluator> e ); 
    void AddLocalMismatch();
                                                  
    const Links *GetNormalLinks() const { return &normal_links; } // pointer returned because the links contain the local links
    const Links *GetAbnormalLinks() const { return &abnormal_links; } // pointer returned because the links contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }
      
    const Choices *GetChoices() { return &choices; }
    const Ranges *GetDecisions() { return &decisions; }
    void InvalidateBack();
    void SetBackChoice( ContainerInterface::iterator newc );
    void PushBackChoice( ContainerInterface::iterator newc );    
    
private:
    shared_ptr<BooleanEvaluator> evaluator;
    Links normal_links; 
    Links abnormal_links; 
    Ranges decisions;
    Ranges::iterator next_decision;
    Choices choices;
    Choices::iterator next_choice;
};


bool operator<(const AgentQuery::Link &l0, const AgentQuery::Link &l1);
};
#endif