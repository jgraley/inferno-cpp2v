#ifndef SYSTEMC_LOWERING
#define SYSTEMC_LOWERING

// These transformations lower SystemC nodes into somehting we can render.

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/vn_step.hpp"

namespace Steps {

using namespace VN;

/// Spot SystemC type-like nodes and replace by user type with the required name.
class LowerSCType : public VNStep 
{
public:
    LowerSCType( TreePtr< CPPTree::Type > s_scnode );
};


class EnsureConstructorsInSCRecordUsers : public VNStep 
{
public:
	EnsureConstructorsInSCRecordUsers();
};

class LowerSCHierarchicalClass : public VNStep 
{
public:
    LowerSCHierarchicalClass( TreePtr< SCTree::SCRecord > s_scclass );
};


class LowerSCDynamic : public VNStep
{
public:
    LowerSCDynamic( TreePtr<SCTree::SCDynamicFunction> s_dynamic,
                    TreePtr<CPPTree::InstanceIdentifier> r_dest );
};


class LowerSCStatic : public VNStep
{
public:
    LowerSCStatic( TreePtr<SCTree::SCFunction> s_dynamic,
                   TreePtr<CPPTree::InstanceIdentifier> r_dest );
};


class LowerSCDelta : public VNStep
{
public:
    LowerSCDelta( TreePtr<SCTree::SCFunction> s_delta,
                  TreePtr<CPPTree::InstanceIdentifier> r_dest,
                  TreePtr<CPPTree::InstanceIdentifier> zero_time_id );
};


class LowerTerminationFunction : public VNStep
{
public:
    LowerTerminationFunction( TreePtr<SCTree::TerminationFunction> s_tf );
};


class LowerSCProcess : public VNStep
{
public:
    LowerSCProcess( TreePtr< SCTree::Process > s_scprocess );
};


class LowerSCNotifyImmediate : public VNStep
{
public:
    LowerSCNotifyImmediate();
};


class LowerSCNotifyDelta : public VNStep
{
public:
    LowerSCNotifyDelta( TreePtr<CPPTree::InstanceIdentifier> zero_time_id );
};


class LowerSCDeltaCount : public VNStep
{
public:
    LowerSCDeltaCount();
};


class AddIncludeSystemC : public VNStep
{
public:
    AddIncludeSystemC();
};


class AddIncludeSCExtensions : public VNStep
{
public:
    AddIncludeSCExtensions();
};


/** Builder for the steps that lowers Inferno's Explicit nodes for SystemC 
    into implicit SysetemC constructs in C++ code for rendering using a
    C/C++ renderer. */
class SystemCLowering
{
public:
    static void Build( vector< shared_ptr<VNStep> > *sequence );
};

};

#endif
