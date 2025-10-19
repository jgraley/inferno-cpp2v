
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

class VisibleNames
{
    // map of basenames to their offset number tables
    typedef pair<const string, Usages> NameUsagesPair;
    map< string, Usages > name_usages;

    static string MakeUniqueName( string b, unsigned n );
    static void SplitName( TreePtr<Node> node, string *b, unsigned *n ); // note static
    unsigned AssignNumber( Usages &nu, TreePtr<Node> node, unsigned n );

public:    
    string AddNode( TreePtr<Node> node );
    void AddNodeNoRename( TreePtr<Node> node );
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

    Fingerprinter();
    
    NodeSetByFingerprint GetNodesInTreeByFingerprint(TreePtr<Node> context);
    void ProcessNode( TreePtr<Node> x, int &index );
    void ProcessChildren( TreePtr<Node> x, int &index );
    void ProcessSingularNode( const TreePtrInterface *p_x_sing, int &index );
    void ProcessSequence( SequenceInterface *x_seq, int &index );
    void ProcessCollection( CollectionInterface *x_col, int &index );    
    
private:
    SimpleCompare comparer;
    map< TreePtr<Node>, Fingerprint > fingerprints;
};


// Main API.
struct UniquifyNames
{
    typedef pair<const TreePtr<Node>, string> IdentifierNamePair;
    typedef map< TreePtr<Node>, string> IdentifierNameMap;
    static IdentifierNameMap UniquifyAll( const TransKit &kit, TreePtr<Node> context, bool relax_about_declarations );
};


// Utility for external use: version of SimpleCompare that uses the uniquified 
// names of the identifiers.
class UniquifyCompare : public SimpleCompare
{
public:
    UniquifyCompare( const UniquifyNames::IdentifierNameMap &unique_ids_ );
    Orderable::Diff Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const override;
    
private:
    const UniquifyNames::IdentifierNameMap &unique_ids;
};
#endif
