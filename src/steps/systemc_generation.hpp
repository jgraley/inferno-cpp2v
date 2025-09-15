#ifndef SYSTEMC_GENERATION
#define SYSTEMC_GENERATION

// These transformations convert C code into SystemC.

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "vn/vn_step.hpp"

namespace Steps {

using namespace VN;

class GlobalScopeToModule : public VNStep 
{
public:
    GlobalScopeToModule();
};


class MainToThread : public VNStep 
{
public:
    MainToThread();
};


/** 
 * Builder for the steps that generate SystemC from C code 
 * 
 * We will only act if no SystemC modules are detected. That tells us
 * that the input program is not SC, so must be C. We create
 * an SC module called GlobalScope and move the free functions
 * (including main()) into it.
 * */
class GenerateSC
{
public:
    static void Build( vector< shared_ptr<VNStep> > *sequence );
};

};

#endif
