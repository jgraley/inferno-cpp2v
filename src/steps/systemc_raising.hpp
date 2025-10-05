#ifndef SYSTEMC_RAISING
#define SYSTEMC_RAISING

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
class RaiseSCType : public VNStep 
{
public:
    RaiseSCType( TreePtr< CPPTree::Type > lr_scnode );
};


/// spot SystemC base class by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all class nodes
 that inherit from the supplied base with the new inferno node and 
 remove the base */
class RaiseSCHierarchicalClass : public VNStep 
{
public:
    RaiseSCHierarchicalClass( TreePtr< SCTree::SCRecord > lr_scclass );
};


class RaiseSCDynamic : public VNStep
{
public:
    RaiseSCDynamic( TreePtr<SCTree::SCDynamicFunction> r_dynamic );
};


class RaiseSCStatic : public VNStep
{
public:
    RaiseSCStatic( TreePtr<SCTree::SCFunction> r_static );
};


class RaiseSCDelta : public VNStep
{
public:
    RaiseSCDelta( TreePtr<SCTree::SCFunction> r_delta );
};


/// spot syscall exit() or equivalent function by its name and replace with inferno node 
/** We look for the decl and remove it since the inferno
 Node does not require declaration. Then replace all calls to 
 the function with the explicit statement node. Bring arguments
 across by name match as per Inferno's IdValueMap style. */
class RaiseTerminationFunction : public VNStep
{
public:
    RaiseTerminationFunction( TreePtr<SCTree::TerminationFunction> r_tf );    
};


class RaiseSCProcess : public VNStep
{
public:
    RaiseSCProcess( TreePtr< SCTree::Process > lr_scprocess );
};



/// spot SystemC sc_delta_count() method by its name and replace with inferno node 
/** Look for sc_delta_count() and replace with DeltaCount */
class RaiseSCDeltaCount : public VNStep  
{
public:
    RaiseSCDeltaCount();
};


/// spot SystemC notify() method by its name and replace with inferno node 
/** Look for myevent.notify() and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class RaiseSCNotifyImmediate : public VNStep  
{
public:
    RaiseSCNotifyImmediate();
};


/// spot SystemC notify(SC_ZERO_TIME) method by its name and replace with inferno node 
/** Look for myevent.notify(SC_ZERO_TIME) and replace with Notify->myevent. No need to 
    eliminate the notify decl - that disappeared with the sc_event class */
class RaiseSCNotifyDelta : public VNStep  
{
public:
    RaiseSCNotifyDelta();
};


/// Remove constructors in SC modules that are now empty thanks to earlier steps
/// Must also remove explicit calls to constructor (which would not do anything)
/// in both member init form (l1) and direct form (l2).
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


/// Remove SC global functions from isystemc.h
/** isystemc.h declares global functions to satisfy parser. Hoover them all up
    efficiently here. */
class RemoveSCPrototypes : public VNStep
{
public:
    RemoveSCPrototypes();
};


/** Builder for the steps that detect implicit SysetemC constructs in C++ code
    and replace them with Inferno's Explicit nodes for SystemC which are much
    more succinct in tree form */
class SystemCRaising
{
public:
    static void Build( vector< shared_ptr<VNStep> > *sequence );
};

};

#endif
