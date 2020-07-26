#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "overlay_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"

using namespace SR;

PatternQueryResult StandardAgent::PatternQuery() const
{
    PatternQueryResult r;
    const vector< Itemiser::Element * > pattern_memb = Itemise();
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )
        {
   			FOREACH( TreePtr<Node> pe, *pattern_seq )
				r.AddLink(false, AsAgent(pe));    
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )
        {
   			StarAgent *star = NULL;
   			FOREACH( TreePtr<Node> pe, *pattern_col )
   			{
				if( StarAgent *s = dynamic_cast<StarAgent *>( AsAgent(pe) ) ) // per the impl, the star in a collection is not linked
				    star = s;
				else
				    r.AddLink(false, AsAgent(pe));    	    
		    }
		    if( star )
		        r.AddLink(false, star);    
        }
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(ie) )
        {
            if( TreePtr<Node>(*pattern_ptr) ) // TreePtrs are allowed to be NULL meaning no restriction            
                r.AddLink(false, AsAgent(*pattern_ptr));
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }
    return r;
}


void StandardAgent::DecidedQuery( QueryAgentInterface &query,
                                  const TreePtrInterface *px ) const
{
    INDENT(".");
    query.Reset();

    // Check pre-restriction
    if( !IsLocalMatch(px->get()) )        
    {
        query.AddLocalMatch(false);  
        return;
    }

    // Recurse through the children. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. x must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > x_memb = Itemise( px->get() );   // Get the members of x corresponding to pattern's class
    ASSERT( pattern_memb.size() == x_memb.size() );
    for( int i=0; i<pattern_memb.size(); i++ )
    {
        ASSERT( pattern_memb[i] )( "itemise returned null element");
        ASSERT( x_memb[i] )( "itemise returned null element");

        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(pattern_memb[i]) )
        {
            SequenceInterface *p_x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]);
            ASSERT( p_x_seq )( "itemise for x didn't match itemise for pattern");
            TRACE("Member %d is Sequence, x %d elts, pattern %d elts\n", i, p_x_seq->size(), pattern_seq->size() );
            DecidedQuerySequence( query, p_x_seq, *pattern_seq );
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
        {
            CollectionInterface *p_x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
            ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
            TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, p_x_col->size(), pattern_col->size() );
            DecidedQueryCollection( query, p_x_col, *pattern_col );
        }
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
            if( TreePtr<Node>(*pattern_ptr) ) // TreePtrs are allowed to be NULL meaning no restriction
            {
                TreePtrInterface *p_x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]);
                ASSERT( p_x_ptr )( "itemise for x didn't match itemise for pattern");
                TRACE("Member %d is TreePtr, pattern=", i)(*pattern_ptr);
                Agent *ap = Agent::AsAgent(*pattern_ptr);
                query.AddLink(false, ap, p_x_ptr);
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }

        if( !query.IsLocalMatch() )
        {
            return;
        }
    }
    return;
}

void StandardAgent::DecidedQuerySequence( QueryAgentInterface &query,
                                          SequenceInterface *px,
		                                  SequenceInterface &pattern ) const
{
    INDENT(" ");
	ContainerInterface::iterator pit, npit, nnpit, nxit;
    
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = px->begin();
	int p_remaining;

    int pattern_num_non_star = 0;
    ContainerInterface::iterator p_last_star;
	for( pit = pattern.begin(); pit != pattern.end(); ++pit ) // @TODO this is just pattern analysis - do in PatternQuery and cache?
    {
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
        Agent *pea = AsAgent(pe);
        if( dynamic_cast<StarAgent *>(pea) )
            p_last_star = pit;
        else
            pattern_num_non_star++;
    }
    if( px->size() < pattern_num_non_star )
    {
        query.AddLocalMatch(false);   // TODO break to get the final trace?
        return;
    }
    ContainerInterface::iterator xit_star_limit = px->end();            
    for( int i=0; i<pattern_num_non_star; i++ )
        --xit_star_limit;
        
	for( pit = pattern.begin(), p_remaining = pattern.size(); pit != pattern.end(); ++pit, --p_remaining )
	{
 		ASSERT( xit == px->end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
        Agent *pea = AsAgent(pe);

        if( StarAgent *psa = dynamic_cast<StarAgent *>(pea) )
        {
            // We have a Star type wildcard that can match multiple elements.
            ContainerInterface::iterator xit_star_end;
                
            // The last Star does not need a decision            
            if( pit == p_last_star )
            {
                // No more stars, so skip ahead to the end of the possible star range. 
                xit_star_end = xit_star_limit;
            }				
            else
            {
                // Decide how many elements the current * should match, using conjecture. The star's range
                // ends at the chosen element. Be inclusive because what we really want is a range.
                ASSERT( xit == px->end() || *xit );
                xit_star_end = query.AddDecision( xit, xit_star_limit, true );
            }
            
            // Star matched [xit, xit_star_end) i.e. xit-xit_begin_star elements
            TreePtr<StarAgent::SubSequenceRange> xss( new StarAgent::SubSequenceRange( xit, xit_star_end ) );

            // Apply couplings to this Star and matched range
            // Restrict to pre-restriction or pattern restriction
            query.AddLocalLink( false, psa, xss );
            
            // Resume at the first element after the matched range
            xit = xit_star_end;
        }
 	    else // not a Star so match singly...
	    {
            if( xit == px->end() )
                break;
       
            query.AddLink( false, pea, &*xit );
            ++xit;
            
            // Every non-star pattern node we pass means there's one fewer remaining
            // and we can match a star one step further
            ASSERT(xit_star_limit != px->end());
            ++xit_star_limit;
	    }
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==px->end(), pit==pattern.end() );
    query.AddLocalMatch( xit==px->end() && pit==pattern.end() );
    if( query.IsLocalMatch() )
    {
        ASSERT( p_remaining==0 );
	}
}


void StandardAgent::DecidedQueryCollection( QueryAgentInterface &query,
                                            CollectionInterface *px,
		 					                CollectionInterface &pattern ) const
{
    INDENT(" ");
    
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
    Collection<Node> xremaining;
    FOREACH( const TreePtrInterface &xe, *px )
        xremaining.insert( xe ); // Note: the new element in xremaining will not be the one from the original x (it's a TreePtr<Node>)
    
    StarAgent *star = NULL;

    for( CollectionInterface::iterator pit = pattern.begin(); pit != pattern.end(); ++pit )
    {
    	TRACE("Collection compare %d remain out of %d; looking at ",
                xremaining.size(),
                pattern.size() )(**pit)(" in pattern\n" );
        Agent *pia = Agent::AsAgent(TreePtr<Node>(*pit));
        if( StarAgent *s = dynamic_cast<StarAgent *>( pia ) ) // Star in pattern collection?
        {
        	ASSERT(!star)("Only one Star node (or NULL ptr) allowed in a search pattern Collection");
            star = s; // remember for later and skip to next pattern
        }
	    else if( !xremaining.empty() ) // not a Star so match singly...
	    {
	    	// We have to decide which node in the tree to match, so use the present conjecture
	    	// Note: would like to use xremaining, but it will fall out of scope
	    	// Report a block for the chosen node
            ContainerInterface::iterator xit;

            if( query.IsAlreadyGotNextOldDecision() )
            {
                // Decision already in conjecture and valid. 
                const Conjecture::Range &old_decision = query.GetNextOldDecision();
                
                // Now take a copy. Would like range-for here #54
                xremaining.clear();
                for( ContainerInterface::iterator it=old_decision.container->begin();
                     it != old_decision.container->end();
                     ++it )
                    xremaining.push_back(*it);
                    
                // re-submit the exact same decision.
                xit = query.AddNextOldDecision();
            }
            else
            {
                // New decision: take a copy of xremaining and pass it to the query.
                // Use a shared_ptr<> so that the exact same Collection<Node> will come back to us 
                // in future calls.
                auto x_decision = make_shared< Collection<Node> >();
                *x_decision = xremaining;
                xit = query.AddDecision( x_decision, false );
            }
            query.AddLocalLink( false, pia, *xit );

	    	// Remove the chosen element from the remaineder collection. If it is not there (ret val==0)
	    	// then the present chosen iterator has been chosen before and the choices are conflicting.
	    	// We'll just return false so we do not stop trying further choices (xit+1 may be legal).
	    	if( xremaining.erase( *xit ) == 0 )
            {
                ASSERT(!"failed to remove element from collection");
            }
	    }
	    else // ran out of x elements - local mismatch
        {
            TRACE("mismatch - x ran out\n");
            query.AddLocalMatch(false);
            return;
        }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining.empty() && !star )
    {
        TRACE("mismatch - x left over\n");
        query.AddLocalMatch(false); // there were elements left over and no star to match them against
        return;
    }

    TRACE("seen_star %d size of xremaining %d\n", !!star, xremaining.size() );

    // Apply pre-restriction to the star
    if( star )
    {
        TreePtr<StarAgent::SubCollection> x_subcollection( new StarAgent::SubCollection );
        *x_subcollection = xremaining;
        query.AddLocalLink( false, star, x_subcollection );
    }
    TRACE("matched\n");
}


void StandardAgent::TrackingKey( Agent *from )
{
    INDENT(".");
    ASSERT( from );
    TreePtr<Node> key = from->GetCoupled();
    ASSERT( key );
    TRACE(*this)("::TrackingKey(")(*(key))(" from ")(*(from))(")\n");
    
    if( GetCoupled() )
        return; // Already keyed, no point wasting time keying this (and the subtree under it) again
        
    if( !IsLocalMatch(from) ) 
        return; // Not compatible with pattern: recursion stops here
        
    DoKey( key );
    
    // Loop over all the elements of keynode and dest that do not appear in pattern or
    // appear in pattern but are NULL TreePtr<>s. Duplicate from keynode into dest.
    vector< Itemiser::Element * > pattern_memb = Itemise(); 
    vector< Itemiser::Element * > keyer_memb = Itemise( from ); 
    
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<pattern_memb.size(); i++ )
    {
        ASSERT( pattern_memb[i] )( "itemise returned null element" );
        ASSERT( keyer_memb[i] )( "itemise returned null element" );
        
        TRACE("Member %d\n", i );

        if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
            TreePtrInterface *keyer_ptr = dynamic_cast<TreePtrInterface *>(keyer_memb[i]);
            if( *pattern_ptr )
            {
                ASSERT(*keyer_ptr)("Cannot key intermediate because correpsonding search node is NULL");
                AsAgent(*pattern_ptr)->TrackingKey( AsAgent(*keyer_ptr) );
            }
        }
    }
}


TreePtr<Node> StandardAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT(".");
    if( keynode && IsLocalMatch(keynode.get()) ) 
        return BuildReplaceOverlay( keynode );
    else
        return BuildReplaceNormal(); // Overwriting pattern over dest, need to make a duplicate 
}


TreePtr<Node> StandardAgent::BuildReplaceOverlay( TreePtr<Node> keynode )  // under substitution if not NULL
{
	INDENT(" ");
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
    dest = DuplicateNode( keynode, true );

    // Loop over the elements of pattern and dest, limited to elements
    // present in pattern, which is a non-strict subclass of keynode and dest.
    // Overlay or overwrite pattern over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > dest_memb = Itemise( dest.get() ); // Get the members of dest corresponding to pattern's class
    ASSERT( pattern_memb.size() == dest_memb.size() );
    Set< Itemiser::Element * > present_in_pattern; // Repeatability audit: OK since only checking for existance 
    
    TRACE("Copying %d members from pattern=%s dest=%s\n", dest_memb.size(), TypeInfo(this).name().c_str(), TypeInfo(dest).name().c_str());
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("member %d from pattern\n", i );
        ASSERT( pattern_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        if( ContainerInterface *pattern_con = dynamic_cast<ContainerInterface *>(pattern_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            ASSERT( dest_con )( "itemise for dest didn't match itemise for pattern_con");
            dest_con->clear();

            TRACE("Copying container size %d from pattern_con\n", (*pattern_con).size() );
	        FOREACH( const TreePtrInterface &p, *pattern_con )
	        {
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*this)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
				TreePtr<Node> n = AsAgent(p)->BuildReplace();
                ASSERT(n); 
                if( ContainerInterface *psc = dynamic_cast<ContainerInterface *>(n.get()) )
                {
                    TRACE("Walking SubContainer length %d\n", psc->size() );
                    FOREACH( const TreePtrInterface &pp, *psc )
                        dest_con->insert( pp );
                }
                else 
                {
                    TRACE("inserting %s directly\n", TypeInfo(n).name().c_str());
                    ASSERT( n->IsFinal() )("Got intermediate node ")(*n);
                    dest_con->insert( n );
                }
	        }
	        present_in_pattern.insert( dest_memb[i] );
        }            
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for pattern");
            TreePtr<Node> pattern_child = *pattern_ptr;
            TreePtr<Node> dest_child = *dest_ptr;
                       
            if( pattern_child )
            {                             
                dest_child = AsAgent(pattern_child)->BuildReplace();
                ASSERT( dest_child );                
                present_in_pattern.insert( dest_memb[i] );
            }
            ASSERT( dest_child->IsFinal() );
            *dest_ptr = dest_child;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }        
    }
    
    // Loop over all the elements of keynode and dest that do not appear in pattern or
    // appear in pattern but are NULL TreePtr<>s. Duplicate from keynode into dest.
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise();
    dest_memb = keynode->Itemise( dest.get() ); 
    
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
		        TreePtr<Node> n = DuplicateSubtree( p );
		        if( ContainerInterface *sc = dynamic_cast<ContainerInterface *>(n.get()) )
		        {
			        TRACE("Walking SubContainer length %d\n", sc->size() );
		            FOREACH( const TreePtrInterface &xx, *sc )
			            dest_con->insert( xx );
           		}
		        else
		        {
			        TRACE("inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
                    ASSERT( n->IsFinal() );
		        }
	        }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr );
            *dest_ptr = DuplicateSubtree( *keynode_ptr );
            ASSERT( *dest_ptr );
            ASSERT( (**dest_ptr).IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    ASSERT( dest );
    return dest;
}

    
TreePtr<Node> StandardAgent::BuildReplaceNormal() 
{
	INDENT(" ");
 
	// Make a new node, force dirty because from pattern
    // Use clone here because we never want to place an Agent object in the output program tree.
    // Identifiers that have multiple referneces in the pattern will be coupled, and  
    // after the first hit, BuildReplaceOverlay() will handle the rest and it uses Duplicate()
    shared_ptr<Cloner> dup_dest = Clone();
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    engine->GetOverallMaster()->dirty_grass.insert( dest );

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
			            dest_con->insert( pp );  
           		}
		        else
		        {
			        TRACE("inserting %s directly\n", TypeInfo(n).name().c_str());
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
            ASSERT( (**dest_ptr).IsFinal() )("Member %d (", i)(**pattern_ptr)(") of ")(*this)(" was not final\n"); 
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    return dest;
}


void StandardAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The graph plotter pre-loads the variables with the appearance of a standard node
	// that will be used when plotting input/output trees that have not been 
	// agentised. We want the same, so leave them alone.
}

