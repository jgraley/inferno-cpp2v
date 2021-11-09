#ifndef INFERNO_HPP
#define INFERNO_HPP

#include "helpers/transformation.hpp"
#include "sr/vn_transformation.hpp"

#include <vector>
#include <functional>


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
            bool allow_stop;
        };

        struct Stage
        {
            Progress::Stage progress_stage;
            bool allow_trace;
            bool allow_hits;
            bool allow_reps;
            bool allow_stop;
            //bool try_generate_pattern_graphs;
            string text;
            function<void(shared_ptr<SR::VNTransformation>, const Plan::Step &sp)> step_function;
        };

        Plan();

        vector<Step> steps;
    } plan;
    
public:    
    Inferno();

    void RunSteppyStage( Plan::Stage stage );
                         
    void MaybeGeneratePatternGraphs();
    void Run();
};

#endif

