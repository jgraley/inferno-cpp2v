#ifndef XTREE_DATABASE_HPP
#define XTREE_DATABASE_HPP

#include "link.hpp"
#include "common/standard.hpp"
#include "sc_relation.hpp"
#include "../helpers/simple_compare.hpp"
#include "../helpers/transformation.hpp"

#include <unordered_set>

class SimpleCompare;

namespace SYM
{
    class BooleanExpression;
};
    
/// SR namespace contains the search and replace implementation
namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
    
class XTreeDatabase : public Traceable, 
                     public TreeKit
{
public:
    explicit XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses = {} );
    
private: 
    const struct Plan : public Traceable
    {
        Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses );
        
        shared_ptr<Lacing> lacing;
    } plan;

public:
    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink UniquifyDomainExtension( XLink xlink );

    // Get the cannonical xlink for the given one.
    XLink FindDomainExtension( XLink xlink ) const;
    
    const Lacing *GetLacing() const;
    
    enum SubtreeMode
    {
        // Behaviour for main domain population: we will check uniqueness
        // of the XLinks we meet during our recursive walk.
        REQUIRE_SOLO,
        
        // Behaviour for domain extensions. We will continue as long as 
        // nodes are not already in the domain. If a node is in the 
        // domain, we don't recurse into it since everything under it
        // will also be in the domain.
        // https://github.com/jgraley/inferno-cpp2v/issues/213#issuecomment-728266001
        STOP_IF_ALREADY_IN
    };
    
    typedef int IndexType;

    class CategoryRelation
    {
    public:
        CategoryRelation();
        CategoryRelation( shared_ptr<Lacing> lacing );
        CategoryRelation& operator=(const CategoryRelation &other);
        
        bool operator() (const XLink& x_link, const XLink& y_link) const;
    private:
        shared_ptr<Lacing> lacing;
    };

    // Create a node here so that a regular XLink can be used and passed
    // through the sym stuff by value.
    class CategoryMinimaxNode : public Node
    {
    public:
        NODE_FUNCTIONS_FINAL
        CategoryMinimaxNode( int lacing_ordinal );
        CategoryMinimaxNode(); ///< default constructor, for making archetypes 
        int GetLacingOrdinal() const;
        string GetTrace() const override;
    private:
        int lacing_ordinal;
    };

    // Domain ordered by depth-first walk
    // Don't use a vector for this:
    // (a) you'd need the size in advance otherwise the iterators in
    // the xlink_table will go bad while populating and
    // (b) incremental domain update will be hard
    typedef list<XLink> DepthFirstOrderedIndex;    
    typedef DepthFirstOrderedIndex::const_iterator DepthFirstOrderedIt;    
    
    // Category ordering TODO merge with SimpleCompare ordering
    typedef multiset<XLink, CategoryRelation> CategoryOrderedIndex;
    typedef CategoryOrderedIndex::iterator CategoryOrderedIt;

    // We will provide a SimpleCompare ordered version of the domain
    typedef multiset<XLink, SimpleCompareRelation> SimpleCompareOrderedIndex;
    typedef SimpleCompareOrderedIndex::iterator SimpleCompareOrderedIt;
    
    class Row : public Traceable
    {
    public:
        enum ContainmentContext
        {
            ROOT,
            SINGULAR,
            IN_SEQUENCE,
            IN_COLLECTION
        };

        ContainmentContext containment_context;
        
        // Parent X link if not ROOT
        // Note that the parent is unique because:
        // - row is relative to a link, not a node,
        // - multiple parents only allowed at leaf, and parent is 
        //   (at least) one level back from that.
        XLink parent_xlink = XLink();
        
        // Last of the descendents in depth first order. If no 
        // descendents, it will be the current node. 
        XLink last_descendant_xlink = XLink();
        
        // First element of container of which I'm a member. 
        // Defined for all item types.
        XLink my_container_front = XLink();
        XLink my_container_back = XLink();

        // Neighbour elements within my sequence (sequences only)
        XLink my_sequence_predecessor = XLink();
        XLink my_sequence_successor = XLink();

        // Index in a depth-first walk
        IndexType depth_first_ordinal = -1;
        
        // Iterator in a depth-first walk
        DepthFirstOrderedIt depth_first_ordered_it;
        
        // Iterator on my_container that dereferneces to me, if 
        // IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
        // queries.
        ContainerInterface::iterator my_container_it;
               
        string GetTrace() const;
    };
    
    class NodeRow : public Traceable
    {
    public:		
		// Our node is the child of these links.
		set<XLink> parents;

		// Declarative XLinks onto our node. 
		// A subset of parents, so to get the declarer node, you'll need 
		// to use eg Row::parent_xlink.GetChildX(). Why have I done 
		// this? So that this info is unambiguous across parallel links:
		// We'll uniquely specify the correct one if only one is a 
		// declaring link (precision). Taking parent discards that info.
		set<XLink> declarers;
		
        void Merge( const NodeRow &nn );
        string GetTrace() const;
    };

    const Row &GetRow(XLink xlink) const;
    bool HasRow(XLink xlink) const;

    const NodeRow &GetNodeRow(TreePtr<Node> node) const;
    bool HasNodeRow(TreePtr<Node> node) const;

    void Clear();
    void FullBuild( XLink root_xlink );
    void ExtendDomainWorker( PatternLink plink );
    void ExtendDomainNewPattern( PatternLink root_plink );
    void ExtendDomainNewX();
    void UpdateRootXLink(XLink root_xlink);
    void BuildNonIncremental();
    void ClearIncremental(XLink base_xlink);
    void BuildIncremental(XLink base_xlink);
    
private:
    void AddAtRoot( SubtreeMode mode, XLink root_xlink );
    void AddLink( SubtreeMode mode, 
                  XLink xlink, 
                  Row &row,
                  NodeRow &node_row );
    void AddChildren( SubtreeMode mode, XLink xlink );
    void AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink );
    void AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink );
    void AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink );
    
public:
    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // Global domain of possible xlink values - ordered
    DepthFirstOrderedIndex depth_first_ordered_index;            
    
    // Domain ordered by category
    CategoryOrderedIndex category_ordered_index;
    
    // Whole domain in here, grouped by simple compare, findable using eg lower_bound()
    // Should be the other way around, as an indication of policy
    SimpleCompareOrderedIndex simple_compare_ordered_index;
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<SimpleCompareQuotientSet> domain_extension_classes;
    
    // XLink-to-row-of-x_tree_db map
    unordered_map<XLink, Row> xlink_table;

    // Node-to-row-of-x_tree_db map
    map<TreePtr<Node>, NodeRow> node_table;

private:    
    // Depth-first ordering
    int current_ordinal;
    
    // Last node to be reached and given a row
    XLink last_xlink;
    
    // TreeKit implementation
  	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
  	
  	XLink root_xlink;
  	PatternLink root_plink;
};    
    
};

#endif
