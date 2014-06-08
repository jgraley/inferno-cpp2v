#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

void NormalAgent::Configure( const CompareReplace *s, CouplingKeys *c )
{
	ASSERT(s);
	ASSERT(c);
	sr = s;
    coupling_keys = c;
	// TODO recursively configure children
}


bool NormalAgent::DecidedCompare( const TreePtrInterface &x,
							       TreePtr<Node> pattern,
							       bool can_key,
    						       Conjecture &conj ) const
{
    INDENT;
	ASSERT(sr)("Agent ")(*pattern)(" at %p appears not to have been configured, since sr is NULL", this);
	ASSERT(coupling_keys);
	ASSERT( x ); // Target must not be NULL
	ASSERT(pattern);
    TRACE("Comparing x=")
         (*x)
         (" with pattern=")
         (*pattern)
         ("\n");
    shared_ptr<Key> special_key;
    
	// Check whether the present node matches. Do this for all nodes: this will be the local
	// restriction for normal nodes and the pre-restriction for special nodes (based on
	// how IsLocalMatch() has been overridden.
	if( !pattern->IsLocalMatch(x.get()) )
	{
		return false;
    }
    
    if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
        // Hand over to any soft search functionality in the search pattern node
        bool r = ssp->DecidedCompare( sr, x, can_key, conj );
        if( !r )
            return false;
    }
    else if( shared_ptr<SoftSearchPatternSpecialKey> sspsk = dynamic_pointer_cast<SoftSearchPatternSpecialKey>(pattern) )
    {
        // Hand over to any soft search functionality in the search pattern node
        special_key = sspsk->DecidedCompare( sr, x, can_key, conj );
        if( !special_key )
            return false;
    }
    else if( dynamic_pointer_cast<SoftReplacePattern>(pattern) )
    {
    	// No further restriction beyond the pre-restriction for these nodes when searching.
    }
    else if( shared_ptr<SearchContainerBase> sc_pattern = dynamic_pointer_cast<SearchContainerBase>(pattern) )
    {
    	// Invoke stuff node compare
    	// Check whether the present node matches
    	bool r = DecidedCompare( x, sc_pattern, can_key, conj );
        if( !r )
            return false;
    }
    else if( shared_ptr<GreenGrassBase> green_pattern = dynamic_pointer_cast<GreenGrassBase>(pattern) )
    {
        // Restrict so that everything in the input program under here must be "green grass"
        // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
       // Walk w( x );
       // FOREACH( TreePtr<Node> p, w )
		if( sr->GetOverallMaster()->dirty_grass.find( /*p*/x ) != sr->GetOverallMaster()->dirty_grass.end() )
		{
			TRACE/*(*p)(" under ")*/(*x)(" is dirty grass so rejecting\n");
			return false;
		}
        TRACE("subtree under ")(*x)(" is green grass\n");
        // Normal matching for the through path
        bool r = DecidedCompare( x, green_pattern->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else if( shared_ptr<OverlayBase> op = dynamic_pointer_cast<OverlayBase>(pattern) )
    {
        // When Overlay node seen duriung search, just forward through the "through" path
        bool r = DecidedCompare( x, op->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else if( dynamic_pointer_cast<InsertBase>(pattern) || dynamic_pointer_cast<EraseBase>(pattern) )
    {
       ASSERTFAIL(*pattern)(" found outside of a container; can only be used in containers\n");
    }
    else if( shared_ptr<SlaveBase> sp = dynamic_pointer_cast<SlaveBase>(pattern) )
    {
        // When a slave node seen duriung search, just forward through the "base" path
        bool r = DecidedCompare( x, sp->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else
    {
		// Recurse through the children. Note that the itemiser internally does a
		// dynamic_cast onto the type of pattern, and itemises over that type. x must
		// be dynamic_castable to pattern's type.
		vector< Itemiser::Element * > pattern_memb = pattern->Itemise();
		vector< Itemiser::Element * > x_memb = pattern->Itemise( x.get() );   // Get the members of x corresponding to pattern's class
		ASSERT( pattern_memb.size() == x_memb.size() );
		for( int i=0; i<pattern_memb.size(); i++ )
		{
			bool r;
			ASSERT( pattern_memb[i] )( "itemise returned null element");
			ASSERT( x_memb[i] )( "itemise returned null element");

			if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(pattern_memb[i]) )
			{
				SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]);
				ASSERT( x_seq )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Sequence, x %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
				r = DecidedCompare( *x_seq, *pattern_seq, can_key, conj );
			}
			else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
			{
				CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
				ASSERT( x_col )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompare( *x_col, *pattern_col, can_key, conj );
			}
			else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
			{
			    if( !TreePtr<Node>(*pattern_ptr) ) // TreePtrs are allowed to be NULL meaning no restriction
				{
				    r = true;
				}
				else
				{
					TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]);
					ASSERT( x_ptr )( "itemise for x didn't match itemise for pattern");
					TRACE("Member %d is TreePtr, pattern ptr=%p\n", i, pattern_ptr->get());
					TreePtr<Node> tpp(*pattern_ptr);
					Agent *ap = Agent::AsAgent(tpp);
					//NormalAgent *nap = dynamic_cast<NormalAgent *>(ap);
					//ASSERT( nap ); 
					//ASSERT( nap->sr == sr )("%p != %p", nap->sr, sr);
					//ASSERT( nap->coupling_keys == coupling_keys );
					r = ap->DecidedCompare( *x_ptr, tpp, can_key, conj );
				}
			}
			else
			{
				ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
			}

			if( !r )
				return false;
		}
    }
   
    // Check couplings after everything else because they can get keyed during keying pass
    // and then seem to match during restricting pass. A key match is not a guarantee 
    // because when it keyed some coupled node below it may not have been keyed yet 
    // and so their checks would not have occured and hte check would not be strict 
    // enough. Perhaps keys can be "concrete" when all the couplings below them have
    // been checked as matching?
    if( TreePtr<Node> keynode = coupling_keys->GetCoupled( pattern ) )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
	
	if( can_key )
    {
        if( special_key )
            coupling_keys->DoKey( special_key, pattern );  
        else
            coupling_keys->DoKey( x, pattern );  
    }

    return true;
}


// TODO support SearchContainerBase(ie Stuff nodes) here and below inside the container.
// Behaviour would be to try the container at each of the nodes matched by the star, and if
// one match is found we have a hit (ie OR behaviour). I think this results in 3 decisions
// for sequences as opposed to Star and Stuff, which are one decision each. They are:
// first: How many elements to match (as with Star)
// second: Which of the above to try for container match
// third: Which element of the SearchContainer to try 
bool NormalAgent::DecidedCompare( SequenceInterface &x,
		                          SequenceInterface &pattern,
		                          bool can_key,
		                          Conjecture &conj ) const
{
    INDENT;
    Sequence<Node> epattern = WalkContainerPattern( pattern, false );    
    
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = x.begin();
	ContainerInterface::iterator pit, npit, nnpit;

	for( pit = epattern.begin(); pit != epattern.end(); ++pit )
	{
		ASSERT( xit == x.end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
		npit=pit;
		++npit;

        if( shared_ptr<StarBase> ps = dynamic_pointer_cast<StarBase>(pe) )
        {
            // We have a Star type wildcard that can match multiple elements.
            // Remember where we are - this is the beginning of the subsequence that
            // potentially matches the Star.
            ContainerInterface::iterator xit_begin_star = xit;

            // Star always matches at the end of a sequence, so we only need a 
            // decision when there are more elements left
            if( npit == epattern.end() )
            {
                xit = x.end(); // match all remaining members of x; jump to end
            }
            else
            {
                TRACE("Pattern continues after star\n");

                // Star not at end so there is more stuff to match; ensure not another star
            //  ASSERT( !dynamic_pointer_cast<StarBase>(TreePtr<Node>(*npit)) )
            //        ( "Not allowed to have two neighbouring Star elements in search pattern Sequence");

                // Decide how many elements the current * should match, using conjecture. Jump forward
                // that many elements, to the element after the star. We need to use the special 
                // interface to Conjecture because the iterator we get is itself an end to the 
                // range matched by the star, and so x.end() is a legitimate choice for ss.end()
                // So allow the Conjecture class to give us two ends, and only give up on the second.
                Conjecture::Choice *current_choice = conj.GetChoicePtr();
                {
                    xit = conj.HandleDecision( xit_begin_star, x.end(), 1 );
                }                               
            }
            
            // Star matched [xit_begin_star, xit) i.e. xit-xit_begin_star elements
            // Now make a copy of the elements that matched the star and apply couplings
            TreePtr<SubSequenceRange> ss( new SubSequenceRange( xit_begin_star, xit ) );
            //for( ContainerInterface::iterator it=xit_begin_star; it != xit; ++it ) // TODO FOREACH?
            //{
            //  ss->push_back( *it );
            //}
            
            // Apply couplings to this Star and matched range
            if( TreePtr<Node> keynode = coupling_keys->GetCoupled( pe ) )
            {
                SimpleCompare sc;
                if( sc( TreePtr<Node>(ss), keynode ) == false )
                    return false;
            }
            
            // Restrict to pre-restriction or pattern
            bool r = ps->MatchRange( sr, *ss, can_key );
            if( !r )
                return false;

            if( can_key )
                coupling_keys->DoKey( TreePtr<Node>(ss), pe );   
        }
 	    else // not a Star so match singly...
	    {
            // If there is one more element in x, see if it matches the pattern
			//TreePtr<Node> xe( x[xit] );
			if( xit != x.end() && /*Agent::AsAgent(pe)->*/DecidedCompare( *xit, pe, can_key, conj ) == true )
			{
				++xit;
			}
			else
			{
				TRACE("Element mismatched\n");
				return false;
			}
	    }
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==x.end(), pit==epattern.end() );
    return (xit==x.end() && pit==epattern.end()) ? true : false;
}


bool NormalAgent::DecidedCompare( CollectionInterface &x,
		 					       CollectionInterface &pattern,
							       bool can_key,
							       Conjecture &conj ) const
{
    INDENT;
    Sequence<Node> epattern = WalkContainerPattern( pattern, false );    
   
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
	// TODO is there some stl algorithm for this?
    TreePtr<SubCollection> xremaining( new SubCollection );
    FOREACH( const TreePtrInterface &xe, x )
        xremaining->insert( xe );
    
    shared_ptr<StarBase> star;
    bool seen_star = false;

    for( CollectionInterface::iterator pit = epattern.begin(); pit != epattern.end(); ++pit )
    {
    	TRACE("Collection compare %d remain out of %d; looking at %s in epattern\n",
    			xremaining->size(),
    			epattern.size(),
    			TypeInfo( TreePtr<Node>(*pit) ).name().c_str() );
    	shared_ptr<StarBase> maybe_star = dynamic_pointer_cast<StarBase>( TreePtr<Node>(*pit) );

        if( maybe_star ) // Star in pattern collection?
        {
        	ASSERT(!seen_star)("Only one Star node (or NULL ptr) allowed in a search epattern Collection");
        	// TODO remove this restriction - I might want to match one star and leave another unmatched.
            star = maybe_star; // remember for later and skip to next pattern
            seen_star = true; // TODO do we need?
        }
	    else // not a Star so match singly...
	    {
	    	// We have to decide which node in the tree to match, so use the present conjecture
	    	ContainerInterface::iterator xit = conj.HandleDecision( x.begin(), x.end() );
			if( xit == x.end() )
				return false;

	    	// Remove the chosen element from the remaineder collection. If it is not there (ret val==0)
	    	// then the present chosen iterator has been chosen before and the choices are conflicting.
	    	// We'll just return false so we do not stop trying further choices (xit+1 may be legal).
	    	if( xremaining->erase( *xit ) == 0 )
	    		return false;

	    	// Recurse into comparison function for the chosen node
			if( !/*Agent::AsAgent(TreePtr<Node>(*pit))->*/DecidedCompare( *xit, TreePtr<Node>(*pit), can_key, conj ) )
			    return false;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return false; // there were elements left over and no star to match them against

    TRACE("seen_star %d star %p size of xremaining %d\n", seen_star, star.get(), xremaining->size() );

    // Apply pre-restriction to the star
    if( seen_star )
    {
        if( TreePtr<Node> keynode = coupling_keys->GetCoupled( star ) )
        {
            SimpleCompare sc;
            if( sc( TreePtr<Node>(xremaining), keynode ) == false )
                return false;
        }

        bool r = star->MatchRange( sr, *xremaining, can_key );
        if( !r )
            return false;
    
        if( can_key )
            coupling_keys->DoKey( TreePtr<Node>(xremaining), star );	
    }
    TRACE("matched\n");
	return true;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
bool NormalAgent::DecidedCompare( const TreePtrInterface &x,
							       shared_ptr<SearchContainerBase> pattern,
						     	   bool can_key,
							       Conjecture &conj ) const
{
    INDENT;
	ASSERT( pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    TRACE("SearchContainer pattern ")(*pattern)(" terminus pattern is ")(*(pattern->terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    shared_ptr<ContainerInterface> pwx = pattern->GetContainerInterface( x );
    
	// Get choice from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = conj.HandleDecision( pwx->begin(), pwx->end() );
	if( thistime == (ContainerInterface::iterator)(pwx->end()) )
		return false; // ran out of choices

	// Try out comparison at this position
	TRACE("Trying terminus ")(**thistime);
	bool r = /*Agent::AsAgent(pattern->terminus)->*/DecidedCompare( *thistime, pattern->terminus, can_key, conj );
    if( !r )
        return false;
        
    if( TreePtr<Node> keynode = coupling_keys->GetCoupled( pattern ) )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
    
    // If we got this far, do the couplings
    if( can_key )
    {
    	shared_ptr<TerminusKey> key( new TerminusKey );
    	key->root = x;
    	key->terminus = *thistime;
    	shared_ptr<Key> sckey( key );
    	TRACE("Matched, so keying search container type ")(*pattern)(" for ")(*x)(" at %p\n", key.get());
        coupling_keys->DoKey( sckey, pattern );	
    }
	return r;
}


TreePtr<Node> NormalAgent::BuildReplace( TreePtr<Node> pattern,
	                                     TreePtr<Node> keynode ) const
{
    INDENT;
    ASSERT( pattern );

    // See if the pattern node is coupled to anything
    shared_ptr<Key> key = coupling_keys->GetKey( pattern );
	if( key )
		keynode = key->root;
	if( dynamic_pointer_cast<SearchContainerBase>(pattern) )
	{
		// SearchContainer.
		// Are we substituting a stuff node? If so, see if we reached the terminus, and if
		// so come out of substitution. Done as tail recursion so that we already duplicated
		// the terminus key, and can just overlay the terminus replace pattern.
		shared_ptr<TerminusKey> stuff_key = dynamic_pointer_cast<TerminusKey>(key);
		ASSERT( stuff_key );
		ASSERT( stuff_key->replace_pattern );
		shared_ptr<TerminusBase> replace_stuff = dynamic_pointer_cast<TerminusBase>( stuff_key->replace_pattern );
		ASSERT( replace_stuff );
		ASSERT( replace_stuff->terminus );
		TRACE( "Stuff node: Duplicating at terminus first: keynode=")(*(replace_stuff->terminus))
		                                                  (", term=")(*(stuff_key->terminus))("\n");
		TreePtr<Node> term = BuildReplace( replace_stuff->terminus, stuff_key->terminus );
		TRACE( "Stuff node: Substituting stuff");
		return sr->DuplicateSubtree(keynode, stuff_key->terminus, term);   
	}     
	else
	{
	    return BuildReplaceKeyed( pattern, keynode );
	}
}
	
	
TreePtr<Node> NormalAgent::BuildReplaceKeyed( TreePtr<Node> pattern,
	                                          TreePtr<Node> keynode ) const
{	
    INDENT;
    ASSERT( pattern );

	if( shared_ptr<StarBase> stb = dynamic_pointer_cast<StarBase>( pattern ) )
	{
		return BuildReplaceStar( stb, keynode ); // Strong modifier
	}
	if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( pattern ) )
	{
		TreePtr<Node> overlay = srp->GetOverlayPattern(); // only strong modifiers use this
		if( overlay ) // Really two different kinds of pattern node
			return BuildReplace( overlay, keynode ); // Strong modifier
		else
			return sr->DuplicateSubtree(keynode);   // Weak modifier
	}
	else if( shared_ptr<OverlayBase> ob = dynamic_pointer_cast<OverlayBase>( pattern ) )
	{
		ASSERT( ob->GetOverlay() );          
		TRACE("Overlay node through=")(*(ob->GetThrough()))(" overlay=")(*(ob->GetOverlay()))("\n");
		return BuildReplace( ob->GetOverlay(), keynode );
	}
	else if( shared_ptr<GreenGrassBase> ggb = dynamic_pointer_cast<GreenGrassBase>( pattern ) )
	{
		ASSERT( ggb->GetThrough() );          
		TRACE("GreenGrass node through=")(*(ggb->GetThrough()))("\n");
    	return BuildReplace( ggb->GetThrough(), keynode );
	}
	else if( shared_ptr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(pattern) )
	{   
		return BuildReplaceSlave( sb, keynode );
	} 
	else if( dynamic_pointer_cast<SpecialBase>(pattern) )
	{
		// Star, Not, TransformOf etc. Also MatchAll with no overlay pattern falls thru to here
		return sr->DuplicateSubtree(keynode);   
	}     
    else // Normal node
    {
        if( keynode && pattern->IsLocalMatch(keynode.get()) ) 
            return BuildReplaceOverlay( pattern, keynode );
        else
            return BuildReplaceNormal( pattern ); // Overwriting pattern over dest, need to make a duplicate 
	}
}



TreePtr<Node> NormalAgent::BuildReplaceOverlay( TreePtr<Node> pattern, 
										         TreePtr<Node> keynode ) const // under substitution if not NULL
{
	INDENT;
    ASSERT( pattern );
    ASSERT( keynode );

#ifdef STRACE
    TRACE("DoOverlayPattern dest={");
   {    Walk w(dest);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
   TRACE("pattern={");
   {    Walk w(pattern);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
#endif   
    
    ASSERT( pattern->IsLocalMatch(keynode.get()) )
	  	  ("pattern=")
		  (*pattern)
		  (" must be a non-strict superclass of keynode=")
		  (*keynode)
		  (", so that it does not have more members");
    TreePtr<Node> dest;
    
    // Make a new node, we will overlay from pattern, so resulting node will be dirty	    
    dest = sr->DuplicateNode( keynode, true );

    // Loop over the elements of pattern, keynode and dest, limited to elements
    // present in pattern, which is a non-strict subclass of keynode and dest.
    // Overlay or overwrite pattern over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > pattern_memb = pattern->Itemise();
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise( dest.get() ); // Get the members of keynode corresponding to pattern's class
    vector< Itemiser::Element * >  dest_memb = pattern->Itemise( dest.get() ); // Get the members of dest corresponding to pattern's class
    ASSERT( pattern_memb.size() == dest_memb.size() );
    Set< Itemiser::Element * > present_in_pattern;
    
    TRACE("Copying %d members from pattern=%s dest=%s\n", dest_memb.size(), TypeInfo(pattern).name().c_str(), TypeInfo(dest).name().c_str());
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("member %d from pattern\n", i );
        ASSERT( pattern_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );                
        if( ContainerInterface *pattern_con = dynamic_cast<ContainerInterface *>(pattern_memb[i]) )                
        {
            Sequence<Node> expanded_pattern_con = WalkContainerPattern( *pattern_con, true );    

            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            ASSERT( dest_con )( "itemise for dest didn't match itemise for expanded_pattern_con");
            dest_con->clear();

            TRACE("Copying container size %d from expanded_pattern_con\n", expanded_pattern_con.size() );
	        FOREACH( const TreePtrInterface &p, expanded_pattern_con )
	        {
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*pattern)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
				TreePtr<Node> n = BuildReplace( p );
                if( ContainerInterface *psc = dynamic_cast<ContainerInterface *>(n.get()) )
                {
                    TRACE("Walking SubContainer length %d\n", psc->size() );
                    FOREACH( const TreePtrInterface &pp, *psc )
                        dest_con->insert( pp );
                }
                else 
                {
                    TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
                    dest_con->insert( n );
                }
	        }
	        present_in_pattern.insert( dest_memb[i] );
        }            
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for pattern");
            TreePtr<Node> pattern_child = *pattern_ptr;
            TreePtr<Node> dest_child = *dest_ptr;
                       
            if( pattern_child )
            {                             
                dest_child = BuildReplace( pattern_child, *keynode_ptr );
                ASSERT( dest_child );
                ASSERT( dest_child->IsFinal() );
                present_in_pattern.insert( dest_memb[i] );
            }
            
            *dest_ptr = dest_child;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }        
    }
    
    // Loop over all the elements of keynode and dest that do not appear in pattern or
    // appear in pattern but are NULL TreePtr<>s. Duplicate from keynode into dest.
    keynode_memb = keynode->Itemise();
    dest_memb = dest->Itemise(); 
    
    TRACE("Copying %d members from keynode=%s dest=%s\n", dest_memb.size(), TypeInfo(keynode).name().c_str(), TypeInfo(dest).name().c_str());
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( present_in_pattern.IsExist(dest_memb[i]) )
            continue; // already did this one in the above loop

    	TRACE("Member %d from key\n", i );
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            // Note: we get here when a wildcard is coupled that does not have the container
            // because it is an intermediate node. Eg Scope as a wildcard matching Module does 
            // not have "bases".
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d from key\n", keynode_con->size() );
	        FOREACH( const TreePtrInterface &p, *keynode_con )
	        {
		        ASSERT( p ); // present simplified scheme disallows NULL
		        TreePtr<Node> n = sr->DuplicateSubtree( p );
		        if( ContainerInterface *sc = dynamic_cast<ContainerInterface *>(n.get()) )
		        {
			        TRACE("Walking SubContainer length %d\n", sc->size() );
		            FOREACH( const TreePtrInterface &xx, *sc )
			            dest_con->insert( xx );
           		}
		        else
		        {
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr );
            *dest_ptr = sr->DuplicateSubtree( *keynode_ptr );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
#ifdef STRACE
    TRACE("DoOverlayPattern result={");
   {    Walk w(dest);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
#endif    
    ASSERT( dest );
    return dest;
}


TreePtr<Node> NormalAgent::BuildReplaceSlave( shared_ptr<SlaveBase> pattern, 
										       TreePtr<Node> keynode ) const 
{
    INDENT;
	ASSERT( pattern );
	ASSERT( pattern->GetThrough() );   
	
	// Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = BuildReplace( pattern->GetThrough(), keynode );
    
	// Run the slave as a new transformation at the current location
	(*pattern)( sr->GetContext(), &dest );
	
    ASSERT( dest );
    return dest;
}

    
TreePtr<Node> NormalAgent::BuildReplaceNormal( TreePtr<Node> pattern ) const
{
	INDENT;
    ASSERT( pattern );
 
	// Make a new node, force dirty because from pattern
    TreePtr<Node> dest = sr->DuplicateNode( pattern, true );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. dest must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = pattern->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Copying %d members pattern=", dest_memb.size())(*pattern)(" dest=")(*dest)("\n");
    // Loop over all the members of pattern (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Copying member %d\n", i );
        ASSERT( pattern_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *pattern_con = dynamic_cast<ContainerInterface *>(pattern_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d\n", pattern_con->size() );
	        FOREACH( const TreePtrInterface &p, *pattern_con )
	        {
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*pattern)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
	            TreePtr<Node> n = BuildReplace( p );
		        if( ContainerInterface *psc = dynamic_cast<ContainerInterface *>(n.get()) )
		        {
			        TRACE("Walking SubContainer length %d\n", psc->size() );
		            FOREACH( const TreePtrInterface &pp, *psc )
			            dest_con->insert( pp );  // TODO support Container::insert(Container) to do this generically
           		}
		        else
		        {
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
        }            
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
            TRACE("Copying single element\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *pattern_ptr )("Member %d (", i)(*pattern_ptr)(") of ")(*pattern)(" was NULL when not overlaying\n");
            *dest_ptr = BuildReplace( *pattern_ptr );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() )("Member %d (", i)(**pattern_ptr)(") of ")(*pattern)(" was not final\n");            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    return dest;
}


TreePtr<Node> NormalAgent::BuildReplaceStar( shared_ptr<StarBase> pattern,
	                                         TreePtr<Node> keynode ) const
{
	ASSERT( pattern ); // not used at present but should be there since we may need to use it
	ASSERT( keynode );
	ContainerInterface *psc = dynamic_cast<ContainerInterface *>(keynode.get());
	ASSERT( psc )("Star node ")(*pattern)(" keyed to ")(*keynode)(" which should implement ContainerInterface");	
	TRACE("Walking container length %d\n", psc->size() );
	
	TreePtr<SubContainer> dest;
	ContainerInterface *dest_container;
	if( dynamic_cast<SequenceInterface *>(keynode.get()) )
		dest = TreePtr<SubSequence>(new SubSequence);
	else if( dynamic_cast<CollectionInterface *>(keynode.get()) )
		dest = TreePtr<SubCollection>(new SubCollection);
	else
		ASSERT(0)("Please add new kind of Star");
	
    dest_container = dynamic_cast<ContainerInterface *>(dest.get());
	FOREACH( const TreePtrInterface &pp, *psc )
	{
		TreePtr<Node> nn = sr->DuplicateSubtree( pp );
		dest_container->insert( nn );
	}
	
	return dest;
}


Sequence<Node> NormalAgent::WalkContainerPattern( ContainerInterface &pattern,
                                                  bool replacing ) const
{
    // This helper is for Insert and Erase nodes. It takes a pattern container (which
    // is the only place these nodes should occur) and expands out either Insert or
    // Erase nodes. When searching, Erase is expanded out so that the program nodes
    // to be erased may be matched off (cond keyed etc) and Insert is skipped because
    // it does not need to correspond to anything during search. When replacing, 
    // erase is skipped to erase the elements and Insert is expanded to insert them. 
    Sequence<Node> expanded;
    FOREACH( TreePtr<Node> n, pattern )
    {
        if( shared_ptr<EraseBase> pe = dynamic_pointer_cast<EraseBase>(n) )
        {
            if( !replacing )
                FOREACH( TreePtr<Node> e, *(pe->GetErase()) )
                    expanded.push_back( e );                
        }
        else if( shared_ptr<InsertBase> pi = dynamic_pointer_cast<InsertBase>(n) )
        {
            if( replacing )
                FOREACH( TreePtr<Node> i, *(pi->GetInsert()) )
                    expanded.push_back( i );                
        }
        else if( shared_ptr<OverlayBase> po = dynamic_pointer_cast<OverlayBase>(n) )
        {
            // Unfortunate inconsistency here: An overlay node ca either (a) support stars under it for use in 
            // collections or (b) support overlaying, but not both. TODO think about overlaying subsequences 
            // We use the pattern-tweaking method if thre are stars, but let the replace engine do overlaying
            // if there are not (overlaying takes place for the single element at the position of the Overlay node)
            if( dynamic_pointer_cast<StarBase>(po->GetOverlay()) || dynamic_pointer_cast<StarBase>(po->GetThrough()) )
            {
                if( replacing )
                    expanded.push_back( po->GetOverlay() );         
                else
                    expanded.push_back( po->GetThrough() );    
            }
            else
            {
                expanded.push_back( n );
            }                
        }
        else
        {
            expanded.push_back( n );
        }
    }
    return expanded;
}
                                                                                                        

