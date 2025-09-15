#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "agents/agent.hpp"
#include <set>

class Graph;


namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class SCREngine;
class VNSequence;

/// Main implementation of a compare and replace function, that matches at the root of the program tree
class CompareReplace : public virtual Graphable,
                       public SerialNumber
{  
public:
    typedef map<const Agent *, Agent::Phase> AgentPhases;
    
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    CompareReplace( bool is_search=false );
                    
private:
    struct Plan : public virtual Traceable
    {          
        Plan( CompareReplace *algo, 
              bool is_search );
        void Configure( TreePtr<Node> cp,
                        TreePtr<Node> rp );
        void PlanningStageOne(VNSequence *vn_sequence);
        void PlanningStageTwo();
        void PlanningStageThree();
        void PlanningStageFive( shared_ptr<const Lacing> lacing );
        string GetTrace() const; // used for debug

        TreePtr<Node> compare_pattern;
        TreePtr<Node> replace_pattern;
        
        CompareReplace * const algo;
        const bool is_search;   
        shared_ptr<SCREngine> scr_engine;
        AgentPhases agent_phases;   
    } plan;

public:
    virtual void Configure( TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    void PlanningStageOne(VNSequence *vn_sequence);
    void PlanningStageTwo();
    void PlanningStageThree();
    void PlanningStageFive( shared_ptr<const Lacing> lacing );

    void SetXTreeDb( shared_ptr<XTreeDatabase> x_tree_db );
    void Transform();
    
    virtual NodeBlock GetGraphBlockInfo() const;
    virtual string GetGraphId() const; 
    void GenerateGraphRegions( Graph &graph ) const;

    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    SCREngine *GetRootEngine();
    TreePtr<Node> GetSearchComparePattern();
    TreePtr<Node> GetEmbeddedReplacePattern();
    string GetTrace() const;

private:
    VNSequence *vn_sequence = nullptr;
    shared_ptr<XTreeDatabase> x_tree_db;
};


/// Main implementation of a search and replace function, that matches anywhere in program tree
class SearchReplace : public CompareReplace
{
public:
    SearchReplace();
};

};

#endif

