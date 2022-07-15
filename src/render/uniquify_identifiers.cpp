#include "uniquify_identifiers.hpp"
#include "helpers/walk.hpp"
#include "tree/misc.hpp"
#include "helpers/flatten.hpp"

using namespace CPPTree;

//#define UID_FORMAT_HINT "%s%u"
#define UID_FORMAT_PURE "id_%u"

//////////////////////////// VisibleIdentifiers ///////////////////////////////

string VisibleIdentifiers::MakeUniqueName( string b, unsigned n ) // note static
{
#ifdef UID_FORMAT_HINT 
	if( n>0 )
		return SSPrintf( UID_FORMAT_HINT, b.c_str(), n );
	else
		return b; // n==0 means no change to identifier name; the "_0" is implied in this case
#endif
#ifdef UID_FORMAT_PURE 
    (void)b;
    return SSPrintf( UID_FORMAT_PURE, n );
#endif    
}


void VisibleIdentifiers::SplitName( TreePtr<SpecificIdentifier> i, string *b, unsigned *n ) // note static
{
	string original_name = i->GetRender();
#ifdef UID_FORMAT_HINT 
	char cb[1024]; // hope that's big enough!
	int c = sscanf( original_name.c_str(), UID_FORMAT_HINT, cb, n ); // TODO maybe add %s at the end to catch junk after the number
	if( c == 2 && *n > 0 ) // note that x_0 is *not* in standard form, so it become eg x_0_1 etc
	{
		*b = string(cb);        
	}    
    else
    {
        *n = 0;
        *b = original_name;
    }
#endif
#ifdef UID_FORMAT_PURE 
    *n = 0;
    *b = ""; // Have to prevent uniquifier from assuming different basenames are already unique
#endif
}


string VisibleIdentifiers::AddIdentifierNumber( NameUsage &nu, TreePtr<SpecificIdentifier> i, string b, unsigned n )
{
	// Uniqueify the number n, by incrementing it until there are no conflicts
	bool tryagain;
	do
	{
		// See if the number n is already used
		tryagain = false;
		for( unsigned u : nu )
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
	for( NameUsagePair &p : name_usages )
	    if( b == p.first )
	       	return AddIdentifierNumber( p.second, i, b, n );

	// Otherwise start a new record for this base name.
	NameUsage nu;
	string nn = AddIdentifierNumber( nu, i, b, n );
	name_usages.insert( NameUsagePair( b, nu ) );
	return nn;
}

//////////////////////////// UniquifyCompare ///////////////////////////////

UniquifyCompare::UniquifyCompare( const UniquifyIdentifiers *unique_ ) :
    SimpleCompare(Orderable::REPEATABLE),  // Use REPEATABLE but doesn't really matter since we're overriding identifier compare 
    unique( unique_ )
{
}


Orderable::Result UniquifyCompare::Compare( TreePtr<Node> a, TreePtr<Node> b ) const
{
    //FTRACE("UC::Compare ")(a)(" - ")(b)("\n");
    
    // We're ovreridden the node entrypoint of SimpleCompare. If we're not
    // dealing with two SpecificIdentifiers, call back into that function
    // explicitly to get normal compare behaviour.
    auto id_a = TreePtr<SpecificIdentifier>::DynamicCast(a);
    auto id_b = TreePtr<SpecificIdentifier>::DynamicCast(b);
    if( !(id_a && id_b) )
        return SimpleCompare::Compare(a, b);
        
    // We have two SpecificIdentifiers, so get their unique names
    string ustr_a = unique->at(id_a);
    string ustr_b = unique->at(id_b);
    //FTRACE(id_a)(" becomes ")(ustr_a)("\n");
    
    // Compare those. This is like a REPEATABLE SC but using the
    // uniquified names instead.
    return ustr_a.compare(ustr_b);
}

//////////////////////////// IdentifierFingerprinter ///////////////////////////////

IdentifierFingerprinter::IdentifierFingerprinter( TreePtr<Node> root_x ) :
    comparer( Orderable::REPEATABLE )
{
    int index=0;
    ProcessNode( root_x, index );
}


void IdentifierFingerprinter::ProcessNode( TreePtr<Node> x, int &index )
{
    // Record the fingerprints and increment index on the way down
    if( auto id_x = TreePtr<SpecificIdentifier>::DynamicCast(x) )
        fingerprints[id_x].insert(index);
    index++;

    // Recurse into our child nodes
    ProcessChildren( x, index );
}


void IdentifierFingerprinter::ProcessChildren( TreePtr<Node> x, int &index )
{
    vector< Itemiser::Element * > x_memb = x->Itemise();
    for( Itemiser::Element *xe : x_memb )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            ProcessSequence( x_seq, index );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            ProcessCollection( x_col, index );
        else if( TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(xe) )
            ProcessSingularNode( p_x_sing, index );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void IdentifierFingerprinter::ProcessSingularNode( const TreePtrInterface *p_x_sing, int &index )
{
    ProcessNode( (TreePtr<Node>)(*p_x_sing), index );
}


void IdentifierFingerprinter::ProcessSequence( SequenceInterface *x_seq, int &index )
{
    for( const TreePtrInterface &x : *x_seq )
    {
        ProcessNode( (TreePtr<Node>)x, index );
    }
}


void IdentifierFingerprinter::ProcessCollection( CollectionInterface *x_col, int &index )
{
    TreePtr<Node> prev_x;
    int index_offset = 0;
    int prev_start_index;
    for( TreePtr<Node> x : comparer.GetOrdering(*x_col) )
    {
        if( prev_x && comparer.Compare(x, prev_x) == Orderable::EQUAL )
        {
            // Our comparer cannot differentiate this subtree from the 
            // previous one, so "replay" the same indexes while traversing it.  
            int temp_start_index = prev_start_index;
            int temp_index = temp_start_index;
            ProcessNode( (TreePtr<Node>)x, temp_index );
            
            // But keep updating the "real" index so we get the same
            // values afterwards (as if this one had not compared equal).
            index += temp_index - temp_start_index;
        }
        else
        {
            prev_start_index = index;
            ProcessNode( (TreePtr<Node>)x, index );        
        }
            
        prev_x = x;
    }
}


map< TreePtr<SpecificIdentifier>, IdentifierFingerprinter::Fingerprint > IdentifierFingerprinter::GetFingerprints()
{
    return fingerprints;
}


map< IdentifierFingerprinter::Fingerprint, set<TreePtr<CPPTree::SpecificIdentifier>> > IdentifierFingerprinter::GetReverseFingerprints()
{
    map< Fingerprint, set<TreePtr<CPPTree::SpecificIdentifier>> > rfp;
    for( pair< TreePtr<SpecificIdentifier>, Fingerprint > p : fingerprints )
    {
        rfp[p.second].insert( p.first );        
    }
    return rfp;
}


//////////////////////////// UniquifyIdentifiers ///////////////////////////////

void UniquifyIdentifiers::UniquifyScope( TreePtr<Node> root, VisibleIdentifiers v )
{
    IdentifierFingerprinter idfp( root );    

    list< TreePtr<SpecificIdentifier> > ids;
    for( auto p : idfp.GetReverseFingerprints() )
    {
        /*ASSERT(p.second.size() == 1)
          ("Could not differentiate between these identifiers: ")(p.second)
          (" fingerprint ")(p.first)
          (". Need to write some more code to uniquify the renders in this case!! (#225)\n");*/
        // If assert is removed, this loop could iterate more than once; the order
        // of the iterations will not be repeatable, and so id uniquification won't be.
        for( TreePtr<SpecificIdentifier> si : p.second ) // TODO change me!
            ids.push_back( si );
    }

    for( TreePtr<SpecificIdentifier> si : ids )
    {
        string nn = v.AddIdentifier( si );
        insert( IdentifierNamePair( si, nn ) );
    }        
}
