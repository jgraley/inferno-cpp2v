#include "uniquify_identifiers.hpp"
#include "helpers/walk.hpp"
#include "tree/misc.hpp"

#define UID_FORMAT "%s_%u"

//TODO Apparently labels do not scope in subblocks like types and instances, but instead are
// always function scope (opbvious when you think about it) so uniquify them accordingly.

string VisibleIdentifiers::MakeUniqueName( string b, unsigned n ) // note static
{
	if( n>0 )
		return SSPrintf( UID_FORMAT, b.c_str(), n );
	else
		return b; // n==0 means no change to identifier name; the "_0" is implied in this case
}


void VisibleIdentifiers::SplitName( TreePtr<SpecificIdentifier> i, string *b, unsigned *n ) // note static
{
	char cb[256]; // hope that's big enough!
	string original_name = string(*i); // Use generic "to-string" in case overloaded
	int c = sscanf( original_name.c_str(), UID_FORMAT, cb, n ); // TODO maybe add %s at the end to catch junk after the number
	if( c == 2 && *n > 0 ) // note that x_0 is *not* in standard form, so it become eg x_0_1 etc
	{
		*b = string(cb);
	}
	else // no match to standard form
	{
		*n = 0;
		*b = original_name;
	}
}


string VisibleIdentifiers::AddIdentifierNumber( NameUsage &nu, TreePtr<SpecificIdentifier> i, string b, unsigned n )
{
	// Uniqueify the number n, by incrementing it until there are no conflicts
	bool tryagain;
	do
	{
		// See if the number n is already used
		tryagain = false;
		FOREACH( unsigned u, nu )
			if( u == n )
			{
				tryagain = true;
				break;
			}

		// if so, try another number
		if( tryagain )
			n++;

	} while(tryagain);

	// Store the number
	nu.insert( n );

	// Return the name that the new identifier should take
	return MakeUniqueName( b, n );
}


string VisibleIdentifiers::AddIdentifier( TreePtr<SpecificIdentifier> i )
{
	// Get canonical form of identifier name
	string b;
	unsigned n;
	SplitName( i, &b, &n );

	// Do we have the base name already? If so, add this new instance
	FOREACH( NameUsagePair &p, name_usages )
	    if( b == p.first )
	       	return AddIdentifierNumber( p.second, i, b, n );

	// Otherwise start a new record for this base name.
	NameUsage nu;
	string nn = AddIdentifierNumber( nu, i, b, n );
	name_usages.insert( NameUsagePair( b, nu ) );
	return nn;
}


void UniquifyIdentifiers::UniquifyScope( TreePtr<Node> root, VisibleIdentifiers v )
{
    bool func = false;
    if( TreePtr<Instance> i = dynamic_pointer_cast<Instance>(root) )
        func = !!dynamic_pointer_cast<Subroutine>(i->type);
    
    if(func)
    {
        // At function level just do everything together - this is the safe option,
        // even with function inside functions. Required for Labels, which are always 
        // whole function scope
        Expand t( root );
        FOREACH( TreePtr<Node> p, t )
        {
            if( TreePtr<Declaration> d = dynamic_pointer_cast<Declaration>(p) )
                if( TreePtr<Identifier> i = GetIdentifierOfDeclaration(d) )
                    if( TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>(i) )
                    {
                        string nn = v.AddIdentifier( si );
                        insert( IdentifierNamePair( si, nn ) );
                    }
        }
    }
    else
    {
        // Above function level (top level, classes etc),
        // look for declarations in the current scope (directly, not in sub scopes).
        Flatten t( root );
        FOREACH( TreePtr<Node> p, t )
        {
            if( TreePtr<Declaration> d = dynamic_pointer_cast<Declaration>(p) )
                if( TreePtr<Identifier> i = GetIdentifierOfDeclaration(d) )
                    if( TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>(i) )
                    {
                        string nn = v.AddIdentifier( si );
                        insert( IdentifierNamePair( si, nn ) );
                    }
        }
        
        // Recurse, to find sub-scopes. Pass v by value so our copy remains the same.
        // This means names must be local. But we will rename more conservatively.        
        FOREACH( TreePtr<Node> p, t )
            UniquifyScope( p, v ); 
    }
}


