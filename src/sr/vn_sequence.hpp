#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"

class Graph;

/// SR namespace contains the search and replace implementation
namespace SR 
{

class VNTransformation;

/**
 * Vida Nova Sequence
 */
class VNSequence
{
public:    
    virtual void Configure( const vector< shared_ptr<VNTransformation> > &sequence );
    
    void PatternTransformations( int step_index );

    void PlanningStageOne( int step_index );
    void PlanningStageTwo( int step_index );
    void PlanningStageThree( int step_index );
    
    static void SetMaxReps( int step_index, int n, bool e );
    void SetStopAfter( int step_index, vector<int> ssa, int d=0 );    
        
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( int step_index,
                     TreePtr<Node> context, 
                     TreePtr<Node> *proot );
    
    /*virtual Block GetGraphBlockInfo() const;
    virtual string GetGraphId() const; */
    void GenerateGraphRegions( int step_index, Graph &graph ) const;

private:
    vector< shared_ptr<VNTransformation> > steps;
};    
    
}

#endif
