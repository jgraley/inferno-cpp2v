/*
 * slave_test
 */

#ifndef SLAVE_TEST_HPP
#define SLAVE_TEST_HPP

#include "sr/vn_transformation.hpp"

namespace Steps {

using namespace SR;

class SlaveTest : public VNTransformation
{
public:
	SlaveTest();
};

class SlaveTest2 : public VNTransformation
{
public:
	SlaveTest2();
};

class SlaveTest3 : public VNTransformation
{
public:
	SlaveTest3();
};

class FixCrazyNumber : public VNTransformation // find 29258 and turn it into 3 for #576
{
public:
    FixCrazyNumber();   
};

}; // end namespace

#endif
