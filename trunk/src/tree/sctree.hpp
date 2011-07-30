#ifndef SCTREE_HPP
#define SCTREE_HPP

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include <string>
#include <deque>
#include "node/node.hpp"
#include "clang/Parse/DeclSpec.h"
#include "tree/type_db.hpp"
#include "tree/cpptree.hpp"

namespace SCTree {

struct SCConstruct {};

/// Base for SC nodes that correspond to named ements of SystemC
struct SCNamedConstruct : public SCConstruct
{
    virtual string GetToken() {ASSERTFAIL("GetToken() called on intermediate node\n")} ///< Produce the name of the corresponding SC element for detection and rendering
};

struct SCNamedIdentifier : SCNamedConstruct {};
struct SCNamedRecord : SCNamedConstruct {};
struct SCNamedFunction : SCNamedConstruct {};


struct Event : CPPTree::Type,
               SCNamedIdentifier
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "sc_event"; }
};

struct Module : CPPTree::InheritanceRecord,
                SCNamedRecord
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "sc_module"; }
};

struct Interface : CPPTree::InheritanceRecord,
                   SCNamedRecord
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "sc_interface"; }
};

struct Wait : CPPTree::Statement,
                 SCNamedFunction
{
    NODE_FUNCTIONS
    virtual string GetToken() { return "wait"; }
};

struct WaitDynamic : Wait
{
    NODE_FUNCTIONS_FINAL
    TreePtr<CPPTree::Expression> event; ///< event to wait for 
};

struct WaitStatic : Wait
{
    NODE_FUNCTIONS_FINAL
};

struct WaitDelta : Wait
{
    NODE_FUNCTIONS_FINAL
};

struct NextTrigger : CPPTree::Statement,
                     SCNamedFunction
{
    NODE_FUNCTIONS
    virtual string GetToken() { return "next_trigger"; }
};

struct NextTriggerDynamic : NextTrigger
{
    NODE_FUNCTIONS_FINAL
    TreePtr<CPPTree::Expression> event; ///< event to wait for 
};

struct NextTriggerStatic : NextTrigger
{
    NODE_FUNCTIONS_FINAL
};

struct NextTriggerDelta : NextTrigger
{
    NODE_FUNCTIONS_FINAL
};

struct Notify : CPPTree::Statement,
                SCNamedFunction
{
    NODE_FUNCTIONS
    virtual string GetToken() { return "notify"; }
    TreePtr<CPPTree::Expression> event; ///< event to notify 
};

struct NotifyImmediate : Notify
{
    NODE_FUNCTIONS_FINAL
};

struct NotifyDelta : Notify
{
    NODE_FUNCTIONS_FINAL
};

struct NotifyTimed : Notify
{
    NODE_FUNCTIONS_FINAL
    TreePtr<CPPTree::Expression> time; ///< how long to wait for before notifying
};

struct Process : CPPTree::Subroutine,
                 SCNamedConstruct 
{
    NODE_FUNCTIONS
};

struct EventProcess : Process
{
    NODE_FUNCTIONS
   //TODO Collection<Sensitivity> sensitivity;
};

/** SystemC method */
struct Method : EventProcess
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "SC_METHOD"; }
};

/** SystemC thread */
struct Thread : EventProcess
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "SC_THREAD"; }
};

/** SystemC clocked thread 
    Sensitivity must be a ClockInPort */
struct ClockedThread : Process
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "SC_CTHREAD"; }
    //TODO TreePtr<Sensitivity> clock;
};

/// The Exit system call
/** we use this to get a result code out of the program because 
    SystemC does not allow control of the return value from its main 
    function. Not strictly SystemC. */
struct Exit : CPPTree::Statement,
              SCNamedFunction
{
    NODE_FUNCTIONS_FINAL
    virtual string GetToken() { return "exit"; }
    TreePtr<CPPTree::Expression> code; ///< exit code for program, 0 to 255 
};

};

#endif
