#ifndef SC_RELATION_HPP
#define SC_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "../link.hpp"

#include <memory>

class SimpleCompare;

namespace SR
{
class SimpleCompareRelation
{
public:
    SimpleCompareRelation();
    Orderable::Diff Compare3Way( XLink l_xlink, XLink r_xlink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink l_xlink, XLink r_xlink ) const;

    void Test( const unordered_set<XLink> &xlinks );

	// Create a node here so that a regular XLink can be used and passed
	// through the sym stuff by value. Is minimus because intervals on this
	// index are half-open.
	class MinimaxNode : public Node
	{
	public:
		NODE_FUNCTIONS_FINAL
		MinimaxNode( TreePtr<Node> guide, BoundingRole role );
		MinimaxNode(); ///< default constructor, for making archetypes 
		TreePtr<Node> GetGuide() const;
		BoundingRole GetRole() const;
		string GetName() const override;
	private:
		TreePtr<Node> guide;
		BoundingRole role;
	};

private:
    shared_ptr<SimpleCompare> simple_compare;
}; 

};

#endif
