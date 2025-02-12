#include "db_common.hpp"
#include "common/standard.hpp"

#include <memory>

using namespace SR;

const shared_ptr<TreePtr<Node>> DBCommon::TreeRecord::IMMUTABLE = nullptr;
