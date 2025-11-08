
#ifndef UNIQUIFY_IDENTIFIERS_HPP
#define UNIQUIFY_IDENTIFIERS_HPP

#include <map>
#include <vector>
#include "tree/cpptree.hpp"
#include "common/standard.hpp"
#include "helpers/simple_compare.hpp"
#include "helpers/transformation.hpp"

// Check names for duplication
// Policy is to dedupe with a simple scheme like <name>_<unique number> or even without the underscore
// This or any scheme could clash with existing names. BUT if we bring existing names that happen to be
// the same form into our scheme, as if we created them, then uniqueness is guaranteed (but we will sometimes
// want to change the number, so what started as foo_2 could become foo_3 by the time we're done with it).

typedef map< unsigned, TreePtr<Node> > Usages;

struct UniquifyNames;

class UniqueNameGenerator
{
public:    
	UniqueNameGenerator( string (Syntax::*name_getter_)() const );
    string AddNode( TreePtr<Node> node );
    void AddNodeNoRename( TreePtr<Node> node );

private:    
    string MakeUniqueName( string b, unsigned n ) const;
    void SplitName( TreePtr<Node> node, string *b, unsigned *n ) const;
    unsigned AssignNumber( Usages &nu, TreePtr<Node> node, unsigned n );

	string (Syntax::*name_getter)() const;
    // map of basenames to their offset number tables
    typedef pair<const string, Usages> NameUsagesPair;
    map< string, Usages > name_usages;
};


///
/// Generate a "fingerprint" for each specific node. This captures the 
/// places the node is reached within the code. These "places" are
/// ambiguous in the case of collections, because every member of a collection
/// is nominally at the same "place". So we sort these using a subtree comparer -
/// this allows us to differentiate different-looking subtrees in a collection 
/// as being different places, to the level of detail of the comparer we use.
///
/// Our comparer object is SimpleCompare(REPEATABLE), so we *may* give the same 
/// fingerprints to different identifiers with the same name if the usage patterns
/// look the same (though this happens rarely).
///
/// See #225
///
class Fingerprinter
{
public:
    typedef set<int> Fingerprint;
    typedef map< Fingerprint, set<TreePtr<Node>> > NodeSetByFingerprint;
    typedef map< TreePtr<Node>, set<const TreePtrInterface *> > LinkSetByNode;

    Fingerprinter();
    
    NodeSetByFingerprint GetNodesInTreeByFingerprint(TreePtr<Node> context);
	void ProcessTPI( const TreePtrInterface *tpi, int &index );
    void ProcessNode( TreePtr<Node> x, int &index );
    void ProcessChildren( TreePtr<Node> x, int &index );
    void ProcessSingularItem( const TreePtrInterface *p_x_sing, int &index );
    void ProcessSequence( SequenceInterface *x_seq, int &index );
    void ProcessCollection( CollectionInterface *x_col, int &index );    
    const LinkSetByNode &GetIncomingLinksMap() const;
    
private:
    SimpleCompare comparer;
    map< TreePtr<Node>, Fingerprint > fingerprints;
    LinkSetByNode incoming_links_map;
};


// Main API.
class UniquifyNames
{
public:
    typedef pair<const TreePtr<Node>, string> NodeAndNamePair;
    typedef map< TreePtr<Node>, string> NodeToNameMap;
    typedef Fingerprinter::LinkSetByNode LinkSetByNode;

	UniquifyNames( string (Syntax::*name_getter_)() const, // Method on nodes to get the initial name string
	               bool multiparent_only_,                 // Restrict to nodes with more than one parent
	               bool preserve_undeclared_ids_ );        // Refuse to rename identifiers that have no definition
    NodeToNameMap UniquifyAll( const TransKit &kit, TreePtr<Node> context );
    const LinkSetByNode &GetIncomingLinksMap() const;
                               
private:
    string (Syntax::*name_getter)() const; 
	const bool multiparent_only;          
	const bool preserve_undeclared_ids;	
	Fingerprinter fingerprinter;
};


// Utility for external use: version of SimpleCompare that uses the uniquified 
// names of the identifiers.
class UniquifyCompare : public SimpleCompare
{
public:
    UniquifyCompare( const UniquifyNames::NodeToNameMap &unique_ids_ );
    Orderable::Diff Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const override;
    
private:
    const UniquifyNames::NodeToNameMap &unique_nodes;
};
#endif
