#include "../search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "overlay_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"

using namespace SR;

//#define ERASE_USING_ITERATOR
//#define CHECK_ITERATOR_IN_CONTAINER

void StandardAgent::AgentConfigure( const SCREngine *master_scr_engine )
{
    plan.ConstructPlan( this );    
    AgentCommon::AgentConfigure(master_scr_engine);
}


void StandardAgent::Plan::ConstructPlan( StandardAgent *algo_ )
{
    algo = algo_;
    const vector< Itemiser::Element * > pattern_memb = algo->Itemise();
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )        
            sequences.emplace( make_pair(pattern_seq, Sequence(this, pattern_seq)) );
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )        
            collections.emplace( make_pair(pattern_col, Collection(this, pattern_col)) );
    }
    MakePatternQuery();
    algo->planned = true;
}


StandardAgent::Plan::Sequence::Sequence( Plan *plan, SequenceInterface *pattern )
{
    num_non_star = 0;
    num_stars = 0;
    SequenceInterface::iterator pit_prev;
    TreePtr<Node> pe_prev;
    PatternLink plink_prev;
    PatternLink non_star_queue;
    list<PatternLink> stars_queue;
	for( SequenceInterface::iterator pit = pattern->begin(); 
         pit != pattern->end(); 
         ++pit ) 
    {
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
        PatternLink plink(plan->algo, &*pit);
        ASSERT( plink );
        if( dynamic_pointer_cast<StarAgent>(pe) )
        {               
            pit_last_star = pit;
            num_stars++;
            stars_queue.push_back( plink );
            if( plink_prev && !dynamic_pointer_cast<StarAgent>(pe_prev) )
                adjacent_non_star_star.insert( make_pair( plink, plink_prev ) );
        }
        else
        {
            if( non_star_queue ) 
            {
                auto non_star_pair = make_pair( non_star_queue, plink );
                if( stars_queue.size() > 0 )                
                    gapped_non_stars.insert( non_star_pair );                                
                else            
                    adjacent_non_stars.insert( non_star_pair );                
            }
            while( stars_queue.size() >= 1 )
            {
                auto star_pair = make_pair( stars_queue.front(), plink );
                if( stars_queue.size() > 1 )
                    gapped_star_non_star.insert( star_pair );
                else
                    adjacent_star_non_star.insert( star_pair );                
                stars_queue.pop_front();
            }
            non_stars.insert( plink );
                    
            num_non_star++;
            non_star_queue = plink;
        }     
        pit_prev = pit;      
        pe_prev = pe;
        plink_prev = plink; 
    }
    
    if( (SequenceInterface::iterator)(pattern->begin()) != pattern->end() )
    {        
        TreePtr<Node> pfront( *(pattern->begin()) );
        if( !dynamic_pointer_cast<StarAgent>(pfront) )
            non_star_at_front = PatternLink(plan->algo, &*(pattern->begin()));
        pit_back = pattern->end();
        --pit_back;
        TreePtr<Node> pback( *pit_back );
        if( !dynamic_pointer_cast<StarAgent>(pback) )
            non_star_at_back = PatternLink(plan->algo, &*pit_back);        
    }
}


StandardAgent::Plan::Collection::Collection( Plan *plan, CollectionInterface *pattern )
{
    p_star = nullptr;
    for( CollectionInterface::iterator pit = pattern->begin(); 
         pit != pattern->end(); 
         ++pit )                 
    {
        const TreePtrInterface *pe = &*pit; 
        auto plink = PatternLink(plan->algo, &*pit);
		ASSERT( pe );
        if( dynamic_cast<StarAgent *>(pe->get()) ) // per the impl, the star in a collection is not linked
        {
            // No, could be in replace pattern, where many stars are permitted TODO resolve
            // ASSERT( !p_star )("Only one star allowed in collection");
            
            p_star = pe;
            star_plink = plink;
        }
    }
}


void StandardAgent::Plan::MakePatternQuery()
{
    // Clear it just in case
    pattern_query = make_shared<PatternQuery>(algo);

    const vector< Itemiser::Element * > pattern_memb = algo->Itemise();
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )
        {
            const Plan::Sequence &plan_seq = sequences.at(pattern_seq);

   			for( SequenceInterface::iterator pit = pattern_seq->begin(); pit != pattern_seq->end(); ++pit )                 
   			{
                const TreePtrInterface *pe = &*pit; 
                ASSERT( pe );
                if( dynamic_cast<StarAgent *>(pe->get()) )
                {
                    if( pit != plan_seq.pit_last_star )
                        pattern_query->RegisterDecision( true ); // Inclusive, please.
                    pattern_query->RegisterAbnormalLink(PatternLink(algo, pe));    
                }
                else
                {
                    pattern_query->RegisterNormalLink(PatternLink(algo, pe));    
                }
            }
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )
        {
            const Plan::Collection &plan_col = collections.at(pattern_col);

   			for( CollectionInterface::iterator pit = pattern_col->begin(); pit != pattern_col->end(); ++pit )                 
   			{
                const TreePtrInterface *pe = &*pit; 
				if( !dynamic_cast<StarAgent *>(pe->get()) ) // per the impl, the star in a collection is not linked
                {
                    pattern_query->RegisterDecision( false ); // Exclusive, please
				    pattern_query->RegisterNormalLink(PatternLink(algo, pe));    	     
                }
		    }
            if( plan_col.star_plink )
            {
                pattern_query->RegisterAbnormalLink( plan_col.star_plink );   		                    
            }
        }
        else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(ie) )
        {
            if( TreePtr<Node>(*pattern_ptr) ) // TreePtrs are allowed to be nullptr meaning no restriction            
                pattern_query->RegisterNormalLink(PatternLink(algo, pattern_ptr));
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }
}


shared_ptr<PatternQuery> StandardAgent::GetPatternQuery() const
{
    return plan.pattern_query;
}


void StandardAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         XLink base_xlink ) const
{
    INDENT("Q");
    query.Reset();

    // Check pre-restriction
    CheckLocalMatch(base_xlink.GetChildX().get());

    // Recurse through the children. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. x must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > x_memb = Itemise( base_xlink.GetChildX().get() );   // Get the members of x corresponding to pattern's class
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
            DecidedQuerySequence( query, base_xlink, p_x_seq, pattern_seq );
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
        {
            CollectionInterface *p_x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
            ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
            TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, p_x_col->size(), pattern_col->size() );
            DecidedQueryCollection( query, base_xlink, p_x_col, pattern_col );
        }
        else if( TreePtrInterface *pattern_sing = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
            if( TreePtr<Node>(*pattern_sing) ) // TreePtrs are allowed to be nullptr meaning no restriction
            {
                TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(x_memb[i]);
                auto sing_plink = PatternLink(this, pattern_sing);
                auto sing_xlink = XLink(base_xlink.GetChildX(), p_x_sing);
                ASSERT( p_x_sing )( "itemise for x didn't match itemise for pattern");
                TRACE("Member %d is TreePtr, pattern=", i)(*pattern_sing)("\n");
                query.RegisterNormalLink(sing_plink, sing_xlink); // Link into X
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }
}


void StandardAgent::DecidedQuerySequence( DecidedQueryAgentInterface &query,
                                          XLink base_xlink,
                                          SequenceInterface *px,
		                                  SequenceInterface *pattern ) const
{
    INDENT("S");
    ASSERT( planned );
    const Plan::Sequence &plan_seq = plan.sequences.at(pattern);
    int num_non_star = plan_seq.num_non_star;
    ContainerInterface::iterator pit_last_star = plan_seq.pit_last_star;

    if( px->size() < num_non_star )
    {
        throw Mismatch();     // TODO break to get the final trace?
    }

	ContainerInterface::iterator pit, npit, nnpit, nxit;            
    ContainerInterface::iterator xit_star_limit = px->end();            
    for( int i=0; i<num_non_star; i++ )
        --xit_star_limit;                
        
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = px->begin();
	for( pit = pattern->begin(); pit != pattern->end(); ++pit )
	{
 		ASSERT( xit == px->end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
        if( dynamic_pointer_cast<StarAgent>(pe) )
        {
            // We have a Star type wildcard that can match multiple elements.
            ContainerInterface::iterator xit_star_end;
                
            // The last Star does not need a decision            
            if( pit == pit_last_star )
            {
                // No more stars, so skip ahead to the end of the possible star range. 
                xit_star_end = xit_star_limit;
            }				
            else
            {
                // Decide how many elements the current * should match, using conjecture. The star's range
                // ends at the chosen element. Be inclusive because what we really want is a range.
                xit_star_end = query.RegisterDecision( xit, xit_star_limit, true );
            }
            
            // Star matched [xit, xit_star_end) i.e. xit-xit_begin_star elements
            TreePtr<SubSequenceRange> xss( new SubSequenceRange( base_xlink.GetChildX(), xit, xit_star_end ) );

            // Apply couplings to this Star and matched range
            // Restrict to pre-restriction or pattern restriction
            query.RegisterAbnormalLink( PatternLink(this, &*pit), XLink::CreateDistinct(xss) ); // Only used in after-pass
             
            // Resume at the first element after the matched range
            xit = xit_star_end;
        }
 	    else // not a Star so match singly...
	    {
            if( xit == px->end() )
                break;
       
            query.RegisterNormalLink( PatternLink(this, &*pit), XLink(base_xlink.GetChildX(), &*xit) ); // Link into X
            ++xit;
            
            // Every non-star pattern node we pass means there's one fewer remaining
            // and we can match a star one step further
            ASSERT(xit_star_limit != px->end());
            ++xit_star_limit;
	    }
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==px->end(), pit==pattern->end() );
    if( xit != px->end() )
        throw Mismatch();  
    else if( pit != pattern->end() )
        throw Mismatch();  
}


void StandardAgent::DecidedQueryCollection( DecidedQueryAgentInterface &query,
                                            XLink base_xlink,
                                            CollectionInterface *px,
		 					                CollectionInterface *pattern ) const
{
    INDENT("C");

    const Plan::Collection &plan_col = plan.collections.at(pattern);
    
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
    Collection<Node> xremaining;
    FOREACH( const TreePtrInterface &xe, *px )
        xremaining.insert( xe ); // Note: the new element in xremaining will not be the one from the original x (it's a TreePtr<Node>)
    
    for( CollectionInterface::iterator pit = pattern->begin(); pit != pattern->end(); ++pit )
    {
		TreePtr<Node> pe( *pit );
        if( dynamic_pointer_cast<StarAgent>(pe) )
            continue; // Looping over non-stars only
            
    	TRACE("Collection compare %d remain out of %d; looking at ",
                xremaining.size(),
                pattern->size() )(**pit)(" in pattern\n" );        
        if( xremaining.empty() ) // not a Star so match singly...
        {
            TRACE("mismatch - x ran out\n");
            throw Mismatch();  
        }
            
        // We have to decide which node in the tree to match, so use the present conjecture
        // Note: would like to use xremaining, but it will fall out of scope
        // Report a block for the chosen node
        ContainerInterface::iterator xit;

        if( query.IsNextChoiceValid() )
        {
            // Decision already in conjecture and valid. 
            const Conjecture::Range &old_decision = query.GetNextOldDecision();
            
            // Now take a copy. 
            xremaining.clear();
            FOREACH( const TreePtrInterface &tp, *old_decision.container )
                xremaining.insert((TreePtr<Node>)tp);
                
            // re-submit the exact same decision.
            xit = query.SkipDecision();
        }
        else
        {
            // New decision: take a copy of xremaining and pass it to the query.
            // Use a shared_ptr<> so that the exact same Collection<Node> will come back to us 
            // in future calls.
            auto x_decision = make_shared< Collection<Node> >();
            *x_decision = xremaining;
            xit = query.RegisterDecision( x_decision, false );
        }
        
        // Find the TreePtr in our collection that points to the same
        // node as *xit, in order to preserve uniqueness properties of links.
        // TODO #167 should remove the ened for this
        const TreePtrInterface *my_real_ppx = nullptr;
        FOREACH( const TreePtrInterface &ppx, *px )
        {
            if( ppx == *xit )
                my_real_ppx = &ppx;
        }
        ASSERT( my_real_ppx );            
        query.RegisterNormalLink( PatternLink(this, &*pit), XLink(base_xlink.GetChildX(), my_real_ppx) ); // Link into X

        // Remove the chosen element from the remaineder collection. 
#ifdef CHECK_ITERATOR_IN_CONTAINER
        bool found = false;
        for( ContainerInterface::iterator checkit=xremaining.begin(); checkit != xremaining.end(); ++checkit )
            if( checkit==xit )
                found=true;
        ASSERT( found ); // If not found, we have a detached iterator, see #167
#endif
#ifdef ERASE_USING_ITERATOR
        xremaining.erase( xit );
#else           
        int n = xremaining.erase( *xit );
        if( n == 0 )
        {
            ASSERT(!"failed to remove element from collection");
        }
        if( n > 1 )
        {
            ASSERT(false)("Removed ")(n)(" elements\n");
        }
#endif        
    }

    // Now handle the p_star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining.empty() && !plan_col.star_plink )
    {
        TRACE("mismatch - x left over\n");
        throw Mismatch();   // there were elements left over and no p_star to match them against
    }

    TRACE("seen_star %d size of xremaining %d\n", !!plan_col.star_plink, xremaining.size() );
    
    if( plan_col.star_plink )
    {
        // Apply pre-restriction to the star
        TreePtr<SubCollection> x_subcollection( new SubCollection );
 
        // For replace...
        *x_subcollection = xremaining;

        // For solver...
        // Find the TreePtrs in our collection that point to the same
        // nodes as xremaining, in order to preserve uniqueness properties of links.
        // TODO #167 should remove the ened for this   
        FOREACH( const TreePtrInterface &ppx, *px )
        {
            if( xremaining.count(ppx) > 0 )
                x_subcollection->elts.insert( XLink(base_xlink.GetChildX(), &ppx) );
        }
 
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass
    }
    TRACE("matched\n");
}


bool StandardAgent::ImplHasDNLQ() const
{
    return true;
}


Agent::Completeness StandardAgent::RunDecidedNormalLinkedQueryImpl( DecidedQueryAgentInterface &query,
                                                                    XLink base_xlink,
                                                                    const SolutionMap *required_links,
                                                                    const TheKnowledge *knowledge ) const
{ 
    INDENT("Q");
    query.Reset();
    Completeness completeness = COMPLETE;

    // Check pre-restriction
    TRACE(*this)("::CheckLocalMatch(")(base_xlink)(")\n");
    CheckLocalMatch(base_xlink.GetChildX().get());

    // Recurse through the children. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. x must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > x_memb = Itemise( base_xlink.GetChildX().get() );   // Get the members of x corresponding to pattern's class
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
            DecidedNormalLinkedQuerySequence( query, base_xlink, p_x_seq, pattern_seq, required_links, knowledge, completeness );
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
        {
            CollectionInterface *p_x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
            ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
            TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, p_x_col->size(), pattern_col->size() );
            DecidedNormalLinkedQueryCollection( query, base_xlink, p_x_col, pattern_col, required_links, knowledge, completeness );
        }
        else if( TreePtrInterface *pattern_sing = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
        {
            if( TreePtr<Node>(*pattern_sing) ) // TreePtrs are allowed to be nullptr meaning no restriction
            {
                TreePtrInterface *p_x_sing = dynamic_cast<TreePtrInterface *>(x_memb[i]);
                ASSERT( p_x_sing )( "itemise for x didn't match itemise for pattern");
                auto sing_plink = PatternLink(this, pattern_sing);
                auto sing_xlink = XLink(base_xlink.GetChildX(), p_x_sing);
                
                TRACE("Member %d is singlular, pattern=", i)(sing_plink)(" x=")(sing_xlink)(" required x=")(required_links->at( sing_plink ))("\n");
                SolutionMap::const_iterator req_sing_it = required_links->find(sing_plink);
                if( req_sing_it == required_links->end() ) 
                {
                    completeness = INCOMPLETE;
                }
                else
                {
                    XLink req_sing_xlink = req_sing_it->second;                 
                    if( sing_xlink != req_sing_xlink )
                        throw SingularMismatch();
                }
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }
    return completeness;
}


void StandardAgent::DecidedNormalLinkedQuerySequence( DecidedQueryAgentInterface &query,
                                                      XLink base_xlink,
                                                      SequenceInterface *px,
                                                      SequenceInterface *pattern,
                                                      const SolutionMap *required_links,
                                                      const TheKnowledge *knowledge,
                                                      Completeness &completeness ) const
{
    INDENT("S");
    ASSERT( planned );

    const Plan::Sequence &plan_seq = plan.sequences.at(pattern);
    int num_non_star = plan_seq.num_non_star;
    ContainerInterface::iterator pit_last_star = plan_seq.pit_last_star;

    TRACEC("DNLQ sequence: %d plinks of which %d are non-star\n", pattern->size(), num_non_star);

    for( PatternLink plink : plan_seq.non_stars ) // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plink);
        if( it == required_links->end() ) 
        {
            completeness = INCOMPLETE;
            continue;
        }
        const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
        if( !(nugget.cadence == TheKnowledge::Nugget::IN_SEQUENCE &&
              nugget.container == px) )
            throw WrongContainerSequenceMismatch(); // Be in the right sequence
    }
    
    if( plan_seq.non_star_at_front ) // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plan_seq.non_star_at_front);
        if( it != required_links->end() ) 
        {        
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( nugget.iterator != px->begin() )
                throw NotAtFrontMismatch();
        }
    }

    if( plan_seq.non_star_at_back ) // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plan_seq.non_star_at_back);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            ContainerInterface::iterator it_incremented = nugget.iterator;
            ++it_incremented;        
            if( it_incremented != px->end() )
                throw NotAtBackMismatch();
        }
    }

    for( pair<PatternLink, PatternLink> p : plan_seq.adjacent_non_stars ) // independent of px
    {
        SolutionMap::const_iterator a_it = required_links->find(p.first);
        SolutionMap::const_iterator b_it = required_links->find(p.second);
        if( a_it == required_links->end() || b_it == required_links->end() ) 
            continue;
        const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_it->second) );        
        const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_it->second) );       
        ContainerInterface::iterator a_it_incremented = a_nugget.iterator;
        ++a_it_incremented;
        if( a_it_incremented != b_nugget.iterator )
             throw NotSuccessorSequenceMismatch();
    }
    
    for( pair<PatternLink, PatternLink> p : plan_seq.gapped_non_stars ) // independent of px
    {
        SolutionMap::const_iterator a_it = required_links->find(p.first);
        SolutionMap::const_iterator b_it = required_links->find(p.second);
        if( a_it == required_links->end() || b_it == required_links->end() ) 
            continue;
        const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_it->second) );        
        const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_it->second) );        
        if( a_nugget.index >= b_nugget.index )
             throw NotAfterSequenceMismatch();
    }

	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit;
	for( ContainerInterface::iterator pit = pattern->begin(); pit != pattern->end(); ++pit ) // --------------- loop -------------------
	{
        PatternLink plink( this, &*pit );

        if( dynamic_pointer_cast<StarAgent>(TreePtr<Node>(*pit)) ) // ------------ STAR ---------------
        {
            // We have a Star type wildcard that can match multiple elements.
            ContainerInterface::iterator xit_star_end;
            PatternLink limit_plink;
            bool register_decision;
                
            if( plan_seq.adjacent_star_non_star.count( plink ) > 0 ) // Next plink is a non-star
            {
                TRACEC("Star adjacent non-star ")(plink)(" ")(limit_plink)("\n");               
                limit_plink = plan_seq.adjacent_star_non_star.at(plink);
                register_decision = false;
            }            
            else if( plan_seq.gapped_star_non_star.count( plink ) > 0 ) // Next plink is a star but a non-star exists after here
            {
                TRACEC("Star gapped non-star ")(plink)(" ")(limit_plink)("\n");
                limit_plink = plan_seq.gapped_star_non_star.at(plink);               
                register_decision = true;
            }	
            else if( pit == plan_seq.pit_back ) // Star at back does not need a decision or a bounding non-star
            {
                // No more stars, so skip ahead to the end of the possible star range. 
                TRACEC("Star at back ")(plink)("\n");
                register_decision = false;
            }	
            else // Next plink is another star
            {
                // Decide how many elements the current * should match, using conjecture. The star's range
                // ends at the chosen element. Be inclusive because what we really want is a range.
                TRACEC("Star before another star ")(plink)("\n");
                register_decision = true;
            }
            
            ContainerInterface::iterator xit_star_begin;
            if( plan_seq.adjacent_non_star_star.count( plink ) > 0 ) // prev was a non-star
            {
                PatternLink begin_plink = plan_seq.adjacent_non_star_star.at(plink);               
                SolutionMap::const_iterator begin_it = required_links->find(begin_plink);
                if( begin_it == required_links->end() )         
                    return; // can't do any more in the current sequence TODO I bet you could if you tried harder                    
                XLink begin_xlink = begin_it->second; 
                const TheKnowledge::Nugget &begin_nugget( knowledge->GetNugget(begin_xlink) );                        
                xit_star_begin = begin_nugget.iterator;
                ++xit_star_begin; // get past the non-star
            }
            else if( pit == pattern->begin() ) // no prev, so begin at the beginning of x
            {
                xit_star_begin = px->begin();
            }
            else // prev was star, we need to start where it left off, so need xit from last decision
            {
                xit_star_begin = xit;
            }
                        
            ContainerInterface::iterator xit_star_limit;
            if( limit_plink ) // there's a non-star after us which will set a limit
            {
                SolutionMap::const_iterator limit_it = required_links->find(limit_plink);
                if( limit_it == required_links->end() )         
                    return; // can't do any more in the current sequence TODO I bet you could if you tried harder                    
                XLink limit_xlink = limit_it->second; 
                const TheKnowledge::Nugget &limit_nugget( knowledge->GetNugget(limit_xlink) );                        
                xit_star_limit = limit_nugget.iterator;
            }
            else // No more non-stars after us so limit is the end of x 
            {
                xit_star_limit = px->end();
            }
            
            if( register_decision ) // Another star follows this one, forcing a decision
            {
                xit_star_end = query.RegisterDecision( xit_star_begin, xit_star_limit, true );
                xit = xit_star_end;
            }
            else // No decision needed, we go all the way up to the limit
            {
                xit_star_end = xit_star_limit;
            }
            
            // Star matched [xit_star_begin, xit_star_end) i.e. xit-xit_begin_star elements
            TreePtr<SubSequenceRange> xss( new SubSequenceRange( base_xlink.GetChildX(), xit_star_begin, xit_star_end ) );

            // Apply couplings to this Star and matched range
            // Restrict to pre-restriction or pattern restriction
            query.RegisterAbnormalLink( plink, XLink::CreateDistinct(xss) ); // Only used in after-pass
        }
    } // ----------------------------------------------- end of loop ------------------------------------------------------
}


void StandardAgent::DecidedNormalLinkedQueryCollection( DecidedQueryAgentInterface &query,
                                                        XLink base_xlink,
                                                        CollectionInterface *px,
                                                        CollectionInterface *pattern,
                                                        const SolutionMap *required_links,
                                                        const TheKnowledge *knowledge,
                                                        Completeness &completeness ) const
{
    INDENT("C");
    
    const Plan::Collection &plan_col = plan.collections.at(pattern);
    
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
    set<XLink> remaining_xlinks;
    FOREACH( const TreePtrInterface &xe, *px )
        remaining_xlinks.insert( XLink( base_xlink.GetChildX(), &xe ) ); // Note: the new element in xremaining will not be the one from the original x (it's a TreePtr<Node>)
    
    PatternLink star_plink;

    for( CollectionInterface::iterator pit = pattern->begin(); pit != pattern->end(); ++pit )
    {
        auto plink = PatternLink(this, &*pit);
        if( dynamic_pointer_cast<StarAgent>(TreePtr<Node>(*pit)) ) 
            continue; // Looping over non-stars only

    	TRACE("Collection compare %d remain out of %d; looking at ",
                remaining_xlinks.size(),
                pattern->size() )(**pit)(" in pattern\n" );
            
        SolutionMap::const_iterator req_it = required_links->find(plink);
        if( req_it == required_links->end() ) 
        {
            completeness = INCOMPLETE;
            return; // can't do any more in the current sequence TODO I bet you could if you tried harder
        }
        
        XLink req_xlink = req_it->second; 
        const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );        
        if( !(nugget.cadence == TheKnowledge::Nugget::IN_COLLECTION &&
              nugget.container == px) )
            throw WrongContainerCollectionMismatch(); // Be in the right collection
            
        if( remaining_xlinks.count( req_xlink ) == 0 )
            throw CollisionCollectionMismatch(); // Already removed this one: collision
        remaining_xlinks.erase( req_xlink );
    }
    
    // Now handle the p_star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !remaining_xlinks.empty() && !plan_col.star_plink )
    {
        TRACE("mismatch - x left over\n");
        throw SurplusXCollectionMismatch();   // there were elements left over and no p_star to match them against
    }

    TRACE("seen_star %d size of xremaining %d\n", !!plan_col.star_plink, remaining_xlinks.size() );
    
    if( plan_col.star_plink )
    {
        // Apply pre-restriction to the star
        TreePtr<SubCollection> x_subcollection( new SubCollection );
 
        // For replace...  
        for( XLink xlink : remaining_xlinks )
        {            
            x_subcollection->insert( xlink.GetChildX() );
        }

        // For solver...
        x_subcollection->elts = remaining_xlinks;
 
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass
    }    
}


void StandardAgent::TrackingKey( Agent *from )
{
    INDENT("T");
    ASSERT( from );
    CouplingKey key = from->GetKey();
    ASSERT( key );
    TRACE(*this)("::TrackingKey(")(key)(" from ")(*(from))(")\n");
    
    if( GetKey() )
        return; // Already keyed, no point wasting time keying this (and the subtree under it) again
        
    if( !IsLocalMatch(from) ) 
        return; // Not compatible with pattern: recursion stops here
        
    SetKey( key );
    
    // Loop over all the elements of keynode and dest that do not appear in pattern or
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from keynode into dest.
    vector< Itemiser::Element * > pattern_memb = Itemise(); 
    vector< Itemiser::Element * > keyer_memb = Itemise( from ); 
    
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
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
                ASSERT(*keyer_ptr)("Cannot key intermediate because correpsonding search node is nullptr");
                AsAgent((TreePtr<Node>)*pattern_ptr)->TrackingKey( AsAgent((TreePtr<Node>)*keyer_ptr) );
            }
        }
    }
}


TreePtr<Node> StandardAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("B");
    if( keylink && IsLocalMatch(keylink.GetChildX().get()) ) 
        return BuildReplaceOverlay( keylink.GetChildX() );
    else
        return BuildReplaceNormal(); // Overwriting pattern over dest, need to make a duplicate 
}


TreePtr<Node> StandardAgent::BuildReplaceOverlay( TreePtr<Node> keynode )  // under substitution if not nullptr
{
	INDENT("O");
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
    set< Itemiser::Element * > present_in_pattern; // Repeatability audit: OK since only checking for existance 
    
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
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*this)(" was nullptr\n");
		        TRACE("Got ")(*p)("\n");
				TreePtr<Node> n = AsAgent((TreePtr<Node>)p)->BuildReplace();
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
            auto pattern_child = (TreePtr<Node>)*pattern_ptr;
            auto dest_child = (TreePtr<Node>)*dest_ptr;
                       
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
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from keynode into dest.
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise();
    dest_memb = keynode->Itemise( dest.get() ); 
    
    TRACE("Copying %d members from keynode=%s dest=%s\n", dest_memb.size(), TypeInfo(keynode).name().c_str(), TypeInfo(dest).name().c_str());
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( present_in_pattern.count(dest_memb[i]) > 0 )
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
		        ASSERT( p ); // present simplified scheme disallows nullptr
		        TreePtr<Node> n = DuplicateSubtree( (TreePtr<Node>)p );
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
            *dest_ptr = DuplicateSubtree( (TreePtr<Node>)*keynode_ptr );
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
	INDENT("N");
 
	// Make a new node, force dirty because from pattern
    // Use clone here because we never want to place an Agent object in the output program tree.
    // Identifiers that have multiple referneces in the pattern will be coupled, and  
    // after the first hit, BuildReplaceOverlay() will handle the rest and it uses Duplicate()
    shared_ptr<Cloner> dup_dest = Clone();
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    master_scr_engine->GetOverallMaster()->dirty_grass.insert( dest );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. dest must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Copying %d members pattern=", dest_memb.size())(*this)(" dest=")(*dest)("\n");
    // Loop over all the members of pattern (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
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
		        ASSERT( p )("Some element of member %d (", i)(*pattern_con)(") of ")(*this)(" was nullptr\n");
		        TRACE("Got ")(*p)("\n");
	            TreePtr<Node> n = AsAgent((TreePtr<Node>)p)->BuildReplace();
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
            ASSERT( *pattern_ptr )("Member %d (", i)(*pattern_ptr)(") of ")(*this)(" was nullptr when not overlaying\n");
            *dest_ptr = AsAgent((TreePtr<Node>)*pattern_ptr)->BuildReplace();
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

