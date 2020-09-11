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
        const TreePtrInterface *ppattern;
#ifdef KEEP_WHODAT_INFO
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
        Link();
        bool operator<(const Link &other) const;
        bool operator!=(const Link &other) const;
        bool operator==(const Link &other) const;
        explicit operator bool() const;
        Agent *GetChildAgent() const;
    };
        
    typedef list< shared_ptr<Link> > Links;       
    typedef vector<Decision> Decisions;

    void clear()
    {
		normal_links.clear();
		abnormal_links.clear();
        multiplicity_links.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
    
    void RegisterDecision( bool inclusive ); 
    
    void RegisterNormalLink( const TreePtrInterface *ppattern );
    void RegisterAbnormalLink( const TreePtrInterface *ppattern );
    void RegisterMultiplicityLink( const TreePtrInterface *ppattern );
    
    void RegisterEvaluator( shared_ptr<BooleanEvaluator> e );
    
    const Decisions *GetDecisions() const { return &decisions; } 

    const Links *GetNormalLinks() const { return &normal_links; } // pointer returned because the links contain the local links
    const Links *GetAbnormalLinks() const { return &abnormal_links; } // pointer returned because the links contain the local links
    const Links *GetMultiplicityLinks() const { return &multiplicity_links; } // pointer returned because the links contain the local links
    Links GetAllLinks() const; 

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
        const TreePtrInterface *ppattern;
        TreePtr<Node> x; 
#ifdef KEEP_WHODAT_INFO
        void *whodat; // the gdb magic you require is eg "info line *b.whodat"
#endif
        Link();
        bool operator<(const Link &other) const;
        explicit operator bool() const;
        Agent *GetChildAgent() const;
        operator PatternQuery::Link() const;
    };
    
    typedef vector<Range> Ranges;
    typedef vector<Choice> Choices; 
    typedef list< shared_ptr<Link> > Links;
    
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
    virtual bool IsNextChoiceValid() const = 0;
    virtual const Range &GetNextOldDecision() const = 0;
    virtual ContainerInterface::iterator SkipDecision() = 0;

    virtual void CompleteDecisionsWithEmpty() = 0;

    virtual void RegisterNormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x ) = 0; 
    virtual void RegisterAbnormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x ) = 0; 
    virtual void RegisterMultiplicityLink( const TreePtrInterface *ppattern, TreePtr<SubContainer> x ) = 0; 
    virtual void RegisterAlwaysMatchingLink( const TreePtrInterface *ppattern ) = 0; // Is a normal link
    
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
    virtual void Start() = 0;

    virtual const Links *GetNormalLinks() const = 0; 
    virtual const Links *GetAbnormalLinks() const = 0; 
    virtual const Links *GetMultiplicityLinks() const = 0; 
    virtual Links GetAllLinks() const = 0; 

    virtual shared_ptr<BooleanEvaluator> GetEvaluator() const = 0;
    
    virtual void Invalidate( int bc ) = 0;
    virtual void SetChoice( int bc, Choice newc ) = 0;
    virtual void PushBackChoice( Choice newc ) = 0;    
    virtual void EnsureChoicesHaveIterators() = 0;
};


class DecidedQuery : virtual public DecidedQueryClientInterface,
                     virtual public DecidedQueryAgentInterface
{
public:    
    DecidedQuery( shared_ptr<const PatternQuery> pq );
    void Start();
    void Reset();

    ContainerInterface::iterator RegisterDecision( const Range &d );
    ContainerInterface::iterator RegisterDecision( ContainerInterface::iterator begin,
                                                   ContainerInterface::iterator end,
                                                   bool inclusive,
                                                   std::shared_ptr<ContainerInterface> container=nullptr );
    ContainerInterface::iterator RegisterDecision( std::shared_ptr<ContainerInterface> container, bool inclusive );
    ContainerInterface::iterator RegisterDecision( const Sequence<Node> &container, bool inclusive );
    ContainerInterface::iterator RegisterDecision( const Collection<Node> &container, bool inclusive );
    bool IsNextChoiceValid() const;
    const Range &GetNextOldDecision() const;
    ContainerInterface::iterator SkipDecision();
    void CompleteDecisionsWithEmpty();

    void RegisterNormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x ); 
    void RegisterAbnormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x ); 
    void RegisterMultiplicityLink( const TreePtrInterface *ppattern, TreePtr<SubContainer> x ); 
    void RegisterAlwaysMatchingLink( const TreePtrInterface *ppattern ); // Is a normal link

    void RegisterEvaluator( shared_ptr<BooleanEvaluator> e ); 
                                                  
    const Links *GetNormalLinks() const { return &normal_links; } 
    const Links *GetAbnormalLinks() const { return &abnormal_links; }
    const Links *GetMultiplicityLinks() const { return &multiplicity_links; }
    Links GetAllLinks() const; 

    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }
      
    const Choices *GetChoices() const { return &choices; }
    const Ranges *GetDecisions() const { return &decisions; }
    void Invalidate( int bc );
    void SetChoice( int bc, Choice newc );
    void PushBackChoice( Choice newc );    
    void EnsureChoicesHaveIterators();
    
private: friend class Conjecture;
    shared_ptr<BooleanEvaluator> evaluator;
    Links normal_links; 
    Links abnormal_links; 
    Links multiplicity_links; 
    Ranges decisions;
    Ranges::iterator next_decision;
    Choices choices;
    Choices::iterator next_choice;
    static shared_ptr< Collection<Node> > empty_container;
    static Range empty_range;
};

};
#endif