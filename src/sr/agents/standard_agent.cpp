#include "../search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "overlay_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"
#include "coupling.hpp"

#define ITEMS_BY_PLAN

using namespace SR;

void StandardAgent::SCRConfigure( const SCREngine *e,
                                  Phase phase )
{
    plan.ConstructPlan( this, phase );    
    AgentCommon::SCRConfigure(e, phase);
}


void StandardAgent::Plan::ConstructPlan( StandardAgent *algo_, Phase phase )
{
    algo = algo_;
    const vector< Itemiser::Element * > pattern_memb = algo->Itemise();
    int ii=0;
    ASSERT( sequences.empty() );
    ASSERT( collections.empty() );
    ASSERT( singulars.empty() );
    FOREACH( Itemiser::Element *ie, pattern_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )        
            sequences.emplace_back( Sequence(ii, this, phase, pattern_seq) );
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )        
            collections.emplace_back( Collection(ii, this, phase, pattern_col) );
        else if( TreePtrInterface *pattern_sing = dynamic_cast<TreePtrInterface *>(ie) )        
        {
            if( *pattern_sing ) // only make plans for non-null singular pointers
                singulars.emplace_back( Singular(ii, this, phase, pattern_sing) );
        }
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        ii++;
    }
    algo->planned = true;
}


StandardAgent::Plan::Sequence::Sequence( int ii, Plan *plan, Phase phase, SequenceInterface *pattern_ ) :
    Item(ii),
    pattern(pattern_)
{
    if( phase == IN_REPLACE_ONLY )
        return;
    
    num_non_star = 0;
    SequenceInterface::iterator pit_prev;
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
        PatternLink plink(plan->algo, &*pit);
        if( dynamic_cast<StarAgent *>(plink.GetChildAgent()) )
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
        plink_prev = plink; 
    }
    run_lambda( PatternLink() );
    
    if( !pattern->empty() )
    {        
        plink_front = PatternLink(plan->algo, &pattern->front());
        if( !dynamic_cast<StarAgent *>(plink_front.GetChildAgent()) )
            non_star_at_front = plink_front;
            
        plink_back = PatternLink(plan->algo, &pattern->back());
        if( !dynamic_cast<StarAgent *>(plink_back.GetChildAgent()) )
            non_star_at_back = plink_back;        
    }
}


StandardAgent::Plan::Collection::Collection( int ii, Plan *plan, Phase phase, CollectionInterface *pattern_ ) :
    Item(ii),
    pattern(pattern_)
{
    if( phase == IN_REPLACE_ONLY )
        return;

    p_star = nullptr;
    for( CollectionInterface::iterator pit = pattern->begin(); 
         pit != pattern->end(); 
         ++pit )                 
    {
        const TreePtrInterface *pe = &*pit; 
        PatternLink plink(plan->algo, &*pit);
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


StandardAgent::Plan::Singular::Singular( int ii, Plan *plan, Phase phase, TreePtrInterface *pattern_ ) :
    Item(ii),
    pattern(pattern_)
{
    ASSERT( pattern );
    plink = PatternLink(plan->algo, pattern);
}


shared_ptr<PatternQuery> StandardAgent::GetPatternQuery() const
{
    // Clear it just in case
    auto pattern_query = make_shared<PatternQuery>(this);

    // Note on the order of checks: We seem to have to register the collection's
    // link before the sequences' otherwise LoopRotation becaomes very slow.

    for( const Plan::Singular &plan_sing : plan.singulars )
    {
        pattern_query->RegisterNormalLink(PatternLink(this, plan_sing.pattern));
    }

    for( const Plan::Collection &plan_col : plan.collections )
    {
        for( CollectionInterface::iterator pit = plan_col.pattern->begin(); pit != plan_col.pattern->end(); ++pit )                 
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
    
    const vector< Itemiser::Element * > pattern_memb = Itemise();
    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        for( SequenceInterface::iterator pit = plan_seq.pattern->begin(); pit != plan_seq.pattern->end(); ++pit )                 
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
        
    return pattern_query;
}


void StandardAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink base_xlink ) const
{
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    vector< Itemiser::Element * > x_memb = Itemise( base_xlink.GetChildX().get() );   
    
    for( const Plan::Singular &plan_sing : plan.singulars )
    {
        auto p_x_sing = dynamic_cast<TreePtrInterface *>(x_memb[plan_sing.itemise_index]);
        ASSERT( p_x_sing )( "itemise for x didn't match itemise for pattern");
        DecidedQuerySingular( query, base_xlink, p_x_sing, plan_sing );
    }

    for( const Plan::Collection &plan_col : plan.collections )
    {
        auto p_x_col = dynamic_cast<CollectionInterface *>(x_memb[plan_col.itemise_index]);
        ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
        DecidedQueryCollection( query, base_xlink, p_x_col, plan_col );
    }

    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        auto p_x_seq = dynamic_cast<SequenceInterface *>(x_memb[plan_seq.itemise_index]);
        ASSERT( p_x_seq )( "itemise for x didn't match itemise for pattern");
        DecidedQuerySequence( query, base_xlink, p_x_seq, plan_seq );
    }
}


void StandardAgent::DecidedQuerySequence( DecidedQueryAgentInterface &query,
                                          XLink base_xlink,
                                          SequenceInterface *p_x_seq,
		                                  const Plan::Sequence &plan_seq ) const
{
    INDENT("S");
    ASSERT( planned );

    if( p_x_seq->size() < plan_seq.num_non_star )
    {
        throw Mismatch();     
    }

    // Determine last star limit by working back from end of x
    ContainerInterface::iterator xit_star_limit = p_x_seq->end();            
    for( int i=0; i<plan_seq.num_non_star; i++ )
        --xit_star_limit;                
        
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or x runs out.
	ContainerInterface::iterator xit = p_x_seq->begin();
    ContainerInterface::iterator pit = plan_seq.pattern->begin();
	for( ; pit != plan_seq.pattern->end(); ++pit )
	{
 		ASSERT( xit == p_x_seq->end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
        if( dynamic_pointer_cast<StarAgent>(pe) )
        {
            // We have a Star type wildcard that can match multiple elements.
            ContainerInterface::iterator xit_star_end;
                
            // The last Star does not need a decision            
            if( pit == plan_seq.pit_last_star )
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
            // Restrict to pre-restriction or pattern_seq restriction
            query.RegisterAbnormalLink( PatternLink(this, &*pit), XLink::CreateDistinct(xss) ); // Only used in after-pass
             
            // Resume at the first element after the matched range
            xit = xit_star_end;
        }
 	    else // not a Star so match singly...
	    {
            if( xit == p_x_seq->end() )
                break;
       
            query.RegisterNormalLink( PatternLink(this, &*pit), XLink(base_xlink.GetChildX(), &*xit) ); // Link into X
            ++xit;
            
            // Every non-star pattern node we pass means there's one fewer remaining
            // and we can match a star one step further
            ASSERT(xit_star_limit != p_x_seq->end());
            ++xit_star_limit;
	    }
	}

    // If we finished the job and pattern and x are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==p_x_seq->end(), pit==plan_seq.pattern->end() );
    if( xit != p_x_seq->end() )
        throw Mismatch();  
    else if( pit != plan_seq.pattern->end() )
        throw Mismatch();  
}


void StandardAgent::DecidedQueryCollection( DecidedQueryAgentInterface &query,
                                            XLink base_xlink,
                                            CollectionInterface *p_x_col,
		 					                const Plan::Collection &plan_col ) const
{
    INDENT("C");

    SubCollectionRange::ExclusionSet excluded_x;
    
    for( PatternLink plink : plan_col.non_stars )
    {
    	TRACE( "Collection compare %d excluded out of %d; looking at ",
               excluded_x.size(),
               plan_col.pattern->size() )
             (plink)
             (" in plan_col.pattern\n" );        

        // We have to decide which node in the tree to match, so use the present conjecture
        // Make a SubCollectionRange excluding x elements we already matched
        ContainerInterface::iterator xit;
        auto x_decision = make_shared< SubCollectionRange >( base_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() );
        x_decision->SetExclusions( excluded_x );                       
                   
        // An empty decision would imply we ran out of elements in p_x_col
        if( x_decision->empty() )
            throw InsufficientXCollectionMismatch();                 
                   
        // No need to provide the container x_decision; iterators will keep it alive and are
        // not invalidated by re-construction of the container (they're proxies for iterators on p_x_col).
        xit = query.RegisterDecision( x_decision->begin(), x_decision->end(), false, x_decision );    
        
        // We have our x element
        query.RegisterNormalLink( plink, XLink(base_xlink.GetChildX(), &*xit) ); // Link into X
        excluded_x.insert( &*xit );        
    }

    // Now handle the p_star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.
    TRACE("seen_star %d size of excluded_x %d\n", !!plan_col.star_plink, excluded_x.size() );
    
    if( plan_col.star_plink )
    {
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( base_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() ) );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!
    }
    else if( excluded_x.size() != p_x_col->size() )
    {
        throw SurplusXCollectionMismatch();   // there were elements left over and no p_star to match them against
    }
    
    TRACE("matched\n");
}


void StandardAgent::DecidedQuerySingular( DecidedQueryAgentInterface &query,
                                          XLink base_xlink,
                                          TreePtrInterface *p_x_sing,
		                                  const Plan::Singular &plan_sing ) const
{
    PatternLink sing_plink(this, plan_sing.pattern);
    XLink sing_xlink(base_xlink.GetChildX(), p_x_sing);
    query.RegisterNormalLink(sing_plink, sing_xlink); // Link into X
}


bool StandardAgent::ImplHasNLQ() const
{    
    return true;
}

    
bool StandardAgent::NLQRequiresBase() const
{
    return false;
}                                         


void StandardAgent::RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                              const TheKnowledge *knowledge ) const
{ 
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    bool based = required_links->count(base_plink);
    vector< Itemiser::Element * > x_memb;
    if( based )
        x_memb = Itemise( required_links->at(base_plink).GetChildX().get() );   

    for( const Plan::Singular &plan_sing : plan.singulars )
    {
        TreePtrInterface *p_x_sing = based ? 
                                     dynamic_cast<TreePtrInterface *>(x_memb[plan_sing.itemise_index]) :
                                     nullptr;
        ASSERT( p_x_sing )( "itemise for x didn't match itemise for pattern");
        NormalLinkedQuerySingular( p_x_sing, plan_sing, required_links, knowledge );
    }
    for( const Plan::Collection &plan_col : plan.collections )
    {
        CollectionInterface *p_x_col = based ?
                                       dynamic_cast<CollectionInterface *>(x_memb[plan_col.itemise_index]) :
                                       nullptr;
        ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
        NormalLinkedQueryCollection( p_x_col, plan_col, required_links, knowledge );
    }
    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        SequenceInterface *p_x_seq = based ? 
                                     dynamic_cast<SequenceInterface *>(x_memb[plan_seq.itemise_index]) :
                                     nullptr;
        ASSERT( p_x_seq )( "itemise for x didn't match itemise for pattern");
        NormalLinkedQuerySequence( p_x_seq, plan_seq, required_links, knowledge );
    }
}


void StandardAgent::NormalLinkedQuerySequence( SequenceInterface *p_x_seq,
                                               const Plan::Sequence &plan_seq,
                                               const SolutionMap *required_links,
                                               const TheKnowledge *knowledge ) const
{
    INDENT("S");
    ASSERT( planned );
    
    // Every child x link is in some sequence (because that can be read 
    // directly off the nugget).
    for( PatternLink plink : plan_seq.non_stars )  // independent of p_x_seq
    {
        if( required_links->count(plink) > 0 ) 
        {
            XLink req_xlink = required_links->at(plink);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );        
            if( !(nugget.cadence == TheKnowledge::Nugget::IN_SEQUENCE) )
                throw WrongCadenceSequenceMismatch(); // Be in the right sequence        
        }
    }
    
    // Require that every child x link is in the correct container.
    // Note: checking p_x_seq only on non_star_at_front and non_star_at_back
    // is insufficient - they might both be stars.
    if( p_x_seq )
    {
        for( PatternLink plink : plan_seq.non_stars )  // depends on p_x_seq
        {        
            if( required_links->count(plink) > 0 ) 
            {
                XLink req_xlink = required_links->at(plink);
                const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );        
                if( !(nugget.container == p_x_seq) )
                    throw WrongContainerSequenceMismatch(); // Be in the right sequence        
            }
        }
    }
    
    // If the pattern begins with a non-star, constrain the child x to be the 
    // front node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_front ) // independent of p_x_seq
    {
        if( required_links->count(plan_seq.non_star_at_front) > 0 ) 
        {        
            XLink req_xlink = required_links->at(plan_seq.non_star_at_front);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );
            auto req_seq = dynamic_cast<SequenceInterface *>(nugget.container);
            ASSERT( req_seq )("Front element not in a sequence, cadence check should have ensured this");            
            XLink req_front_xlink(required_links->at(base_plink).GetChildX(), &req_seq->front());            
            if( req_xlink != req_front_xlink )
                throw NotAtFrontMismatch();
        }
    }

    // If the pattern finishes with a non-star, constrain the child x to be the 
    // back node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_back ) // independent of p_x_seq
    {
        if( required_links->count(plan_seq.non_star_at_back) > 0 ) 
        {        
            XLink req_xlink = required_links->at(plan_seq.non_star_at_back);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );
            auto req_seq = dynamic_cast<SequenceInterface *>(nugget.container);
            ASSERT( req_seq )("Back element not in a sequence, cadence check should have ensured this");            
            XLink req_back_xlink(required_links->at(base_plink).GetChildX(), &req_seq->back());            
            if( req_xlink != req_back_xlink )
                throw NotAtBackMismatch();
        }
    }

    // Adjacent pairs of non-stars in the pattern should correspond to adjacent
    // pairs of child x nodes. Only needs the two child x nodes, so binary constraint.
    for( pair<PatternLink, PatternLink> p : plan_seq.adjacent_non_stars ) // independent of p_x_seq
    {
        if( required_links->count(p.first) > 0 && required_links->count(p.second) > 0 )
        {
            XLink a_req_xlink = required_links->at(p.first);
            XLink b_req_xlink = required_links->at(p.second);
            const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_req_xlink) );        
            const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_req_xlink) );       
            ContainerInterface::iterator a_it_incremented = a_nugget.iterator;
            ++a_it_incremented;
            if( !(a_nugget.container == b_nugget.container && a_it_incremented == b_nugget.iterator) )
                 throw NotSuccessorSequenceMismatch();
        }
    }
    
    // Gapped pairs of non-stars in the pattern (i.e. stars in between) should 
    // correspond to pairs of child x nodes that are ordered correctly. Only needs 
    // the two child x nodes, so binary constraint.    
    for( pair<PatternLink, PatternLink> p : plan_seq.gapped_non_stars ) // independent of p_x_seq
    {
        if( required_links->count(p.first) > 0 && required_links->count(p.second) > 0 )
        {
            XLink a_req_xlink = required_links->at(p.first);
            XLink b_req_xlink = required_links->at(p.second);
            const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_req_xlink) );        
            const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_req_xlink) );        
            if( !(a_nugget.container == b_nugget.container && a_nugget.index < b_nugget.index) )
                throw NotAfterSequenceMismatch();
        }
    }
}


void StandardAgent::NormalLinkedQueryCollection( CollectionInterface *p_x_col,
                                                 const Plan::Collection &plan_col,
                                                 const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{
    INDENT("C");
    bool incomplete = false;

    // The only true unary constraint is that every child x link
    // is in some collection (because that can be read directly off the
    // nugget).
    for( PatternLink plink : plan_col.non_stars )  // independent of p_x_col
    {
        if( required_links->count(plink) > 0 )
        {
            XLink req_xlink = required_links->at(plink);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );        
            if( !(nugget.cadence == TheKnowledge::Nugget::IN_COLLECTION) )
                throw WrongCadenceCollectionMismatch(); // Be in a collection
        }
    }

    // Require that every child x link is in the correct collection.
    if( p_x_col )
    {
        for( PatternLink plink : plan_col.non_stars )  // depends on p_x_col
        {
            if( required_links->count(plink) > 0 )
            {
                XLink req_xlink = required_links->at(plink);
                const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );  
                if( nugget.container != p_x_col )
                    throw WrongContainerCollectionMismatch(); // Be in the right collection
            }
        }
    }

    // Require that every child x link is different (alldiff).
    set<XLink> x_so_far;
    for( PatternLink plink : plan_col.non_stars ) // independent of p_x_col
    {
        if( required_links->count(plink) > 0 )
        {
            XLink req_xlink = required_links->at(plink);
            if( x_so_far.count( req_xlink ) > 0 )
                throw CollisionCollectionMismatch(); // Already removed this one: collision
            x_so_far.insert( req_xlink );
        }
    }

    // Require that there are no leftover x, if no stars in pattern. Depends on p_x_col ONLY.
    if( !plan_col.star_plink && p_x_col )
    {
        if( p_x_col->size() > plan_col.non_stars.size() )
        {
            TRACE("mismatch - x left over\n");
            throw SurplusXCollectionMismatch();   // there were elements left over and no p_star to match them against
        }
    }
}


void StandardAgent::NormalLinkedQuerySingular( TreePtrInterface *p_x_sing,
                                               const Plan::Singular &plan_sing,
                                               const SolutionMap *required_links,
                                               const TheKnowledge *knowledge ) const
{
    if( p_x_sing )
    {
        XLink sing_xlink(required_links->at(base_plink).GetChildX(), p_x_sing);        
        if( required_links->count(plan_sing.plink) > 0 ) 
        {
            XLink req_sing_xlink = required_links->at(plan_sing.plink);                
            if( sing_xlink != req_sing_xlink )
                throw SingularMismatch();
        }
    }
}

                                               
void StandardAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                              const SolutionMap *required_links,
                                              const TheKnowledge *knowledge ) const
{ 
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    XLink base_xlink = required_links->at(base_plink);
    vector< Itemiser::Element * > x_memb = Itemise( base_xlink.GetChildX().get() );   

    for( const Plan::Collection &plan_col : plan.collections )
    {
        auto p_x_col = dynamic_cast<CollectionInterface *>(x_memb[plan_col.itemise_index]);
        RegenerationQueryCollection( query, p_x_col, plan_col, required_links, knowledge );
    }
    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        auto p_x_seq = dynamic_cast<SequenceInterface *>(x_memb[plan_seq.itemise_index]);
        RegenerationQuerySequence( query, p_x_seq, plan_seq, required_links, knowledge );
    }
}


void StandardAgent::RegenerationQuerySequence( DecidedQueryAgentInterface &query,
                                               SequenceInterface *p_x_seq,
                                               const Plan::Sequence &plan_seq,
                                               const SolutionMap *required_links,
                                               const TheKnowledge *knowledge ) const
{
    INDENT("S");
    ASSERT( planned );

    // If we got this far with an undersized p_x_seq, something has gone wrong 
    // in the logic, and the following code will crash into p_x_seq->end(). 
    // And I mean "crash" literally.
    ASSERT(p_x_seq->size() >= plan_seq.num_non_star); 

    // We now look at the runs of star patterns. Each is bounded by some combination
    // of the bounds of the x sequence and the surrounding non-star child x values. 
    for( shared_ptr<Plan::Sequence::Run> run : plan_seq.star_runs )
    {
      	ContainerInterface::iterator xit, xit_star_limit;

        if( run->predecessor )
        {
            if( required_links->count(run->predecessor) == 0 )         
                break; // can't do any more in the current run
            
            XLink pred_xlink = required_links->at(run->predecessor);
            const TheKnowledge::Nugget &pred_nugget( knowledge->GetNugget(pred_xlink) );                        
            xit = pred_nugget.iterator;
            ++xit; // get past the non-star
        }
        else
        {
            xit = p_x_seq->begin();
        }
        
        if( run->successor )
        {
            if( required_links->count(run->successor) == 0 )         
                break; // can't do any more in the current run
            
            XLink succ_xlink = required_links->at(run->successor);
            const TheKnowledge::Nugget &succ_nugget( knowledge->GetNugget(succ_xlink) );                        
            xit_star_limit = succ_nugget.iterator;
        }
        else
        {
            xit_star_limit = p_x_seq->end();
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
            XLink base_xlink = required_links->at(base_plink);
            TreePtr<SubSequenceRange> xss( new SubSequenceRange( base_xlink.GetChildX(), xit_star_begin, xit_star_end ) );

            // Apply couplings to this Star and matched range
            // Restrict to pre-restriction or pattern_seq restriction
            query.RegisterAbnormalLink( plink, XLink::CreateDistinct(xss) ); // Only used in after-pass AND REPLACE!!
        }
    } 
}


void StandardAgent::RegenerationQueryCollection( DecidedQueryAgentInterface &query,
                                                 CollectionInterface *p_x_col,
                                                 const Plan::Collection &plan_col,
                                                 const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{
    INDENT("C");

    if( plan_col.star_plink )
    {
        // Determine the set of non-star tree pointers 
        SubCollectionRange::ExclusionSet excluded_x;
        for( PatternLink plink : plan_col.non_stars ) // independent of p_x_col
            excluded_x.insert( required_links->at(plink).GetXPtr() );

        // Now handle the p_star; all the non-star matches are excluded, leaving only the star matches.
        XLink base_xlink = required_links->at(base_plink);
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( base_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() ) );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!       
    }    
}


void StandardAgent::KeyForOverlay( Agent *from )
{
    INDENT("T");
    ASSERT( from );
    CouplingKey from_key = from->GetKey();
    ASSERT( from_key );
    TRACE(*this)("::KeyForOverlay(")(from_key)(" from ")(*(from))(")\n");
    
    if( GetKey() )
        return; // Already keyed, no point wasting time keying this (and the subtree under it) again
        
    if( !IsLocalMatch(from) ) 
        return; // Not compatible with pattern: recursion stops here
        
    SetKey( from_key );
    
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
                AsAgent((TreePtr<Node>)*pattern_ptr)->KeyForOverlay( AsAgent((TreePtr<Node>)*keyer_ptr) );
            }
        }
    }
}


TreePtr<Node> StandardAgent::BuildReplaceImpl() 
{
    INDENT("B");
    if( GetKey() && IsLocalMatch(GetKey().GetKeyX().get()) ) 
        return BuildReplaceOverlay();
    else
        return BuildReplaceNormal(); // Overwriting pattern over dest, need to make a duplicate 
}


TreePtr<Node> StandardAgent::BuildReplaceOverlay()  // under substitution if not nullptr
{
	INDENT("O");
    TreePtr<Node> keynode = GetKey().GetKeyX();
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
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
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


Graphable::Block StandardAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                                   const NonTrivialPreRestrictionFunction &ntprf) const
{
	ASSERT( master_scr_engine )("Agent must before configured before graphing");

	// Inject a non-trivial pre-restriction detector
	Block block = Node::GetGraphBlockInfo( lnf, my_ntprf );

	// Overwrite link phase depending on the phase we're in
    for( Graphable::SubBlock &sub_block : block.sub_blocks ) 
        for( shared_ptr<Graphable::Link> link : sub_block.links )
            link->phase = phase;

	return block;
}


const StandardAgent::NonTrivialPreRestrictionFunction StandardAgent::my_ntprf = []( const TreePtrInterface *ppattern )
{
	return SpecialBase::IsNonTrivialPreRestriction( ppattern );
};		

