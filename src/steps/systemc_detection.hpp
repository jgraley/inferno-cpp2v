#ifndef SYSTEMC_DETECTION
#define SYSTEMC_DETECTION

// These transformations detect SystemC constructs by identifier 
// name and node pattern within input code, and substitute the 
// Inferno SystemC nodes.

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/vn_step.hpp"

namespace Steps {

using namespace VN;

/// spot SystemC type by its name and replace with inferno node 
/** We look for the decl and remeove it since the inferno
 Node does not require declaration. Then just switch each appearance
 over to the new node, using an embedded pattern */
class DetectSCType : public VNStep 
{
public:
    DetectSCType( TreePtr< SCTree::SCNamedConstruct > lr_scnode );
};


/// spot SystemC base class by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all class nodes
 that inherit from the supplied base with the new inferno node and 
 remove the base */
class DetectSCBase : public VNStep 
{
public:
    DetectSCBase( TreePtr< SCTree::SCNamedRecord > lr_scclass );
};


class DetectSCDynamic : public VNStep
{
public:
    DetectSCDynamic( TreePtr<SCTree::SCDynamicNamedFunction> r_dynamic );
};


class DetectSCStatic : public VNStep
{
public:
    DetectSCStatic( TreePtr<SCTree::SCNamedFunction> r_static );
};


class DetectSCDelta : public VNStep
{
public:
    DetectSCDelta( TreePtr<SCTree::SCNamedFunction> r_delta );
};


/// spot syscall exit() or equivalent function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's MapOperator style. */
class DetectTerminationFunction : public VNStep
{
public:
    DetectTerminationFunction( TreePtr<SCTree::TerminationFunction> r_tf );    
};


class DetectSCProcess : public VNStep
{
public:
    DetectSCProcess( TreePtr< SCTree::Process > lr_scprocess );
};



/// spot SystemC notify() method by its name and replace with inferno node 
/** Look for myevent.notify() and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class DetectSCNotifyImmediate : public VNStep  
{
public:
    DetectSCNotifyImmediate();
};


/// spot SystemC notify(SC_ZERO_TIME) method by its name and replace with inferno node 
/** Look for myevent.notify(SC_ZERO_TIME) and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class DetectSCNotifyDelta : public VNStep  
{
public:
    DetectSCNotifyDelta();
};


/// Remove constructors in SC modules that are now empty thanks to earlier steps
/// Must also remove explicit calls to constructor (which would not do anything)
class RemoveEmptyModuleConstructors : public VNStep
{
public:
    RemoveEmptyModuleConstructors();
};


/// Remove top-level instances that are of type void
/** isystemc.h declares void variables to satisfy parser. Hoover them all up
    efficiently here. */
class RemoveVoidInstances : public VNStep
{
public:
    RemoveVoidInstances();
};



/** Builder for the steps that detect implicit SysetemC constructs in C++ code
    and replace them with Inferno's Explicit nodes for SystemC which are much
    more succinct in tree form */
class DetectAllSystemC
{
public:
    static void Build( vector< shared_ptr<VNStep> > *sequence );
};

};

#endif
