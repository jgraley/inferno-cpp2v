
#ifndef UNIQUIFY_IDENTIFIERS_HPP
#define UNIQUIFY_IDENTIFIERS_HPP

#include <map>
#include <vector>
#include "tree/cpptree.hpp"
#include "common/standard.hpp"
#include "helpers/simple_compare.hpp"

// Check identifiers for duplication
// Policy is to dedupe with a simple scheme like <name>_<unique number> or even without the underscore
// This or any scheme could clash with existing names. BUT if we bring existing names that happen to be
// the same form into our scheme, as if we created them, then uniqueness is guaranteed (but we will sometimes
// want to change the number, so what started as foo_2 could become foo_3 by the time we're done with it).

typedef set< unsigned > NameUsage;

struct UniquifyIdentifiers;

struct VisibleIdentifiers
{
	// map of basenames to their offset number tables
	typedef pair<const string, NameUsage> NameUsagePair;
	map< string, NameUsage > name_usages;

	string AddIdentifierNumber( NameUsage &nu, TreePtr<CPPTree::SpecificIdentifier> i, string b, unsigned n );
	string AddIdentifier( TreePtr<CPPTree::SpecificIdentifier> i );

	static string MakeUniqueName( string b, unsigned n );
	static void SplitName( TreePtr<CPPTree::SpecificIdentifier> i, string *b, unsigned *n ); // note static
};


class UniquifyCompare : public SimpleCompare
{
public:
    UniquifyCompare( const UniquifyIdentifiers *unique );
    virtual Orderable::Result Compare( TreePtr<Node> a, TreePtr<Node> b ) const override;
    
private:
    const UniquifyIdentifiers * const unique;
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

    IdentifierFingerprinter( TreePtr<Node> root_x );
    
    void ProcessNode( TreePtr<Node> x, int &index );
    void ProcessChildren( TreePtr<Node> x, int &index );
    void ProcessSingularNode( const TreePtrInterface *p_x_sing, int &index );
    void ProcessSequence( SequenceInterface *x_seq, int &index );
    void ProcessCollection( CollectionInterface *x_col, int &index );    
    map< TreePtr<CPPTree::SpecificIdentifier>, Fingerprint > GetFingerprints();
    map< Fingerprint, set<TreePtr<CPPTree::SpecificIdentifier>> > GetReverseFingerprints();
    
private:
    SimpleCompare comparer;
    map< TreePtr<CPPTree::SpecificIdentifier>, Fingerprint > fingerprints;
};


struct UniquifyIdentifiers : public map< TreePtr<CPPTree::SpecificIdentifier>, string >
{
	typedef pair<const TreePtr<CPPTree::SpecificIdentifier>, string> IdentifierNamePair;
	void UniquifyScope( TreePtr<Node> root, VisibleIdentifiers v = VisibleIdentifiers() ); // Not a ref because we want a copy so we can go back TODO optimise
};


#endif
