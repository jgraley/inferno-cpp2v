
#ifndef EMBEDDED_SCR_TEST_HPP
#define EMBEDDED_SCR_TEST_HPP

#include "vn/vn_step.hpp"

namespace Steps {

using namespace VN;

class EmbeddedSCRTest : public VNStep
{
public:
    EmbeddedSCRTest();
   	bool IsLoweringForRenderStep() const override { return false; }
};

class EmbeddedSCRTest2 : public VNStep
{
public:
    EmbeddedSCRTest2();
   	bool IsLoweringForRenderStep() const override { return false; }
};

class EmbeddedSCRTest3 : public VNStep
{
public:
    EmbeddedSCRTest3();
   	bool IsLoweringForRenderStep() const override { return false; }
};

class FixCrazyNumber : public VNStep // find 29258 and turn it into 3 for #576
{
public:
    FixCrazyNumber();   
   	bool IsLoweringForRenderStep() const override { return false; }
};

class FixCrazyNumberEmb : public VNStep // find 29258 under assign and turn it into 3 for #667
{
public:
    FixCrazyNumberEmb();   
   	bool IsLoweringForRenderStep() const override { return false; }
};

class DroppedTreeZone : public VNStep // #754
{
public:
    DroppedTreeZone();   
   	bool IsLoweringForRenderStep() const override { return false; }
};

}; // end namespace

#endif
