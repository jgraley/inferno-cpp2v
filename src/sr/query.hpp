#ifndef QUERY_HPP
#define QUERY_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

#include "boolean_evaluator.hpp"
#include "subcontainers.hpp" 

#include <vector>
#include <boost/type_traits.hpp>

namespace SR
{ 
class Agent;

class PatternQuery 
{
public:    
    struct Decision
    {
        bool inclusive; // If true, include "end" as a possible choice
    };
    struct Link 
    {
        Agent *agent;
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
    };
        
    typedef list<Link> Links;       
    typedef vector<Decision> Decisions;

    void clear()
    {
		normal_links.clear();
		abnormal_links.clear();
        multiplicity_links.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
    
    void RegisterDecision( bool inclusive ); 
    void RegisterNormalLink( Agent *a );
    void RegisterAbnormalLink( Agent *a );
    void RegisterMultiplicityLink( Agent *a );
    void RegisterEvaluator( shared_ptr<BooleanEvaluator> e );
    
    const Decisions *GetDecisions() const { return &decisions; } 
    const Links *GetNormalLinks() const { return &normal_links; } // pointer returned because the links contain the local links
    const Links *GetAbnormalLinks() const { return &abnormal_links; } // pointer returned because the links contain the local links
    const Links *GetMultiplicityLinks() const { return &multiplicity_links; } // pointer returned because the links contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }

private:
    Decisions decisions; // ha ha!
    Links normal_links; 
    Links abnormal_links; 
    Links multiplicity_links; 
    shared_ptr<BooleanEvaluator> evaluator;
};


class DecidedQueryCommon
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
			if( begin != o.begin )
                return false;
            if( end != o.end )
                return false;
            if( inclusive != o.inclusive )
                return false;
            if( container != o.container )
                return false;
            return true;
		}
    };
    
    struct Choice
    {
        enum
        {
            BEGIN,
            ITER
        } mode;
        ContainerInterface::iterator iter;
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
    typedef vector<Choice> Choices; 
    typedef list<Link> Links;
    
    virtual const Choices *GetChoices() const = 0;
    virtual const Ranges *GetDecisions() const = 0;
    
    enum
    {
        NEW,
        QUERY,
        CONJECTURE
    } last_activity = NEW;
};


class DecidedQueryAgentInterface : virtual public DecidedQueryCommon
{
public:
    virtual void Reset() = 0;

    virtual ContainerInterface::iterator RegisterDecision( const Range &d ) = 0; 
    virtual ContainerInterface::iterator RegisterDecision( ContainerInterface::iterator begin,
                                                           ContainerInterface::iterator end,
                                                           bool inclusive,
                                                           std::shared_ptr<ContainerInterface> container=nullptr ) = 0; 
    virtual ContainerInterface::iterator RegisterDecision( std::shared_ptr<ContainerInterface> container, bool inclusive ) = 0; 
    virtual const Range &GetNextOldDecision() const = 0;
    virtual Ranges::iterator GetNextDecisionIterator() const = 0;
    virtual void CompleteDecisionsWithEmpty() = 0;

    virtual void RegisterNormalLink( Agent *a, const TreePtrInterface *px ) = 0; 
    virtual void RegisterAbnormalLink( Agent *a, const TreePtrInterface *px ) = 0; 
    virtual void RegisterMultiplicityLink( Agent *a, const TreePtrInterface *px ) = 0; 
    virtual void RegisterLocalNormalLink( Agent *a, TreePtr<Node> x ) = 0; 
    virtual void RegisterLocalAbnormalLink( Agent *a, TreePtr<Node> x ) = 0; 
    virtual void RegisterLocalMultiplicityLink( Agent *a, TreePtr<SubContainer> x ) = 0; 
    virtual void RegisterEvaluator( shared_ptr<BooleanEvaluator> e ) = 0; 

    class RAIIDecisionsCleanup
    {
    public:
        RAIIDecisionsCleanup( DecidedQueryAgentInterface &query_ ) : query(query_) {}
        ~RAIIDecisionsCleanup()
        {
            query.CompleteDecisionsWithEmpty();
        }
        DecidedQueryAgentInterface &query;
    };
    
};


class DecidedQueryClientInterface : virtual public DecidedQueryCommon
{
public:
    virtual const Links *GetNormalLinks() const = 0; // pointer returned because the links contain the local links
    virtual const Links *GetAbnormalLinks() const = 0; // pointer returned because the links contain the local links
    virtual const Links *GetMultiplicityLinks() const = 0; // pointer returned because the links contain the local links
    virtual shared_ptr<BooleanEvaluator> GetEvaluator() const = 0;
    
    virtual void InvalidateBack() = 0;
    virtual void SetBackChoice( Choice newc ) = 0;
    virtual void PushBackChoice( Choice newc ) = 0;    
    virtual void EnsureChoicesHaveIterators() = 0;
};


class DecidedQuery : virtual public DecidedQueryClientInterface,
                     virtual public DecidedQueryAgentInterface
{
public:    
    DecidedQuery(const PatternQuery &pq);
    void Reset();

    ContainerInterface::iterator RegisterDecision( const Range &d );
    ContainerInterface::iterator RegisterDecision( ContainerInterface::iterator begin,
                                                   ContainerInterface::iterator end,
                                                   bool inclusive,
                                                   std::shared_ptr<ContainerInterface> container=nullptr );
    ContainerInterface::iterator RegisterDecision( std::shared_ptr<ContainerInterface> container, bool inclusive );
    const Range &GetNextOldDecision() const;
    Ranges::iterator GetNextDecisionIterator() const;
    void CompleteDecisionsWithEmpty();

    void RegisterNormalLink( Agent *a, const TreePtrInterface *px ); 
    void RegisterAbnormalLink( Agent *a, const TreePtrInterface *px ); 
    void RegisterMultiplicityLink( Agent *a, const TreePtrInterface *px ); 
    void RegisterLocalNormalLink( Agent *a, TreePtr<Node> x ); 
    void RegisterLocalAbnormalLink( Agent *a, TreePtr<Node> x ); 
    void RegisterLocalMultiplicityLink( Agent *a, TreePtr<SubContainer> x ); 
    void RegisterEvaluator( shared_ptr<BooleanEvaluator> e ); 
                                                  
    const Links *GetNormalLinks() const { return &normal_links; } // pointer returned because the links contain the local links
    const Links *GetAbnormalLinks() const { return &abnormal_links; } // pointer returned because the links contain the local links
    const Links *GetMultiplicityLinks() const { return &multiplicity_links; } // pointer returned because the links contain the local links
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }
      
    const Choices *GetChoices() const { return &choices; }
    const Ranges *GetDecisions() const { return &decisions; }
    void InvalidateBack();
    void SetBackChoice( Choice newc );
    void PushBackChoice( Choice newc );    
    void EnsureChoicesHaveIterators();
    
private:
    shared_ptr<BooleanEvaluator> evaluator;
    Links normal_links; 
    Links abnormal_links; 
    Links multiplicity_links; 
    Ranges decisions;
    Ranges::iterator next_decision;
    Choices choices;
    Choices::iterator next_choice;
    static shared_ptr< Collection<Node> > empty_container;
};


bool operator<(const DecidedQuery::Link &l0, const DecidedQuery::Link &l1);
};
#endif