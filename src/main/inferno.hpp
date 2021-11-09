#ifndef INFERNO_HPP
#define INFERNO_HPP

#include "helpers/transformation.hpp"

#include <vector>


class Inferno
{
    const class Plan
    {
    public:
        struct Step
        {
            shared_ptr<Transformation> tx;
            int step_index;
            bool allow_trace;
            bool allow_hits;
            bool allow_reps;
        };

        Plan();

        vector<Step> steps;
    } plan;
    
public:    
    Inferno();

    void MaybeGeneratePatternGraphs();
    void Run();
};

#endif

