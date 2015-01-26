
#ifndef UNIQUIFY_IDENTIFIERS_HPP
#define UNIQUIFY_IDENTIFIERS_HPP

#include <map>
#include <vector>
#include "tree/cpptree.hpp"
#include "common/standard.hpp"

// Check identifiers for duplication
// Policy is to dedupe with a simple scheme like <name>_<unique number> or even without the underscore
// This or any scheme could clash with existing names. BUT if we bring existing names that happen to be
// the same form into our scheme, as it we created them, then uniqueness is guaranteed (but we will sometimes
// want to change the number, so what started as foo_2 could become foo_3 by the time we're done with it).

typedef Set< unsigned > NameUsage;

struct UniquifyIdentifiers;

struct VisibleIdentifiers
{
	// Map of basenames to their offset number tables
	typedef pair<const string, NameUsage> NameUsagePair;
	Map< string, NameUsage > name_usages;

	string AddIdentifierNumber( NameUsage &nu, TreePtr<CPPTree::SpecificIdentifier> i, string b, unsigned n );
	string AddIdentifier( TreePtr<CPPTree::SpecificIdentifier> i );

	static string MakeUniqueName( string b, unsigned n );
	static void SplitName( TreePtr<CPPTree::SpecificIdentifier> i, string *b, unsigned *n ); // note static
};

struct UniquifyIdentifiers : public Map< TreePtr<CPPTree::SpecificIdentifier>, string >
{
	typedef pair<const TreePtr<CPPTree::SpecificIdentifier>, string> IdentifierNamePair;
	void UniquifyScope( TreePtr<Node> root, VisibleIdentifiers v = VisibleIdentifiers() ); // Not a ref because we want a copy so we can go back TODO optimise
};


#endif
