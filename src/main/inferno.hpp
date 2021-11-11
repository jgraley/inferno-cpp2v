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
            string text;
            function<void(shared_ptr<SR::VNTransformation>, const Plan::Step &)> step_function;
            function<void()> stage_function;
        };

        Plan(Inferno *algo_);
        bool ShouldIQuitAfter(Stage stage);

        Inferno *algo;
        vector<Step> steps;      
        list<Stage> stages;  
    } plan;
    
public:    
    Inferno();

    void RunStage( Plan::Stage stage );
                         
    void GeneratePatternGraphs();
    void RunTransformationStep(shared_ptr<SR::VNTransformation> pvnt, const Plan::Step &sp);
    void Run();
    
private:
    TreePtr<Node> program;
};

#endif

