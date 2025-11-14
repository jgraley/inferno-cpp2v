#ifndef SCTREE_HPP
#define SCTREE_HPP

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include <string>
#include "node/specialise_oostd.hpp"
#include "clang/Parse/DeclSpec.h"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"

/// SCTree namespace contains node definitions that represent elements of SystemC, as native language elements
namespace SCTree {

/** Base for all SystemC nodes, permitting detection of SystemC dialect eg for rendering */
struct SCNode : virtual Node {}; 

/// Anything derived from this renders like a record
struct SCRecord : virtual SCNode,
                  CPPTree::InheritanceRecord 
{
    NODE_FUNCTIONS
};

/// Anything derived from this renders like a function
struct SCFunction : virtual SCNode {};

/** SystemC event type, no members need to be set up. Event instances
    are declared to be of this type. They can then be signalled, waited etc. */
struct Event : CPPTree::Type
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "sc_event"; }
};

/** SystemC module type. The processes, registers, submodules and everything
    else within the module appear as members of the record. No constructor/destructor 
    is needed (so module is not a kind of class) - the renderer generates a constructor
    based on the module's members. Thus elaboration is structural in the tree
    (as with Verilog) and the renderer generates the run-time elaboration
    fu to satisfy SystemC. */
struct Module : SCRecord
{
    NODE_FUNCTIONS_FINAL
    
    virtual string GetLoweredIdName() const { return "sc_module"; }
  	TreePtr<CPPTree::AccessSpec> GetInitialAccess() const override { return MakeTreeNode<CPPTree::Private>(); }   
};

/** SystemC interface construct. Not exactly sure whether/how I will use 
    this. Presumably this is why Module comes from InheritanceRecord not Record. */                                                                      
struct Interface : SCRecord
{
    NODE_FUNCTIONS_FINAL
    
    virtual string GetLoweredIdName() const { return "sc_interface"; }
  	TreePtr<CPPTree::AccessSpec> GetInitialAccess() const override { return MakeTreeNode<CPPTree::Private>(); } 
};

/** SystemC interface construct. Not exactly sure whether/how I will use 
    this. Presumably this is why Module comes from InheritanceRecord not Record. */   
struct SCDynamicFunction : virtual SCFunction
{
    NODE_FUNCTIONS
    TreePtr<CPPTree::Expression> event; 
};


/** Intermediate node for SystemC wait() primitive. wait() can be used
    in a few fundamentally different ways and to be explicit we use specialised
    nodes for each. All waits must be done in local execution contexts
    like threads. Waits allow the SystemC kernel to run other processes. */
struct Wait : CPPTree::Statement,
              virtual SCFunction, 
              CPPTree::Uncombable
{
    NODE_FUNCTIONS
    virtual string GetLoweredIdName() const { return "wait"; }
};

/** Waiting for a SystemC event - blocks until the event indicated by the expression is 
    triggered, regardless of clocks, deltas etc. Ands and ors etc are probably OK as 
    the correpsonding boolean expressions on the events. */
struct WaitDynamic : Wait,
                     SCDynamicFunction
{
    NODE_FUNCTIONS_FINAL
};

/** Waiting for a SystemC event - blocks until an event is triggered. I think the event
    is specified during elaboration, but I'm not sure if nodes exist for this yet */
struct WaitStatic : Wait
{
    NODE_FUNCTIONS_FINAL
};

/** Basically a yield. Blocks for a short period of time (a delta cycle) and then continues.
    Allows other processes to run immediately. Equivalent to wait(SC_ZERO_TIME) */    
struct WaitDelta : Wait
{
    NODE_FUNCTIONS_FINAL
};

/** Intermediate node for SystemC next_trigger() primitive. next_trigger() can be used
    in a few fundamentally different ways and to be explicit we use specialised
    nodes for each. All next_triggers must be done in combable contexts like SC_METHOD.
    Next_triggers do NOT allow the SystemC kernel to run other processes until
    the combable block completes. */
struct NextTrigger : CPPTree::Statement,
                     virtual SCFunction
{
    NODE_FUNCTIONS
    virtual string GetLoweredIdName() const { return "next_trigger"; }
};

/** Causes the method to be triggered again when the event indicated by the expression is 
    triggered, regardless of clocks, deltas etc. Ands and ors etc are probably OK as 
    the correpsonding boolean expressions on the events. */
struct NextTriggerDynamic : NextTrigger,
                            SCDynamicFunction
{
    NODE_FUNCTIONS_FINAL
};

/** Causes the method to be triggered again when an event is triggered. I think the event
    is specified during elaboration, but I'm not sure if nodes exist for this yet */
struct NextTriggerStatic : NextTrigger
{
    NODE_FUNCTIONS_FINAL
};

/** Causes the method to be triggered again very soon after it completes.
    Other things run when combable block completes. Equivalent to 
    next_trigger(SC_ZERO_TIME) */    
struct NextTriggerDelta : NextTrigger
{
    NODE_FUNCTIONS_FINAL
};

/** Triggers the event instance given in the expression. It must be an lvalue of
    type Event I would think. This is an intermediate because there are a few
    distinct flavours. */
struct Notify : CPPTree::Statement,
                virtual SCFunction
{
    NODE_FUNCTIONS
    virtual string GetLoweredIdName() const { return "notify"; }
    TreePtr<CPPTree::Expression> event; ///< event to notify 
};

/** Notify the event immediately. Not sure if this can force control to go 
    directly to a waiting process like a function call. Probably not. */
struct NotifyImmediate : Notify
{
    NODE_FUNCTIONS_FINAL
};

/** Notify the event with a delta cycle delay (SC_ZERO_TIME). */
struct NotifyDelta : Notify
{
    NODE_FUNCTIONS_FINAL
};

/** Notify the event with a specified time delay. */
struct NotifyTimed : Notify
{
    NODE_FUNCTIONS_FINAL
    TreePtr<CPPTree::Expression> time; ///< how long to wait for before notifying
};

/** Intermediate class for processes, which are the places in SystemC where we 
    put code. Different final nodes have different invocation and execution models.
    Processes look like functions that have no params or return value. */
struct Process : CPPTree::Subroutine,
                 virtual SCNode 
{
    NODE_FUNCTIONS
};

/** Any process that begins or resumes execution in response to events (presumably
    including the virtual events created by deltas). Essentially unclocked */
struct EventProcess : Process
{
    NODE_FUNCTIONS
   //TODO Collection<Sensitivity> sensitivity;
};

/** SystemC method process. This must contain a combable block of code. The method
    will return when the combinational block has settled and then issued any state
    updates. */
struct Method : EventProcess
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "SC_METHOD"; }
};

/** SystemC thread process. Local context, so this can run forever (stopping only to indicate completion
    of a test run) and may block in wait or run busy for a while. Actually I dont think 
    the SystemC kernel can pre-empt, so it should not run busy all the time. */
struct Thread : EventProcess // TODO if SystemC really can't pre-empt, then this should be renamed to Context
                             // TODO and I should create a real thread support extension because user's threads will run busy sometimes
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "SC_THREAD"; }
};

/** SystemC clocked thread process. A local context as with Thread, but can only
    wait on clock cycles. I forget the advantage of this over Thread.
    Sensitivity must be a ClockInPort */
struct ClockedThread : Process
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "SC_CTHREAD"; }
    //TODO TreePtr<Sensitivity> clock;
};

/** Evaluates to the total number of delta cycles thus far. Can be compared with zero 
    to produce an inferred reset signal for initialising state machines */
struct DeltaCount : CPPTree::Operator,
                    virtual SCFunction // TODO rename as InferredReset() since that will transform more easily to a real reset system
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "sc_delta_count"; }    
	Production GetMyProduction() const override { return Production::POSTFIX; };	// renders like a function call
};

/// Termination functions
/** These are used to stop the program and produce an exit code because 
    SystemC does not allow control of the return value from its main 
    function. */
struct TerminationFunction : CPPTree::Statement,
                             virtual SCFunction
{
    NODE_FUNCTIONS
    TreePtr<CPPTree::Expression> code; ///< exit code for program, 0 to 255 
};

/// The exit() system call; cease() below is preferred.
struct Exit : TerminationFunction
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "exit"; }
};


/// For SC extension. Not ideal because nodes will still have SCNode but
/// it will do for determining header file inclusion during lowering.
struct SCExtension : virtual Node
{
	NODE_FUNCTIONS
};

/// Cease function 
/** an alternative to exit(), supplied by the inferno runtime glue for SC
    to complete logging activities etc before calling exit(). */
struct Cease : TerminationFunction, SCExtension
{
    NODE_FUNCTIONS_FINAL
    virtual string GetLoweredIdName() const { return "cease"; }
};

};

#endif
