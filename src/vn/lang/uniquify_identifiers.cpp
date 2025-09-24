#include "uniquify_identifiers.hpp"
#include "helpers/walk.hpp"
#include "tree/misc.hpp"
#include "helpers/flatten.hpp"

using namespace CPPTree;

#define UID_FORMAT_HINT "%s_%u"
//#define UID_FORMAT_PURE "id_%u"


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


unsigned VisibleIdentifiers::AssignNumber( NameUsage &nu, TreePtr<SpecificIdentifier>, unsigned n )
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

    // Store the number got
    nu.insert( n );

	// Return the number got
	return n;
}


string VisibleIdentifiers::AddIdentifier( TreePtr<SpecificIdentifier> i )
{
    // Get canonical form of identifier name
    string base_name;
    unsigned n_want;
    SplitName( i, &base_name, &n_want );

    // Do we have the base name already? If so, add this new instance
    for( NameUsagePair &p : name_usages )
    {
        if( base_name == p.first )
        {
            unsigned n_got = AssignNumber( p.second, i, n_want );
            return MakeUniqueName( base_name, n_got );
		}
	}

    // Otherwise start a new record for this base name.
    NameUsage nu;
    unsigned n_got = AssignNumber( nu, i, n_want );
    name_usages.insert( NameUsagePair( base_name, nu ) );
    return MakeUniqueName( base_name, n_got );
}

//////////////////////////// IdentifierFingerprinter ///////////////////////////////

IdentifierFingerprinter::IdentifierFingerprinter() :
    comparer( Orderable::REPEATABLE )
{
}


IdentifierFingerprinter::IdsByFingerprint IdentifierFingerprinter::GetIdentifiersInTreeByFingerprint( TreePtr<Node> root_x )
{
    int index=0;
    ProcessNode( root_x, index );	
	
    map< Fingerprint, set<TreePtr<CPPTree::SpecificIdentifier>> > rfp;
    for( pair< TreePtr<SpecificIdentifier>, Fingerprint > p : fingerprints )
    {
        rfp[p.second].insert( p.first );        
    }
    return rfp;
}


void IdentifierFingerprinter::ProcessNode( TreePtr<Node> x, int &index )
{
	ASSERT( x );
    // Record the fingerprints and increment index in depth-first pre-order
    if( auto id_x = TreePtr<SpecificIdentifier>::DynamicCast(x) )
        fingerprints[id_x].insert(index);
        
    index++;

    // Recurse into our child nodes
    ProcessChildren( x, index );
}


void IdentifierFingerprinter::ProcessChildren( TreePtr<Node> x, int &index )
{
	ASSERT( x );
    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
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
	//if( !*p_x_sing )
	//	return; // NULL singular indicates wildcard
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
    int prev_start_index = 0;
    for( TreePtr<Node> x : comparer.GetTreePtrOrdering(*x_col) )
    {
        if( prev_x && comparer.Compare3Way(x, prev_x) == 0 )
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

//////////////////////////// UniquifyIdentifiers ///////////////////////////////

UniquifyIdentifiers::IdentifierNameMap UniquifyIdentifiers::UniquifyAll( TreePtr<Node> root )
{
	IdentifierFingerprinter::IdsByFingerprint ids_by_fp = IdentifierFingerprinter().GetIdentifiersInTreeByFingerprint(root);    
	
	// For repeatability of renders, get a list of identifiers in the tree, ordered:
	// - mainly depth-first, wind-in
	// - collections disambiguated using SimpleCompare
    list< TreePtr<SpecificIdentifier> > ids;
	for( auto p : ids_by_fp )
	{
		//ASSERT(p.second.size() == 1)
		//  ("Could not differentiate between these identifiers: ")(p.second)
		//  (" fingerprint ")(p.first)
		//  (". Need to write some more code to uniquify the renders in this case!! (#225)\n");
		// If assert is removed, this loop could iterate more than once; the order
		// of the iterations will not be repeatable, and so id uniquification won't be.
		for( TreePtr<SpecificIdentifier> si : p.second ) 
			ids.push_back( si );
	}
	
    list< TreePtr<SpecificIdentifier> > renamable_ids;
	for( auto id : ids )
	{
		try
		{
			//DeclarationOf().TryApplyTransformation( kit, id );
			renamable_ids.push_back( id ); // can only rename if there is a decl
		}
		catch(DeclarationOf::DeclarationNotFound &)
		{
			
		}
	}

	VisibleIdentifiers vi;
	IdentifierNameMap inm;
    for( TreePtr<SpecificIdentifier> si : renamable_ids )
    {
        string nn = vi.AddIdentifier( si );
        inm.insert( IdentifierNamePair( si, nn ) );
    }        
    
    return inm;
}

//////////////////////////// UniquifyCompare ///////////////////////////////

UniquifyCompare::UniquifyCompare( const UniquifyIdentifiers::IdentifierNameMap &unique_ids_ ) :
    SimpleCompare(Orderable::REPEATABLE),  // Use REPEATABLE but doesn't really matter since we're overriding identifier compare 
    unique_ids( unique_ids_ )
{
}


Orderable::Diff UniquifyCompare::Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const
{
    //FTRACE("UC::Compare ")(a)(" - ")(b)("\n");
    
    // We're overriding the node entrypoint of SimpleCompare. If we're not
    // dealing with two SpecificIdentifiers, call back into that function
    // explicitly to get normal compare behaviour.
    auto id_l = TreePtr<SpecificIdentifier>::DynamicCast(l);
    auto id_r = TreePtr<SpecificIdentifier>::DynamicCast(r);
    if( !(id_l && id_r) )
        return SimpleCompare::Compare3Way(l, r);
        
    // We have two SpecificIdentifiers, so get their unique names
    string ustr_l = unique_ids.at(id_l);
    string ustr_r = unique_ids.at(id_r);
    //FTRACE(id_a)(" becomes ")(ustr_a)("\n");
    
    // Compare those. This is like a REPEATABLE SC but using the
    // uniquified names instead.
    return ustr_l.compare(ustr_r);
}


