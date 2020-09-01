#ifndef SOLVER_HOLDER_HPP
#define SOLVER_HOLDER_HPP

#include "constraint.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

namespace CSP
{ 
    
class Agent;
class SimpleSolver;
    
/** A simple back-tracking solver
 */
class SolverHolder
{
public:
    SolverHolder( shared_ptr<SimpleSolver> solver_ );

    void Start( const set<Value> &initial_domain_ );

    void ReportSolution( const map< shared_ptr<Constraint>, list< Value > > &values, 
                         shared_ptr<SideInfo> side_info );

    bool GetNextSolution( map< shared_ptr<Constraint>, list< Value > > *values = nullptr, 
                          SideInfo *side_info = nullptr );
    
private:
    shared_ptr<SimpleSolver> solver;

    // Only needed to reserialise the matches TODO move to "holder" class
    list< pair< map< shared_ptr<Constraint>, list< Value > >, shared_ptr<SideInfo> > > matches;    
};

};

#endif
