#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "overlay_agent.hpp"
#include "normal_agent.hpp"

using namespace SR;

bool NormalAgent::DecidedCompareImpl( const TreePtrInterface &x,
							          bool can_key,
    						          Conjecture &conj )
{
    if( SlaveBase *s_this = dynamic_cast<SlaveBase *>(this) )
    {
        // When a slave node seen duriung search, just forward through the "base" path
        bool r = Agent::AsAgent(s_this->GetThrough())->DecidedCompare( x, can_key, conj );
        if( !r )
            return false;
    }
    else
    {
		// Recurse through the children. Note that the itemiser internally does a
		// dynamic_cast onto the type of pattern, and itemises over that type. x must
		// be dynamic_castable to pattern's type.
		vector< Itemiser::Element * > pattern_memb = Itemise();
		vector< Itemiser::Element * > x_memb = Itemise( x.get() );   // Get the members of x corresponding to pattern's class
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
				r = DecidedCompareSequence( *x_seq, *pattern_seq, can_key, conj );
			}
			else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
			{
				CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
				ASSERT( x_col )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompareCollection( *x_col, *pattern_col, can_key, conj );
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
					TRACE("Member %d is TreePtr, pattern=", i)(*pattern_ptr);
					Agent *ap = Agent::AsAgent(*pattern_ptr);
					//NormalAgent *nap = dynamic_cast<NormalAgent *>(ap);
					//ASSERT( nap ); 
					//ASSERT( nap->sr == sr );
					//ASSERT( nap->coupling_keys == coupling_keys );
					r = ap->DecidedCompare( *x_ptr, can_key, conj );
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
    return true;
}


// TODO support SearchContainerBase(ie Stuff nodes) here and below inside the container.
// Behaviour would be to try the container at each of the nodes matched by the star, and if
// one match is found we have a hit (ie OR behaviour). I think this results in 3 decisions
// for sequences as opposed to Star and Stuff, which are one decision each. They are:
// first: How many elements to match (as with Star)
// second: Which of the above to try for container match
// third: Which element of the SearchContainer to try 
bool NormalAgent::DecidedCompareSequence( SequenceInterface &x,
		                                  SequenceInterface &pattern,
		                                  bool can_key,
		                                  Conjecture &conj )
{
    INDENT;
    Sequence<Node> epattern = OverlayAgent::WalkContainerPattern( pattern, false );    
    
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
            if( TreePtr<Node> keynode = coupling_keys->GetCoupled( Agent::AsAgent(pe) ) )
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
                coupling_keys->DoKey( TreePtr<Node>(ss), Agent::AsAgent(pe) );   
        }
 	    else // not a Star so match singly...
	    {
            // If there is one more element in x, see if it matches the pattern
			//TreePtr<Node> xe( x[xit] );
			if( xit != x.end() && Agent::AsAgent(pe)->DecidedCompare( *xit, can_key, conj ) == true )
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


bool NormalAgent::DecidedCompareCollection( CollectionInterface &x,
		 					                CollectionInterface &pattern,
							                bool can_key,
							                Conjecture &conj )
{
    INDENT;
    Sequence<Node> epattern = OverlayAgent::WalkContainerPattern( pattern, false );    
   
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
			if( !Agent::AsAgent(TreePtr<Node>(*pit))->DecidedCompare( *xit, can_key, conj ) )
			    return false;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return false; // there were elements left over and no star to match them against

    TRACE("seen_star %d size of xremaining %d\n", seen_star, xremaining->size() );

    // Apply pre-restriction to the star
    if( seen_star )
    {
        if( TreePtr<Node> keynode = coupling_keys->GetCoupled( Agent::AsAgent(star) ) )
        {
            SimpleCompare sc;
            if( sc( TreePtr<Node>(xremaining), keynode ) == false )
                return false;
        }

        bool r = star->MatchRange( sr, *xremaining, can_key );
        if( !r )
            return false;
    
        if( can_key )
            coupling_keys->DoKey( TreePtr<Node>(xremaining), Agent::AsAgent(star) );	
    }
    TRACE("matched\n");
	return true;
}


TreePtr<Node> NormalAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
	if( dynamic_cast<StarBase *>(this) )
	{
		return BuildReplaceStar( keynode ); // Strong modifier
	}
	else if( dynamic_cast<SlaveBase *>(this) )
	{   
		return BuildReplaceSlave( keynode );
	} 
	else if( dynamic_cast<SpecialBase *>(this) )
	{
		// Star, Not, TransformOf etc. Also MatchAll with no overlay pattern falls thru to here
		return sr->DuplicateSubtree(keynode);   
	}     
    else // Normal node
    {
        if( keynode && IsLocalMatch(keynode.get()) ) 
            return BuildReplaceOverlay( keynode );
        else
            return BuildReplaceNormal(); // Overwriting pattern over dest, need to make a duplicate 
	}
}


TreePtr<Node> NormalAgent::BuildReplaceOverlay( TreePtr<Node> keynode )  // under substitution if not NULL
{
	INDENT;
    ASSERT( keynode );
    
    ASSERT( IsLocalMatch(keynode.get()) )
	  	  ("pattern=")
		  (*this)
		  (" must be a non-strict superclass of keynode=")
		  (*keynode)
		  (", so that it does not have more members");
    TreePtr<Node> dest;
    
    // Make a new node, we will overlay from pattern, so resulting node will be dirty	
    // Duplicate the key node since it is at least as specialised    
    dest = sr->DuplicateNode( keynode, true );

    // Loop over the elements of pattern, keynode and dest, limited to elements
    // present in pattern, which is a non-strict subclass of keynode and dest.
    // Overlay or overwrite pattern over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise( dest.get() ); // Get the members of keynode corresponding to pattern's class
    vector< Itemiser::Element * >  dest_memb = Itemise( dest.get() ); // Get the members of dest corresponding to pattern's class
    ASSERT( pattern_memb.size() == dest_memb.size() );
    Set< Itemiser::Element * > present_in_pattern; // Repeatability audit: OK since only checking for existance TODO special version of set that disallows non-repeatable things
    
    TRACE("Copying %d members from pattern=%s dest=%s\n", dest_memb.size(), TypeInfo(this).name().c_str(), TypeInfo(dest).name().c_str());
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("member %d from pattern\n", i );
        ASSERT( pattern_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );                
        if( ContainerInterface *pattern_con = dynamic_cast<ContainerInterface *>(pattern_memb[i]) )                
        {
            Sequence<Node> expanded_pattern_con = OverlayAgent::WalkContainerPattern( *pattern_con, true );    

            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            ASSERT( dest_con )( "itemise for dest didn't match itemise for expanded_pattern_con");
            dest_con->clear();

            TRACE("Copying container size %d from expanded_pattern_con\n", expanded_pattern_con.size() );
	        FOREACH( const TreePtrInterface &p, expanded_pattern_con )
	        {
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*this)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
				TreePtr<Node> n = AsAgent(p)->BuildReplace();
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
                dest_child = AsAgent(pattern_child)->BuildReplace( *keynode_ptr );
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
    
    ASSERT( dest );
    return dest;
}


TreePtr<Node> NormalAgent::BuildReplaceSlave( TreePtr<Node> keynode )  
{
    INDENT;
    SlaveBase *sb_this = dynamic_cast<SlaveBase *>(this);
	ASSERT( sb_this );
	ASSERT( sb_this->GetThrough() );   
	
	// Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = AsAgent(sb_this->GetThrough())->BuildReplace( keynode );
    
	// Run the slave as a new transformation at the current location
	(*sb_this)( sr->GetContext(), &dest );
	
    ASSERT( dest );
    return dest;
}

    
TreePtr<Node> NormalAgent::BuildReplaceNormal() 
{
	INDENT;
 
	// Make a new node, force dirty because from pattern
    // Use clone here because we never want to place an Agent object in the output program tree.
    // Identifiers that have multiple referneces in the pattern will be coupled, and  
    // after the first hit, BuildReplaceOverlay() will handle the rest and it uses Duplicate()
    shared_ptr<Cloner> dup_dest = Clone();
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    sr->GetOverallMaster()->dirty_grass.insert( dest );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. dest must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Copying %d members pattern=", dest_memb.size())(*this)(" dest=")(*dest)("\n");
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
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*this)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
	            TreePtr<Node> n = AsAgent(p)->BuildReplace();
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
            ASSERT( *pattern_ptr )("Member %d (", i)(*pattern_ptr)(") of ")(*this)(" was NULL when not overlaying\n");
            *dest_ptr = AsAgent(*pattern_ptr)->BuildReplace();
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() )("Member %d (", i)(**pattern_ptr)(") of ")(*this)(" was not final\n");            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    return dest;
}


TreePtr<Node> NormalAgent::BuildReplaceStar( TreePtr<Node> keynode ) 
{
    StarBase *star_this = dynamic_cast<StarBase *>(this);
	ASSERT( star_this ); // not used at present but should be there since we may need to use it
	ASSERT( keynode );
	ContainerInterface *psc = dynamic_cast<ContainerInterface *>(keynode.get());
	ASSERT( psc )("Star node ")(*star_this)(" keyed to ")(*keynode)(" which should implement ContainerInterface");	
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

