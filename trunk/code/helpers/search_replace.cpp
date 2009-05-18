#include "search_replace.hpp"

// Constructor remembers search pattern, replace apttern and any suppleid match sets as required
SearchReplace::SearchReplace( shared_ptr<Node> sp, 
                              shared_ptr<Node> rp,
                              const set<MatchSet> *m ) :
    search_pattern( sp ),
    replace_pattern( rp ),
    matches( m )
{  
    our_matches = !matches;
    if( our_matches )    
        matches = new set<MatchSet>;
    
    ASSERT( matches );        
}


// Destructor tries not to leak memory lol
SearchReplace::~SearchReplace()
{
    if( our_matches )    
        delete matches;
}


// Helper for IsMatchPattern that does the testing for the present node, including 
// superclass wildcarding and data member checking on Property nodes.
bool SearchReplace::IsMatchPatternLocal( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    TRACE();
    ASSERT( pattern ); // Disallow NULL pattern for now, could change this if required
    ASSERT( x ); // No NULL in tree

    // Is node correct class?
    TRACE("Is %s >= %s? ", TypeInfo(pattern).name().c_str(), TypeInfo(x).name().c_str() );
    if( !(TypeInfo(pattern) >= TypeInfo(x)) ) // Note >= is "non-strict superset" i.e. pattern is superclass of x or same class
    {
        TRACE("lol no!\n" );
        return false;
    }

    if( shared_ptr<String> pattern_str = dynamic_pointer_cast<String>(pattern) )
    {
        shared_ptr<String> x_str = dynamic_pointer_cast<String>(x);
        ASSERT( x_str );
        if( x_str->value != pattern_str->value )
        {
            TRACE("Strings differ\n");
            return false;
        }
    }    
    else if( shared_ptr<Integer> pattern_int = dynamic_pointer_cast<Integer>(pattern) )
    {
        shared_ptr<Integer> x_int = dynamic_pointer_cast<Integer>(x);
        ASSERT( x_int );
        TRACE("%s %s\n", x_int->value.toString(10).c_str(), pattern_int->value.toString(10).c_str() );
        if( x_int->value != pattern_int->value )
        {
            TRACE("Integers differ\n");
            return false;
        }
    }    
    else if( shared_ptr<Float> pattern_flt = dynamic_pointer_cast<Float>(pattern) )
    {
        shared_ptr<Float> x_flt = dynamic_pointer_cast<Float>(x);
        ASSERT( x_flt );
        if( !x_flt->value.bitwiseIsEqual( pattern_flt->value ) )
        {
            TRACE("Floats differ\n");
            return false;
        }
    }
    else if( shared_ptr<FloatSemantics> pattern_sem = dynamic_pointer_cast<FloatSemantics>(pattern) )
    {
        shared_ptr<FloatSemantics> x_sem = dynamic_pointer_cast<FloatSemantics>(x);
        ASSERT( x_sem );
        if( x_sem->value != pattern_sem->value )
        {
            TRACE("Float semantics differ\n");
            return false;
        }
    }
    TRACE("yes!!\n");
    return true;
}    


// Helper for IsMatchPattern that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
bool SearchReplace::IsMatchPatternNoKey( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    ASSERT( !!pattern ); // Disallow NULL pattern for now, could change this if required
    
    if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
        return ssp->IsMatchPattern( this, x );

    if( !IsMatchPatternLocal( x, pattern ) )
        return false;
    
    // recurse children (or we have a match)
    vector< Itemiser::Element * > pattern_memb = Itemiser::Itemise( pattern.get() ); 
    vector< Itemiser::Element * > x_memb = Itemiser::Itemise( x.get(),           // The thing we're itemising
                                                              pattern.get() );   // Just get the members corresponding to pattern's class
    ASSERT( pattern_memb.size() == x_memb.size() );
    
    for( int i=0; i<pattern_memb.size(); i++ )
    {
        ASSERT( pattern_memb[i] && "itemise returned null element");
        ASSERT( x_memb[i] && "itemise returned null element");
        
        if( GenericSequence *pattern_seq = dynamic_cast<GenericSequence *>(pattern_memb[i]) )                
        {
            GenericSequence *x_seq = dynamic_cast<GenericSequence *>(x_memb[i]);
            ASSERT( x_seq && "itemise for target didn't match itemise for pattern");
            TRACE("Member %d is Sequence, target %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
            if( x_seq->size() != pattern_seq->size() )
                return false;
            
            for( int j=0; j<pattern_seq->size(); j++ )
            {
                TRACE("Elt %d target ptr=%p pattern ptr=%p\n", j, x_seq->Element(j).Get().get(), pattern_seq->Element(j).Get().get());
                if( !pattern_seq->Element(j).Get() )
                    continue; // NULL is a wildcard in search patterns
                TRACE();
                bool match = IsMatchPattern( x_seq->Element(j).Get(), pattern_seq->Element(j).Get() );
                if( !match )
                    return false;                    
            }
        }            
        else if( GenericSharedPtr *pattern_ptr = dynamic_cast<GenericSharedPtr *>(pattern_memb[i]) )         
        {
            GenericSharedPtr *x_ptr = dynamic_cast<GenericSharedPtr *>(x_memb[i]);
            ASSERT( x_ptr && "itemise for target didn't match itemise for pattern");
            TRACE("Member %d is SharedPtr, pattern ptr=%p\n", i, pattern_ptr->Get().get());
            if( pattern_ptr->Get() ) // NULL is a wildcard in search patterns  
            {                   
                bool match = IsMatchPattern( x_ptr->Get(), pattern_ptr->Get() );
                if( !match )
                    return false;                     
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a sequence or a shared pointer");               
        }
    }       
   
    TRACE("Matches search pattern\n");
    return true;
}


// Try to match a pattern with the inferno rules: soft patterns allowed to
// determine own match result, match sets restrict to same actual node. Also
// keys the match sets as matches are found.
bool SearchReplace::IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    ASSERT( !!pattern ); // Disallow NULL pattern for now, could change this if required
    
    if( !IsMatchPatternNoKey( x, pattern ) )
        return false;
    
    // If we got here, the node matched the search pattern. Now apply match sets
    const MatchSet *m = FindMatchSet( pattern );
    if( m )
    {
        // It's in a match set!!
        if( m->key )
        {
            // This match set has already been keyed!!
            // No need to recurse, since we already did IsMatchPatternNoKey() and passed - we
            // only need to restrict the search based on the current node, not its children.
            if( !IsMatchPatternLocal( x, m->key ) )
                return false;            
        }
        else
        {
            // Not keyed yet, so key it now!!!
            m->key = x;
        }
    }    
    
    return true;
}


// Search supplied program for a match to the configured search pattern.
// If found, return double pointer to assist replace algorithm.
GenericSharedPtr *SearchReplace::Search( shared_ptr<Node> program )
{
    Walk w( program );
    while(!w.Done())
    {
        shared_ptr<Node> x = w.Get();
        ClearKeys();
        if( IsMatchPattern( x, search_pattern ) )
            return w.GetGeneric();                            
        w.Advance(); 
    }    
    
    return NULL;
}


// Clear all pointer members in supplied dest to NULL
void SearchReplace::ClearPtrs( shared_ptr<Node> dest )
{
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get() );
    for( int i=0; i<dest_memb.size(); i++ )
    {
        if( GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]) )                
        {
            for( int j=0; j<dest_seq->size(); j++ )
                dest_seq->Element(j).Set( shared_ptr<Node>() );
        }            
        else if( GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]) )         
        {
            dest_ptr->Set( shared_ptr<Node>() );
        }
    }       
}


// Helper for DuplicateSubtree, fills in children of dest node from source node when source node child
// is non-NULL. This means we can call this multiple times with different sources and get a priority 
// scheme.
void SearchReplace::OverlayPtrs( shared_ptr<Node> dest, shared_ptr<Node> source, bool under_substitution )
{
    ASSERT( TypeInfo(source) >= TypeInfo(dest) )("source must be a non-strict subclass of destination, so that it does not have more members");

    vector< Itemiser::Element * > source_memb = Itemiser::Itemise( source.get() ); 
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get(),       // The thing we're itemising
                                                                 source.get() );   // Just get the members corresponding to source's class
    ASSERT( source_memb.size() == dest_memb.size() );

    for( int i=0; i<dest_memb.size(); i++ )
    {
        ASSERT( source_memb[i] && "itemise returned null element" );
        ASSERT( dest_memb[i] && "itemise returned null element" );
        
        if( GenericSequence *source_seq = dynamic_cast<GenericSequence *>(source_memb[i]) )                
        {
            GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]);
            ASSERT( dest_seq && "itemise for dest didn't match itemise for source");
            ASSERT( source_seq->size() == dest_seq->size() );

            for( int j=0; j<source_seq->size(); j++ )
            {
                if( !dest_seq->Element(j).Get() ) // Only over NULL!!!
                    dest_seq->Element(j).Set( DuplicateSubtree( source_seq->Element(j).Get(), under_substitution ) );
            }
        }            
        else if( GenericSharedPtr *source_ptr = dynamic_cast<GenericSharedPtr *>(source_memb[i]) )         
        {
            GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]);
            ASSERT( dest_ptr && "itemise for target didn't match itemise for source");
                        
            if( !dest_ptr->Get() ) // Only over NULL!!!1
                dest_ptr->Set( DuplicateSubtree( source_ptr->Get(), under_substitution ) );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a sequence or a shared pointer");               
        }
    }        
}


// Duplicate an entire subtree, following the rules for inferno search and replace.
// We recurse through the subtree, using Duplcator to create the new nodes. We support 
// substitution based on configured match sets, and we do not suplicate identifiers when 
// substituting.
shared_ptr<Node> SearchReplace::DuplicateSubtree( shared_ptr<Node> source, bool under_substitution )
{
    // Check match set
    shared_ptr<Node> local_substitute; // source after substitution
    const MatchSet *m = FindMatchSet( source );
    if( m )
    {
        // It's in a match set, so local_substitute the key
        ASSERT( m->key )("Match set in replace pattern but did not key to search pattern");
        local_substitute = m->key;       
        ASSERT( TypeInfo(source) >= TypeInfo(local_substitute) )("source must be a non-strict subclass of local_substitute, so that it does not have more members");
        under_substitution = true;
    }
    
    shared_ptr<Node> to_duplicate = local_substitute ? local_substitute : source;
    
    if( under_substitution && dynamic_pointer_cast<Identifier>( to_duplicate ) )
    {
        // Substitute is an identifier, so preserve its uniqueness by just linking 
        // in the same node. Don't do any more - we wouldn't want to change the
        // identifier in the tree even if it had members, lol!
        return to_duplicate;
    }

    // Make the destination node based on local_substitute if found, otherwise the source
    ASSERT( source );
    shared_ptr<Duplicator> dup_dest = Duplicator::Duplicate( to_duplicate );
    shared_ptr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);
    
    // Make all members in the dest NULL
    ClearPtrs( dest );
    
    // Copy the source over, including any NULLs in the source. If source is superclass
    // of dest (i.e. has possibly fewer members) the missing ones will remain NULL and
    // will come from local_substitute node.
    OverlayPtrs( dest, source, under_substitution );
    
    // If found local_substitute, copy local_substitute members *only* over members NULL in the source
    if( local_substitute )
        OverlayPtrs( dest, local_substitute, true );

    return dest;
}


// Perform the configures replacement at the supplied target. 
// Note target is a double pointer, since we wish to enact the
// replacement by changing a SharedPtr somewhere.
void SearchReplace::Replace( GenericSharedPtr *target )
{
    ASSERT( replace_pattern );
    target->Set( DuplicateSubtree( replace_pattern ) );
}


// Perform search and replace on supplied program based
// on current patterns and match sets. Does search and replace
// operations repeatedly until there are no more matches.
void SearchReplace::operator()( shared_ptr<Program> p )
{
    program = p;
    while(1)
    {
        GenericSharedPtr *gp = Search( program );        
        if( gp )
            Replace( gp );
        else
            break;
    }
    program = shared_ptr<Program>();
}


// Find a match set containing the supplied node
const SearchReplace::MatchSet *SearchReplace::FindMatchSet( shared_ptr<Node> node )
{
    for( set<MatchSet>::iterator msi = matches->begin();
         msi != matches->end();
         msi++ )
    {
        MatchSet::iterator ni = msi->find( node );
        if( ni != msi->end() )
            return &*msi;
    }
    return 0;
}


// Reset the keys in all the matchsets 
void SearchReplace::ClearKeys()
{
    for( set<MatchSet>::iterator msi = matches->begin();
         msi != matches->end();
         msi++ )
    {
        msi->key = shared_ptr<Node>();
    }
}


void SearchReplace::Test()
{
    SearchReplace sr;
    
    {
        // single node with topological wildcarding
        shared_ptr<Void> v(new Void);
        ASSERT( sr.IsMatchPattern( v, v ) == true );
        shared_ptr<Bool> b(new Bool);
        ASSERT( sr.IsMatchPattern( v, b ) == false );
        ASSERT( sr.IsMatchPattern( b, v ) == false );
        shared_ptr<Type> t(new Type);
        ASSERT( sr.IsMatchPattern( v, t ) == true );
        ASSERT( sr.IsMatchPattern( t, v ) == false );
        ASSERT( sr.IsMatchPattern( b, t ) == true );
        ASSERT( sr.IsMatchPattern( t, b ) == false );
        
        // node points directly to another with TC
        shared_ptr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.IsMatchPattern( p1, b ) == false );
        ASSERT( sr.IsMatchPattern( p1, p1 ) == true );
        shared_ptr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.IsMatchPattern( p1, p2 ) == false );
        p2->destination = t;
        ASSERT( sr.IsMatchPattern( p1, p2 ) == true );
        ASSERT( sr.IsMatchPattern( p2, p1 ) == false );
    }
    
    {
        // string property
        shared_ptr<String> s1( new String );
        shared_ptr<String> s2( new String );
        s1->value = "here";
        s2->value = "there";
        ASSERT( sr.IsMatchPattern( s1, s1 ) == true );
        ASSERT( sr.IsMatchPattern( s1, s2 ) == false );        
    }    
    
    {
        // int property
        llvm::APSInt apsint( 32, true );
        shared_ptr<Integer> i1( new Integer );
        shared_ptr<Integer> i2( new Integer );
        apsint = 3;
        i1->value = apsint;
        apsint = 5;
        i2->value = apsint;
        TRACE("  %s %s\n", i1->value.toString(10).c_str(), i2->value.toString(10).c_str() );
        ASSERT( sr.IsMatchPattern( i1, i1 ) == true );
        ASSERT( sr.IsMatchPattern( i1, i2 ) == false );        
    }    
    
    {
        // node with sequence, check lengths 
        shared_ptr<Compound> c1( new Compound );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Compound> c2( new Compound );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == true );        
        shared_ptr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );        
    }

    {
        // node with sequence, TW 
        shared_ptr<Compound> c1( new Compound );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        shared_ptr<Compound> c2( new Compound );
        shared_ptr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == true );
        ASSERT( sr.IsMatchPattern( c2, c1 ) == false );
    }
    
    {        
        // topological with extra member in terget node
        shared_ptr<Label> l( new Label );
        shared_ptr<Public> p1( new Public );
        l->access = p1;
        shared_ptr<LabelIdentifier> li( new LabelIdentifier );
        li->name = "mylabel";
        l->identifier = li;
        shared_ptr<Declaration> d( new Declaration );
        shared_ptr<Public> p2( new Public );
        d->access = p2;
        ASSERT( sr.IsMatchPattern( l, d ) == true );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
        shared_ptr<Private> p3( new Private );
        d->access = p3;
        ASSERT( sr.IsMatchPattern( l, d ) == false );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
        shared_ptr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( sr.IsMatchPattern( l, d ) == true );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
    }
}
