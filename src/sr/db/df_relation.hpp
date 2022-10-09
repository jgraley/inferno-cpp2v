#ifndef DF_RELATION_HPP
#define DF_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "../link.hpp"

#include <memory>

namespace SR
{

class LinkTable;

class DepthFirstRelation
{
public:
    DepthFirstRelation(const LinkTable *link_table);
    Orderable::Diff Compare3Way( XLink l_xlink, XLink r_xlink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink l_xlink, XLink r_xlink ) const;

    void Test( const unordered_set<XLink> &xlinks );
    
private:
	const LinkTable * const link_table;
}; 

};

#endif
