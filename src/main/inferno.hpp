#ifndef INFERNO_HPP
#define INFERNO_HPP

#include "helpers/transformation.hpp"
#include "vn/vn_step.hpp"

#include <vector>
#include <functional>

namespace SR
{
    class VNStep;
    class VNSequence;
};

class Inferno
{
public:    
    Inferno( shared_ptr<SR::VNSequence> vn_sequence );
    ~Inferno();
    
    struct Step
    {
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
        function<void(const Step &)> step_function;
        function<void()> stage_function;
    };

private:
    shared_ptr<SR::VNSequence> vn_sequence;

    const class Plan
    {
    public:
        Plan(Inferno *algo_);    

        Inferno *algo;
        vector<Step> steps;      
        list<Stage> stages;  
    } plan;
    
public:    
    void RunStage( Stage stage );
                         
    void GeneratePatternGraphs();
    void GenerateGraphRegions( const Step &sp, Graph &graph );
    void RunTransformationStep(const Step &sp);
    void Run();
    
    static bool ShouldIQuitAfter(Progress::Stage stage);
    
private:
    TreePtr<Node> program;
};

#endif

