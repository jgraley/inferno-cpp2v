#include "df_relation.hpp"

#include "relation_test.hpp"
#include "link_table.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

using namespace SR;

DepthFirstRelation::DepthFirstRelation(const LinkTable *link_table_) :
    link_table( link_table_ )
{
}


Orderable::Diff DepthFirstRelation::Compare3Way( XLink l_xlink, XLink r_xlink ) const
{
	const LinkTable::Row &l_row = link_table->GetRow(l_xlink);
	const LinkTable::Row &r_row = link_table->GetRow(r_xlink);

    return l_row.depth_first_ordinal - r_row.depth_first_ordinal;
}


bool DepthFirstRelation::operator()( XLink l_xlink, XLink r_xlink ) const
{
    return Compare3Way(l_xlink, r_xlink) < 0;
}


void DepthFirstRelation::Test( const unordered_set<XLink> &xlinks )
{
	using namespace std::placeholders;

	TestRelationProperties( xlinks,
                            true,
                            "DepthFirstRelation",
                            [&](){ return Trace(link_table); },                            
                            bind(&DepthFirstRelation::Compare3Way, *this, _1, _2), 
    [&](XLink l, XLink r)
    { 
        return l==r; 
    } );
}

