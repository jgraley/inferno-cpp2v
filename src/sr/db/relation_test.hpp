#ifndef RELATION_TEST_HPP
#define RELATION_TEST_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "../link.hpp"

#include <functional>

namespace SR
{

void TestRelationProperties( const unordered_set<XLink> &xlinks,
                             bool expect_totality,
                             string relation_name, 
                             function<string()> log_on_fail,
                             function<Orderable::Diff(XLink l, XLink r)> compare,
                             function<bool(XLink l, XLink r)> is_equal_native, 
				     		 function<XLink(XLink x, int randval)> get_special = function<XLink(XLink x, int randval)>() );
};

#endif
