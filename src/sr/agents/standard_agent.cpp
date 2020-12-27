#include "../search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "overlay_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"

using namespace SR;

void StandardAgent::AgentConfigure( Phase phase, const SCREngine *master_scr_engine )
{
    plan.ConstructPlan( this, phase );    
    AgentCommon::AgentConfigure(phase, master_scr_engine);
}


void StandardAgent::Plan::ConstructPlan( StandardAgent *algo_, Phase phase )
{
    algo = algo_;
    const vector< Itemiser::Element * > pattern_memb = algo->Itemise();
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )        
            sequences.emplace( make_pair(pattern_seq, Sequence(this, phase, pattern_seq)) );
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )        
            collections.emplace( make_pair(pattern_col, Collection(this, phase, pattern_col)) );
    }
    algo->planned = true;
}


StandardAgent::Plan::Sequence::Sequence( Plan *plan, Phase phase, SequenceInterface *pattern )
{
    if( phase == IN_REPLACE_ONLY )
        return;
    
    num_non_star = 0;
    SequenceInterface::iterator pit_prev;
    TreePtr<Node> pe_prev;
    PatternLink plink_prev;
    PatternLink non_star_queue;
    list<PatternLink> stars_queue;
    
    auto run_lambda = [&](PatternLink plink)
    {
        auto run = make_shared<Run>();
        run->predecessor = non_star_queue;
        run->elts = stars_queue;
        run->successor = plink;
        star_runs.insert( run );    
        stars_queue.clear();            
    };
    
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
            stars_queue.push_back( plink );
        }
        else // plink is non-star
        {
            if( non_star_queue ) 
            {
                auto non_star_pair = make_pair( non_star_queue, plink );
                if( stars_queue.size() > 0 )                
                    gapped_non_stars.insert( non_star_pair );                                
                else            
                    adjacent_non_stars.insert( non_star_pair );                
            }
            run_lambda( plink );
            non_stars.insert( plink );
                    
            num_non_star++;
            non_star_queue = plink;
        }     
        pit_prev = pit;      
        pe_prev = pe;
        plink_prev = plink; 
    }
    run_lambda( PatternLink() );
    
    if( (SequenceInterface::iterator)(pattern->begin()) != pattern->end() )
    {        
        plink_front = PatternLink(plan->algo, &*(pattern->begin()));
        TreePtr<Node> pfront( *(pattern->begin()) );
        if( !dynamic_pointer_cast<StarAgent>(pfront) )
            non_star_at_front = plink_front;
            
        SequenceInterface::iterator pit_back = pattern->end();
        --pit_back;
        TreePtr<Node> pback( *pit_back );
        plink_back = PatternLink(plan->algo, &*pit_back);
        if( !dynamic_pointer_cast<StarAgent>(pback) )
            non_star_at_back = plink_back;        
    }
}


StandardAgent::Plan::Collection::Collection( Plan *plan, Phase phase, CollectionInterface *pattern )
{
    if( phase == IN_REPLACE_ONLY )
        return;

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
            ASSERT( !p_star )("Only one star allowed in collections when in compare pattern");
            
            p_star = pe;
            star_plink = plink;
        }
        else
        {
            non_stars.insert( plink );
        }
    }
}


shared_ptr<PatternQuery> StandardAgent::GetPatternQuery() const
{
    // Clear it just in case
    auto pattern_query = make_shared<PatternQuery>(this);

    const vector< Itemiser::Element * > pattern_memb = Itemise();
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )
        {
            const Plan::Sequence &plan_seq = plan.sequences.at(pattern_seq);

   			for( SequenceInterface::iterator pit = pattern_seq->begin(); pit != pattern_seq->end(); ++pit )                 
   			{
                const TreePtrInterface *pe = &*pit; 
                ASSERT( pe );
                if( dynamic_cast<StarAgent *>(pe->get()) )
                {
                    if( pit != plan_seq.pit_last_star )
                        pattern_query->RegisterDecision( true ); // Inclusive, please.
                    pattern_query->RegisterAbnormalLink(PatternLink(this, pe));    
                }
                else
                {
                    pattern_query->RegisterNormalLink(PatternLink(this, pe));    
                }
            }
        }
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )
        {
            const Plan::Collection &plan_col = plan.collections.at(pattern_col);

   			for( CollectionInterface::iterator pit = pattern_col->begin(); pit != pattern_col->end(); ++pit )                 
   			{
                const TreePtrInterface *pe = &*pit; 
				if( !dynamic_cast<StarAgent *>(pe->get()) ) // per the impl, the star in a collection is not linked
                {
                    pattern_query->RegisterDecision( false ); // Exclusive, please
				    pattern_query->RegisterNormalLink(PatternLink(this, pe));    	     
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
                pattern_query->RegisterNormalLink(PatternLink(this, pattern_ptr));
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }
    
    return pattern_query;
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

    SubCollectionRange::ExclusionSet excluded_x;
    
    for( PatternLink plink : plan_col.non_stars )
    {
    	TRACE( "Collection compare %d excluded out of %d; looking at ",
               excluded_x.size(),
               pattern->size() )
             (plink)
             (" in pattern\n" );        

        // We have to decide which node in the tree to match, so use the present conjecture
        // Make a SubCollectionRange excluding x elements we already matched
        ContainerInterface::iterator xit;
        auto x_decision = make_shared< SubCollectionRange >( base_xlink.GetChildX(), px->begin(), px->end() );
        x_decision->SetExclusions( excluded_x );                       
                   
        // An empty decision would imply we ran out of elements in px
        if( x_decision->empty() )
            throw InsufficientXCollectionMismatch();                 
                   
        // No need to provide the container x_decision; iterators will keep it alive and are
        // not invalidated by re-construction of the container (they're proxies for iterators on px).
        xit = query.RegisterDecision( x_decision->begin(), x_decision->end(), false );    
        
        // We have our x element
        query.RegisterNormalLink( plink, XLink(base_xlink.GetChildX(), &*xit) ); // Link into X
        excluded_x.insert( &*xit );        
    }

    // Now handle the p_star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.
    TRACE("seen_star %d size of excluded_x %d\n", !!plan_col.star_plink, excluded_x.size() );
    
    if( plan_col.star_plink )
    {
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( base_xlink.GetChildX(), px->begin(), px->end() ) );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!
    }
    else if( excluded_x.size() != px->size() )
    {
        throw SurplusXCollectionMismatch();   // there were elements left over and no p_star to match them against
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

    TRACEC("DNLQ sequence: %d plinks\n", pattern->size());
    bool incomplete = false;

    for( PatternLink plink : plan_seq.non_stars )  // independent of px
    {
        if( required_links->find(plink) == required_links->end() ) 
        {
            completeness = INCOMPLETE;
            incomplete = true;
        }
    }
    
    // The only true unary constraint is that every child x link
    // is in some sequence (because that can be read directly off the
    // nugget).
    for( PatternLink plink : plan_seq.non_stars )  // independent of px
    {
        SolutionMap::const_iterator it = required_links->find(plink);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( !(nugget.cadence == TheKnowledge::Nugget::IN_SEQUENCE) )
                throw WrongContainerSequenceMismatch(); // Be in the right sequence        
        }
    }
    
    // Require that every child x link is in the correct collection.
    // Note: checking px only on non_star_at_front and non_star_at_back
    // is insufficient - they might both be stars.
    for( PatternLink plink : plan_seq.non_stars )  // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plink);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( !(nugget.container == px) )
                throw WrongContainerSequenceMismatch(); // Be in the right sequence        
        }
    }
    
    // If the pattern begins with a non-star, constrain the child x to be the 
    // front node in the collection at our base x. Uses base so a binary constraint.
    if( plan_seq.non_star_at_front ) // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plan_seq.non_star_at_front);
        if( it != required_links->end() ) 
        {        
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( !(nugget.container == px && nugget.iterator == px->begin()) )
                throw NotAtFrontMismatch();
        }
    }

    // If the pattern ends with a non-star, constrain the child x to be the 
    // back node in the collection at our base x. Uses base so a binary constraint.
    if( plan_seq.non_star_at_back ) // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plan_seq.non_star_at_back);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            ContainerInterface::iterator it_incremented = nugget.iterator;
            ++it_incremented;        
            if( !(nugget.container == px && it_incremented == px->end()) )
                throw NotAtBackMismatch();
        }
    }

    // Adjacent pairs of non-stars in the pattern should correspond to adjacent
    // pairs of child x nodes. Only needs the two child x nodes, so binary constraint.
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
        if( !(a_nugget.container == b_nugget.container && a_it_incremented == b_nugget.iterator) )
             throw NotSuccessorSequenceMismatch();
    }
    
    // Gapped pairs of non-stars in the pattern (i.e. stars in between) should 
    // correspond to pairs of child x nodes that are ordered correctly. Only needs 
    // the two child x nodes, so binary constraint.    
    for( pair<PatternLink, PatternLink> p : plan_seq.gapped_non_stars ) // independent of px
    {
        SolutionMap::const_iterator a_it = required_links->find(p.first);
        SolutionMap::const_iterator b_it = required_links->find(p.second);
        if( a_it == required_links->end() || b_it == required_links->end() ) 
            continue;
        const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_it->second) );        
        const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_it->second) );        
        if( !(a_nugget.container == b_nugget.container && a_nugget.index < b_nugget.index) )
             throw NotAfterSequenceMismatch();
    }

    if( incomplete )
        return;

    // If we got this far with an undersized px, something has gone wrong 
    // in the logic, and the following code will crash into px->end(). 
    // And I mean "crash" literally.
    ASSERT(px->size() >= plan_seq.num_non_star); 

    // We now look at the runs of star patterns. Each is bounded by some combination
    // of the bounds of the x sequence and the surrounding non-star child x values. 
    for( shared_ptr<Plan::Sequence::Run> run : plan_seq.star_runs )
    {
      	ContainerInterface::iterator xit, xit_star_limit;

        if( run->predecessor )
        {
            SolutionMap::const_iterator pred_it = required_links->find(run->predecessor);
            if( pred_it == required_links->end() )         
                break; // can't do any more in the current run
            XLink pred_xlink = pred_it->second; 
            const TheKnowledge::Nugget &pred_nugget( knowledge->GetNugget(pred_xlink) );                        
            xit = pred_nugget.iterator;
            ++xit; // get past the non-star
        }
        else
        {
            xit = px->begin();
        }
        
        if( run->successor )
        {
            SolutionMap::const_iterator succ_it = required_links->find(run->successor);
            if( succ_it == required_links->end() )         
                break; // can't do any more in the current run
            XLink succ_xlink = succ_it->second; 
            const TheKnowledge::Nugget &succ_nugget( knowledge->GetNugget(succ_xlink) );                        
            xit_star_limit = succ_nugget.iterator;
        }
        else
        {
            xit_star_limit = px->end();
        }
        
        // Within a run of star patterns, register decisions for all but the last
        // and for all of them register a subsequence range as an abnormal link.
        for( PatternLink plink : run->elts )
        {
            // We have a Star type wildcard that can match multiple elements.
            ContainerInterface::iterator xit_star_end;               
            ContainerInterface::iterator xit_star_begin = xit;            
                        
            if( plink != run->elts.back() ) // Another star follows this one, forcing a decision
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
            query.RegisterAbnormalLink( plink, XLink::CreateDistinct(xss) ); // Only used in after-pass AND REPLACE!!
        }
    } 
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
    bool incomplete = false;
    const Plan::Collection &plan_col = plan.collections.at(pattern);

    // The only true unary constraint is that every child x link
    // is in some collection (because that can be read directly off the
    // nugget).
    for( PatternLink plink : plan_col.non_stars )  // independent of px
    {
        SolutionMap::const_iterator it = required_links->find(plink);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( !(nugget.cadence == TheKnowledge::Nugget::IN_COLLECTION) )
                throw WrongContainerCollectionMismatch(); // Be in a sequence
        }
    }

    // Require that every child x link is in the correct collection.
    for( PatternLink plink : plan_col.non_stars )  // depends on px
    {
        SolutionMap::const_iterator it = required_links->find(plink);
        if( it != required_links->end() ) 
        {
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(it->second) );        
            if( nugget.container != px )
                throw WrongContainerCollectionMismatch(); // Be in the right sequence
        }
    }

    // Require that every child x link is different (alldiff).
    SubCollectionRange::ExclusionSet x_so_far;
    for( PatternLink plink : plan_col.non_stars ) // independent of px
    {
        SolutionMap::const_iterator req_it = required_links->find(plink);
        if( req_it != required_links->end() ) 
        {
            XLink req_xlink = req_it->second; 
            if( x_so_far.count( req_xlink.GetXPtr() ) > 0 )
                throw CollisionCollectionMismatch(); // Already removed this one: collision
            x_so_far.insert( req_xlink.GetXPtr() );
        }
    }

    // Require that there are no leftover x of no star. Depends on px.
    if( !plan_col.star_plink )
    {
        if( px->size() > plan_col.non_stars.size() )
        {
            TRACE("mismatch - x left over\n");
            throw SurplusXCollectionMismatch();   // there were elements left over and no p_star to match them against
        }
    }
    
    for( PatternLink plink : plan_col.non_stars )  // independent of px
    {
        if( required_links->find(plink) == required_links->end() ) 
        {
            completeness = INCOMPLETE;
            return;
        }
    }
    
    if( plan_col.star_plink )
    {
        SubCollectionRange::ExclusionSet excluded_x;
        for( PatternLink plink : plan_col.non_stars ) // independent of px
            excluded_x.insert( required_links->find(plink)->second.GetXPtr() );

        // Now handle the p_star; all the non-star matches are excluded, leaving only the star matches.
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( base_xlink.GetChildX(), px->begin(), px->end() ) );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!       
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

    ASSERT( dest->IsFinal() )(*this)(" about to build non-final ")(*dest)("\n"); 

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
    // Identifiers that have multiple references in the pattern will be coupled, and  
    // after the first hit, BuildReplaceOverlay() will handle the rest and it uses Duplicate()
    shared_ptr<Cloner> dup_dest = Clone();
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    master_scr_engine->GetOverallMaster()->dirty_grass.insert( dest );

    ASSERT( dest->IsFinal() )(*this)(" about to build non-final ")(*dest)("\n"); 

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
            Agent *agent = AsAgent((TreePtr<Node>)*pattern_ptr);  
            TreePtr<Node> dest_child = agent->BuildReplace();
            *dest_ptr = dest_child;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    return dest;
}

