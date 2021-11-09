#ifndef INFERNO_HPP
#define INFERNO_HPP

#include "helpers/transformation.hpp"

#include <vector>


class Inferno
{
    struct StepPlan
    {
        shared_ptr<Transformation> tx;
        int step_index;
        bool allow_trace;
        bool allow_hits;
        bool allow_reps;
    };

    const class Plan
    {
    public:
        Plan();

        vector<StepPlan> steps_plan;
    } plan;
    
public:    
    Inferno();

    void MaybeGeneratePatternGraphs();
    void Run();
};

#endif

