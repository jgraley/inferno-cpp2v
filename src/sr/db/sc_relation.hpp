#ifndef SC_RELATION_HPP
#define SC_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "../link.hpp"

#include <memory>

//#define SC_KEY_IS_NODE

class SimpleCompare;

namespace SR
{
class SimpleCompareRelation
{
public:
#ifdef SC_KEY_IS_NODE
	typedef TreePtr<Node> KeyType;
#else	
	typedef XLink KeyType;
#endif	
    SimpleCompareRelation();

    /// Less operator: for use with set, map etc
    bool operator()( KeyType l_key, KeyType r_key ) const;
    Orderable::Diff Compare3Way( KeyType l_key, KeyType r_key ) const;

    void Test( const vector<KeyType> &keys );

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
