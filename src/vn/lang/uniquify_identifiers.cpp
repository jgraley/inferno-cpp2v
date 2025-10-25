#include "uniquify_identifiers.hpp"
#include "helpers/walk.hpp"
#include "tree/misc.hpp"
#include "helpers/flatten.hpp"
#include "agents/embedded_scr_agent.hpp"

using namespace CPPTree;
using namespace VN;

#define UID_FORMAT_HINT "%s_%u"
//#define UID_FORMAT_PURE "id_%u"


//////////////////////////// UniqueNameGenerator ///////////////////////////////

UniqueNameGenerator::UniqueNameGenerator( string (Syntax::*name_getter_)() const ) :
    name_getter( name_getter_ )
{	
}


string UniqueNameGenerator::AddNode( TreePtr<Node> node )
{
    // Get canonical form of identifier name
    string base_name;
    unsigned n_want;
    SplitName( node, &base_name, &n_want );

    // Do we have the base name already? If so, add this new instance
    if( name_usages.count(base_name) > 0 )
    {
        unsigned n_got = AssignNumber( name_usages.at(base_name), node, n_want );
        return MakeUniqueName( base_name, n_got );
	}

    // Otherwise start a new record for this base name.
    Usages nu;
    unsigned n_got = AssignNumber( nu, node, n_want );
    name_usages.insert( NameUsagesPair( base_name, nu ) );
    return MakeUniqueName( base_name, n_got );
}


void UniqueNameGenerator::AddNodeNoRename( TreePtr<Node> node )
{
	ASSERT( !(node.get()->*name_getter)().empty() ); // GetToken for ids
		
    // Get canonical form of identifier name
    string base_name;
    unsigned n_want;
    SplitName( node, &base_name, &n_want );

    // Undeclared identifiers should already be unique. We must assume they are 
    // declared "somewhere else" and that to rename them would break things. In
    // fact they're probably system node ids. These have the usual identifier 
    // semantics; do a #819 when introducing.
    ASSERT( name_usages.count(base_name) == 0 )
            ("Name conflict among undeclared identifiers (would force a rename - unsafe)\n")
            ("node: ")(node)(" name: ")((node.get()->*name_getter)())("\n")  
            ("previous usages: ")(name_usages); 

    // Otherwise start a new record for this base name.
    Usages nu;
    unsigned n_got = AssignNumber( nu, node, n_want );
    ASSERT( n_got == n_want )( "Undeclared identifier: ")(node)(" would be renamed - unsafe"); 
    name_usages.insert( NameUsagesPair( base_name, nu ) );
}


string UniqueNameGenerator::MakeUniqueName( string b, unsigned n ) const // note static
{
#ifdef UID_FORMAT_HINT 
    if( n>0 )
        return SSPrintf( UID_FORMAT_HINT, b.c_str(), n );
    else
    {
		ASSERT( !b.empty() );
        return b; // n==0 means no change to identifier name; the "_0" is implied in this case
    }
      
#endif
#ifdef UID_FORMAT_PURE 
    (void)b;
    return SSPrintf( UID_FORMAT_PURE, n );
#endif    
}


void UniqueNameGenerator::SplitName( TreePtr<Node> node, string *b, unsigned *n ) const // note static
{
    string original_name = (node.get()->*name_getter)(); // GetToken for ids
    //FTRACE(node)(" has name \"")(original_name)("\"\n");
#ifdef UID_FORMAT_HINT 
    char cb[1024]; // hope that's big enough!
    int c = sscanf( original_name.c_str(), UID_FORMAT_HINT, cb, n ); // TODO maybe add %s at the end to catch junk after the number
    if( c == 2 && *n > 0 ) // note that x_0 is *not* in standard form, so it become eg x_0_1 etc
    {
        *b = string(cb);        
    }        
	else
    {
        *n = original_name.empty() ? 1 : 0;
        *b = original_name;
    }
#endif
#ifdef UID_FORMAT_PURE 
    *n = 1;
    *b = ""; // Have to prevent uniquifier from assuming different basenames are already unique
#endif
}


unsigned UniqueNameGenerator::AssignNumber( Usages &nu, TreePtr<Node> node, unsigned n )
{
    // Uniquify the number n, by incrementing it until there are no conflicts
    bool tryagain;
    do
    {
        // See if the number n is already used
        tryagain = false;
        for( auto p : nu )
            if( p.first == n )
            {
                tryagain = true;
                break;
            }

        // if so, try another number
        if( tryagain )
            n++;

    } while(tryagain);

    // Store the number got
    nu.insert( make_pair(n, node) );

	// Return the number got
	return n;
}

//////////////////////////// Fingerprinter ///////////////////////////////

Fingerprinter::Fingerprinter() :
    comparer( Orderable::REPEATABLE )
{
}


Fingerprinter::NodeSetByFingerprint Fingerprinter::GetNodesInTreeByFingerprint( TreePtr<Node> context )
{
    int index=0;
    ProcessNode( context, index );	
	
    Fingerprinter::NodeSetByFingerprint rfp;
    for( pair< TreePtr<Node>, Fingerprint > p : fingerprints )
    {
        rfp[p.second].insert( p.first );        
    }
    return rfp;
}


void Fingerprinter::ProcessNode( TreePtr<Node> x, int &index )
{
	ASSERT( x );
		
    // Record the fingerprints and increment index in depth-first pre-order
    bool first = fingerprints.count(x)==0;
    fingerprints[x].insert(index);        
    index++;

	// Finding nono-trivial pre-restrictions here so that we don't have to pass
	// TPI's around everywhere in the renderer.
	
	// This function to take TPI for x and determine if the current parent->child
	// path (aka link, aka arrowhead) implies a non-trivial pre-restriction.
	// If so, "or" the info into some data structure (probably just add the node
	// to a set<TP Node> non-solo). Use AgentCommon::IsNonTrivialPreRestriction()
	// which means we'll only do it on Agents - in fact we can only do it on 
	// special agents so use dynamic_cast<const SpecialBase *>(agent). Maybe store
	// a map to const SpecialBase * to be super helpful.

    // Recurse into our child nodes
    // Notw: not worried about repeat visits of arbitrary nodes due to couplings
    // because the purpose of this walk is to determine a fingerprint for
    // repeatable uniquification - if all paths go into the fingerprint, that's
    // good.
    if( first )
	    ProcessChildren( x, index );
}


void Fingerprinter::ProcessChildren( TreePtr<Node> x, int &index )
{
	ASSERT( x );

	// If we see an embedded agent with search and replace patterns equal,
	// we will render that as a single stem, and we don't want to report
	// multiple parents for it which would infer a coupling.
	const Agent *agent = Agent::TryAsAgentConst(x);
	if( agent )
	{
		if( auto emb = dynamic_cast<const EmbeddedSCRAgent *>(agent) )
		{
			if( emb->search_pattern == emb->replace_pattern )
			{
				ProcessSingularNode( &emb->search_pattern, index );
				return;
			}
		}
	}		

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


void Fingerprinter::ProcessSingularNode( const TreePtrInterface *p_x_sing, int &index )
{
	if( *p_x_sing ) // Permitting NULL because patterns
		ProcessNode( (TreePtr<Node>)(*p_x_sing), index );
}


void Fingerprinter::ProcessSequence( SequenceInterface *x_seq, int &index )
{
    for( const TreePtrInterface &x : *x_seq )
    {
        ProcessNode( (TreePtr<Node>)x, index );
    }
}


void Fingerprinter::ProcessCollection( CollectionInterface *x_col, int &index )
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

//////////////////////////// UniquifyNames ///////////////////////////////

UniquifyNames::UniquifyNames( string (Syntax::*name_getter_)() const, bool multiparent_only_, bool preserve_undeclared_ids_ ) :
    name_getter(name_getter_),
    multiparent_only(multiparent_only_),
    preserve_undeclared_ids(preserve_undeclared_ids_)
{
}


UniquifyNames::NodeToNameMap UniquifyNames::UniquifyAll( const TransKit &kit, TreePtr<Node> context ) const
{
	(void)multiparent_only;
	Fingerprinter::NodeSetByFingerprint node_sets_by_fp = Fingerprinter().GetNodesInTreeByFingerprint(context);    
	
	// For repeatability of renders, get a list of identifiers in the tree, ordered:
	// - mainly depth-first, wind-in
	// - collections disambiguated using SimpleCompare
    list< TreePtr<Node> > nodes;
	for( auto p : node_sets_by_fp )
	{
		//ASSERT(p.second.size() == 1)
		//  ("Could not differentiate between these identifiers: ")(p.second)
		//  (" fingerprint ")(p.first)
		//  (". Need to write some more code to uniquify the renders in this case!! (#225)\n");
		// If assert is removed, this loop could iterate more than once; the order
		// of the iterations will not be repeatable, and so id uniquification won't be.
		for( TreePtr<Node> node : p.second ) 
		{
			ASSERT( node );		
			int num_reachings = p.first.size();
			if( multiparent_only && num_reachings == 1 )
				continue;
				
			nodes.push_back( node );
		}
	}
	
	UniqueNameGenerator name_gen( name_getter );
	NodeToNameMap nodes_to_names;

	// Deal with undeclared (system) identifiers which must be preserved
    list< TreePtr<Node> > renamable_nodes;
	for( auto node : nodes )
	{
		ASSERT( node );
		if( preserve_undeclared_ids )
		{
			try
			{		
				if( TreePtr<SpecificIdentifier>::DynamicCast(node) )
				    DeclarationOf().TryApplyTransformation( kit, node );
				renamable_nodes.push_back( node ); // can only rename if there is a decl
			}
			catch(DeclarationOf::DeclarationNotFound &)
			{
				// An undeclared indentifier cannot safely be renamed and so must have a non-empty name
				if( node->GetRenderTerminal().empty() )
					continue;
		
				// Assume undeclared identifier is really a system node identifier.
				// Ensure it will keep its name and not be conflicted, and add to the
				// map so normal IDs don't conflict with it.
				name_gen.AddNodeNoRename( node );
				nodes_to_names.insert( NodeAndNamePair( node, node->GetRenderTerminal() ) );
			}
		}
		else
		{
			renamable_nodes.push_back( node );
		}
	}

    for( TreePtr<Node> node : renamable_nodes )
    {
        string name = name_gen.AddNode( node );
        ASSERT( !name.empty() );
        nodes_to_names.insert( NodeAndNamePair( node, name ) );
    }        
    
    return nodes_to_names;
}

//////////////////////////// UniquifyCompare ///////////////////////////////

UniquifyCompare::UniquifyCompare( const UniquifyNames::NodeToNameMap &unique_nodes_ ) :
    SimpleCompare(Orderable::REPEATABLE),  // Use REPEATABLE but doesn't really matter since we're overriding identifier compare 
    unique_nodes( unique_nodes_ )
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
    string ustr_l = unique_nodes.at(l);
    string ustr_r = unique_nodes.at(r);
    //FTRACE(id_a)(" becomes ")(ustr_a)("\n");
    
    // Compare those. This is like a REPEATABLE SC but using the
    // uniquified names instead.
    return ustr_l.compare(ustr_r);
}


