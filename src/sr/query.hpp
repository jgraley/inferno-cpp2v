#ifndef QUERY_HPP
#define QUERY_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

#include "boolean_evaluator.hpp"
#include "subcontainers.hpp" 
#include "link.hpp" 

#include <vector>

namespace SR
{ 
class Agent;

class PatternQuery 
{
public:    
    PatternQuery();
    struct Decision
    {
        bool inclusive; // If true, include "end" as a possible choice
    };
    typedef PatternLink Link;        
    typedef list<PatternLink> Links;       
    typedef vector<Decision> Decisions;

    void clear();
    
    void RegisterDecision( bool inclusive ); 
    
    void RegisterNormalLink( PatternLink plink );
    void RegisterAbnormalLink( PatternLink plink );
    void RegisterMultiplicityLink( PatternLink plink );
    
    void RegisterEvaluator( shared_ptr<BooleanEvaluator> e );
    
    const Decisions &GetDecisions() const { return decisions; } 

    const Links &GetNormalLinks() const { return normal_links; } // pointer returned because the links contain the local links
    const Links &GetAbnormalLinks() const { return abnormal_links; } // pointer returned because the links contain the local links
    const Links &GetMultiplicityLinks() const { return multiplicity_links; } // pointer returned because the links contain the local links
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
    struct Range : public Traceable
    {
        Range(ContainerInterface::iterator begin,
              ContainerInterface::iterator end,    
              bool inclusive,
              std::shared_ptr<ContainerInterface> container);
        Range() {}
        bool operator==(const Range &o) const;
        string GetTrace() const;
        
        ContainerInterface::iterator begin;
        ContainerInterface::iterator end;    
        bool inclusive; // If true, include "end" as a possible choice
        std::shared_ptr<ContainerInterface> container; // Only needed if the container is not part of the x tree
    };
    
    struct Choice
    {
        enum
        {
            BEGIN,
            ITER
        } mode;
        ContainerInterface::iterator iter;
        
        string GetTrace( const Range &d ) const;
    };
    
    typedef LocatedLink Link;
    typedef vector<Range> Ranges;
    typedef vector<Choice> Choices; 
    typedef SolutionMap Links;
    typedef map<PatternLink, TreePtr<Node>> Nodes;
    typedef map<PatternLink, pair<TreePtr<Node>, XLink>> Pairs;
    
    virtual const Choices &GetChoices() const = 0;
    virtual const Ranges &GetDecisions() const = 0;
    
    enum
    {
        NEW,
        QUERY,
        CONJECTURE
    } last_activity = NEW;
    
    static void AssertMatchingLinks( const DecidedQueryCommon::Links &mut_links, 
                                     const DecidedQueryCommon::Links &ref_links );
    static void AssertMatchingNodes( const DecidedQueryCommon::Nodes &mut_nodes, 
                                     const DecidedQueryCommon::Nodes &ref_nodes );
    static string TraceLinks( const DecidedQueryCommon::Links &links );    
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

    virtual void CompleteDecisionsWithEmpty() = 0;

    virtual void RegisterNormalLink( PatternLink plink, XLink xlink ) = 0; 
    virtual void RegisterAbnormalNode( PatternLink plink, TreePtr<Node> node, XLink xlink=XLink() ) = 0; 
    virtual void RegisterMultiplicityNode( PatternLink plink, TreePtr<Node> node ) = 0;
    
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

    virtual const Links &GetNormalLinks() const = 0; 
    virtual const Pairs &GetAbnormalNodes() const = 0; 
    virtual const Nodes &GetMultiplicityNodes() const = 0; 
    
    virtual void Invalidate( Choices::size_type bc ) = 0;
    virtual void SetChoice( Choices::size_type bc, Choice newc ) = 0;
    virtual void PushBackChoice( Choice newc ) = 0;    
};


class DecidedQuery : virtual public DecidedQueryClientInterface,
                     virtual public DecidedQueryAgentInterface,
                     public Traceable
{
public:    
    DecidedQuery( shared_ptr<const PatternQuery> pq );
    void Start();
    void Reset();

    ContainerInterface::iterator RegisterDecision( const Range &d ) final;
    ContainerInterface::iterator RegisterDecision( ContainerInterface::iterator begin,
                                                   ContainerInterface::iterator end,
                                                   bool inclusive,
                                                   std::shared_ptr<ContainerInterface> container=nullptr ) final;
    ContainerInterface::iterator RegisterDecision( std::shared_ptr<ContainerInterface> container, bool inclusive ) final;
    ContainerInterface::iterator RegisterDecision( const Sequence<Node> &container, bool inclusive );
    ContainerInterface::iterator RegisterDecision( const Collection<Node> &container, bool inclusive );
    void CompleteDecisionsWithEmpty() final;

    void RegisterNormalLink( PatternLink plink, XLink xlink ) final; 
    void RegisterAbnormalNode( PatternLink plink, TreePtr<Node> node, XLink xlink=XLink() ) final; 
    void RegisterMultiplicityNode( PatternLink plink, TreePtr<Node> node ) final; 
                                                  
    const Links &GetNormalLinks() const final { return normal_links; } 
    const Pairs &GetAbnormalNodes() const final { return abnormal_nodes; }
    const Nodes &GetMultiplicityNodes() const final { return multiplicity_nodes; }
     
    const Choices &GetChoices() const final { return choices; }
    const Ranges &GetDecisions() const final { return decisions; }
    void Invalidate( Choices::size_type bc ) final;
    void SetChoice( Choices::size_type bc, Choice newc ) final;
    void PushBackChoice( Choice newc ) final;    
    string GetTrace() const;    
    
private: friend class Conjecture;
    Links normal_links; 
    Pairs abnormal_nodes; 
    Nodes multiplicity_nodes; 
    Ranges decisions;
    Ranges::iterator next_decision;
    Choices choices;
    Choices::iterator next_choice;
    static shared_ptr< Collection<Node> > empty_container;
    static Range empty_range;
};

};

#endif
