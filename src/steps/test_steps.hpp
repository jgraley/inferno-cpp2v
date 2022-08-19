/*
 * slave_test
 */

#ifndef SLAVE_TEST_HPP
#define SLAVE_TEST_HPP

#include "sr/vn_step.hpp"

namespace Steps {

using namespace SR;

class SlaveTest : public VNStep
{
public:
	SlaveTest();
};

class SlaveTest2 : public VNStep
{
public:
	SlaveTest2();
};

class SlaveTest3 : public VNStep
{
public:
	SlaveTest3();
};

class FixCrazyNumber : public VNStep // find 29258 and turn it into 3 for #576
{
public:
    FixCrazyNumber();   
};

}; // end namespace

#endif
