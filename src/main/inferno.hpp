#ifndef INFERNO_HPP
#define INFERNO_HPP

#include "helpers/transformation.hpp"
#include "vn/vn_step.hpp"

#include <vector>
#include <functional>

namespace VN
{
    class VNStep;
    class VNSequence;
};

class Inferno
{
public:    
    Inferno( shared_ptr<VN::VNSequence> vn_sequence );
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
    shared_ptr<VN::VNSequence> vn_sequence;

    const class Plan
    {
    public:
        Plan(Inferno *algo_);    

        Inferno *algo;
        vector<Step> steps;      
        list<Stage> stages;  
    } plan;
    
    typedef function<void(const Step &sp, string outfile, bool add_file_extension, string title)> PatternAction;
    
public:    
    void RunStage( Stage stage );
                         
    void GeneratePatternGraphs();
    void GeneratePatternRenders();
    void PatternDispatcher( PatternAction action, int pattern_index, string pattern_name, bool prepend_step_number = true );
    void DoPatternGraph( const Step &sp, string outfile, bool add_file_extension, string title ) const;
	void DoPatternRender( const Step &sp, string outfile, bool add_file_extension, string title ) const;
    
    void RunTransformationStep(const Step &sp);
    void Run();
    
    static bool ShouldIQuitAfter(Progress::Stage stage);
    
private:
    TreePtr<Node> program;
};

#endif

