
#ifndef EMBEDDED_SCR_TEST_HPP
#define EMBEDDED_SCR_TEST_HPP

#include "vn/vn_step.hpp"

namespace Steps {

using namespace SR;

class EmbeddedSCRTest : public VNStep
{
public:
    EmbeddedSCRTest();
};

class EmbeddedSCRTest2 : public VNStep
{
public:
    EmbeddedSCRTest2();
};

class EmbeddedSCRTest3 : public VNStep
{
public:
    EmbeddedSCRTest3();
};

class FixCrazyNumber : public VNStep // find 29258 and turn it into 3 for #576
{
public:
    FixCrazyNumber();   
};

class FixCrazyNumberEmb : public VNStep // find 29258 under assign and turn it into 3 for #667
{
public:
    FixCrazyNumberEmb();   
};

class DroppedTreeZone : public VNStep // #754
{
public:
    DroppedTreeZone();   
};


}; // end namespace

#endif
