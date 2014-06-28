#ifndef SYSTEMC_DETECTION
#define SYSTEMC_DETECTION

// These transformations detect SystemC constructs by identifier 
// name and node pattern within input code, and substitute the 
// Inferno SystemC nodes.

#include "sr/search_replace.hpp"
#include "sr/soft_patterns.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

namespace Steps {

using namespace SR;

/** Holder for the steps that detect implicit SysetemC constructs in C++ code
    and replace them with Inferno's Explicit nodes for SystemC which are much
    more succinct in tree form */
class DetectAllSCTypes : public TransformationVector // TODO make generic version
{
public:
    DetectAllSCTypes();
};

};

#endif
