#ifndef RELATION_TEST_HPP
#define RELATION_TEST_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "../link.hpp"

#include <functional>

namespace SR
{

void TestRelationProperties( function<Orderable::Result(XLink l, XLink r)> compare,
				     		 const unordered_set<XLink> &xlinks );

};

#endif
