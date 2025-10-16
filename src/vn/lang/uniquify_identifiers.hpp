
#ifndef UNIQUIFY_IDENTIFIERS_HPP
#define UNIQUIFY_IDENTIFIERS_HPP

#include <map>
#include <vector>
#include "tree/cpptree.hpp"
#include "common/standard.hpp"
#include "helpers/simple_compare.hpp"
#include "helpers/transformation.hpp"

// Check identifiers for duplication
// Policy is to dedupe with a simple scheme like <name>_<unique number> or even without the underscore
// This or any scheme could clash with existing names. BUT if we bring existing names that happen to be
// the same form into our scheme, as if we created them, then uniqueness is guaranteed (but we will sometimes
// want to change the number, so what started as foo_2 could become foo_3 by the time we're done with it).

typedef map< unsigned, TreePtr<CPPTree::SpecificIdentifier> > NameUsage;

struct UniquifyIdentifiers;

class VisibleIdentifiers
{
    // map of basenames to their offset number tables
    typedef pair<const string, NameUsage> NameUsagePair;
    map< string, NameUsage > name_usages;

    static string MakeUniqueName( string b, unsigned n );
    static void SplitName( TreePtr<CPPTree::SpecificIdentifier> i, string *b, unsigned *n ); // note static
    unsigned AssignNumber( NameUsage &nu, TreePtr<CPPTree::SpecificIdentifier> i, unsigned n );

public:    
    string AddIdentifier( TreePtr<CPPTree::SpecificIdentifier> i );
    void AddUndeclaredIdentifier( TreePtr<CPPTree::SpecificIdentifier> i );
};


///
/// Generate a "fingerprint" for each specific identifier node. This captures the 
/// places the identifier is referenced within the code. These "places" are
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
class IdentifierFingerprinter
{
public:
    typedef set<int> Fingerprint;
    typedef map< Fingerprint, set<TreePtr<CPPTree::SpecificIdentifier>> > IdsByFingerprint;

    IdentifierFingerprinter();
    
    IdsByFingerprint GetIdentifiersInTreeByFingerprint(TreePtr<Node> context);
    void ProcessNode( TreePtr<Node> x, int &index );
    void ProcessChildren( TreePtr<Node> x, int &index );
    void ProcessSingularNode( const TreePtrInterface *p_x_sing, int &index );
    void ProcessSequence( SequenceInterface *x_seq, int &index );
    void ProcessCollection( CollectionInterface *x_col, int &index );    
    
private:
    SimpleCompare comparer;
    map< TreePtr<CPPTree::SpecificIdentifier>, Fingerprint > fingerprints;
};


// Main API.
struct UniquifyIdentifiers
{
    typedef pair<const TreePtr<CPPTree::SpecificIdentifier>, string> IdentifierNamePair;
    typedef map< TreePtr<CPPTree::SpecificIdentifier>, string> IdentifierNameMap;
    static IdentifierNameMap UniquifyAll( const TransKit &kit, TreePtr<Node> context, bool relax_about_declarations );
};


// Utility for external use: version of SimpleCompare that uses the uniquified 
// names of the identifiers.
class UniquifyCompare : public SimpleCompare
{
public:
    UniquifyCompare( const UniquifyIdentifiers::IdentifierNameMap &unique_ids_ );
    Orderable::Diff Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const override;
    
private:
    const UniquifyIdentifiers::IdentifierNameMap &unique_ids;
};
#endif
