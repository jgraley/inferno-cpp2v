#include "../search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "delta_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/comparison_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/symbol_operators.hpp"

#define ITEMS_BY_PLAN

using namespace SR;
using namespace SYM;

void StandardAgent::SCRConfigure( const SCREngine *e,
                                  Phase phase )
{
    plan.ConstructPlan( this, phase );    
    AgentCommon::SCRConfigure(e, phase);
}

// ---------------------------- Planning ----------------------------------                                               

void StandardAgent::Plan::ConstructPlan( StandardAgent *algo_, Phase phase )
{
    algo = algo_;
    const vector< Itemiser::Element * > my_memb = algo->Itemise();
    int ii=0;
    ASSERT( sequences.empty() );
    ASSERT( collections.empty() );
    ASSERT( singulars.empty() );
    FOREACH( Itemiser::Element *ie, my_memb )
    {
        if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(ie) )        
            sequences.emplace_back( Sequence(ii, this, phase, pattern_seq) );
        else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(ie) )        
            collections.emplace_back( Collection(ii, this, phase, pattern_col) );
        else if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(ie) )        
        {
            if( *my_singular ) // only make plans for non-null singular pointers
                singulars.emplace_back( Singular(ii, this, phase, my_singular) );
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
		ASSERTS( pe );
        if( dynamic_cast<StarAgent *>(pe->get()) ) // per the impl, the star in a collection is not linked
        {
            ASSERTS( !p_star )("Only one star allowed in collections when in compare pattern");
            
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
    ASSERTS( pattern );
    plink = PatternLink(plan->algo, pattern);
}


string StandardAgent::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}

// ---------------------------- Pattern Query ----------------------------------                                               

shared_ptr<PatternQuery> StandardAgent::GetPatternQuery() const
{
    // Clear it just in case
    auto pattern_query = make_shared<PatternQuery>(this);

    // Note on the order of checks: We seem to have to register the collection's
    // link before the sequences' otherwise LoopRotation becaomes very slow.

    for( const Plan::Singular &plan_sing : plan.singulars )
        IncrPatternQuerySingular( plan_sing, pattern_query );

    for( const Plan::Collection &plan_col : plan.collections )
        IncrPatternQueryCollection( plan_col, pattern_query );
    
    for( const Plan::Sequence &plan_seq : plan.sequences )
        IncrPatternQuerySequence( plan_seq, pattern_query );
        
    return pattern_query;
}


void StandardAgent::IncrPatternQuerySequence( const Plan::Sequence &plan_seq, 
                                              shared_ptr<PatternQuery> &pattern_query ) const
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

                                              
void StandardAgent::IncrPatternQueryCollection( const Plan::Collection &plan_col, 
                                                shared_ptr<PatternQuery> &pattern_query ) const
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

                                                
void StandardAgent::IncrPatternQuerySingular( const Plan::Singular &plan_sing, 
                                              shared_ptr<PatternQuery> &pattern_query ) const
{
    pattern_query->RegisterNormalLink(PatternLink(this, plan_sing.pattern));
}                                              


// ---------------------------- Decided Queries ----------------------------------                                               

void StandardAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink keyer_xlink ) const
{
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    vector< Itemiser::Element * > x_memb = Itemise( keyer_xlink.GetChildX().get() );   
    
    for( const Plan::Singular &plan_sing : plan.singulars )
    {
        auto p_x_singular = dynamic_cast<TreePtrInterface *>(x_memb[plan_sing.itemise_index]);
        ASSERT( p_x_singular )( "itemise for x didn't match itemise for pattern");
        DecidedQuerySingular( query, keyer_xlink, p_x_singular, plan_sing );
    }

    for( const Plan::Collection &plan_col : plan.collections )
    {
        auto p_x_col = dynamic_cast<CollectionInterface *>(x_memb[plan_col.itemise_index]);
        ASSERT( p_x_col )( "itemise for x didn't match itemise for pattern");
        DecidedQueryCollection( query, keyer_xlink, p_x_col, plan_col );
    }

    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        auto p_x_seq = dynamic_cast<SequenceInterface *>(x_memb[plan_seq.itemise_index]);
        ASSERT( p_x_seq )( "itemise for x didn't match itemise for pattern");
        DecidedQuerySequence( query, keyer_xlink, p_x_seq, plan_seq );
    }
}


void StandardAgent::DecidedQuerySequence( DecidedQueryAgentInterface &query,
                                          XLink keyer_xlink,
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
            TreePtr<SubSequenceRange> xss( new SubSequenceRange( keyer_xlink.GetChildX(), xit, xit_star_end ) );

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
       
            query.RegisterNormalLink( PatternLink(this, &*pit), XLink(keyer_xlink.GetChildX(), &*xit) ); // Link into X
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
                                            XLink keyer_xlink,
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
        auto x_decision = make_shared< SubCollectionRange >( keyer_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() );
        x_decision->SetExclusions( excluded_x );                       
                   
        // An empty decision would imply we ran out of elements in p_x_col
        if( x_decision->empty() )
            throw InsufficientXCollectionMismatch();                 
                   
        // No need to provide the container x_decision; iterators will keep it alive and are
        // not invalidated by re-construction of the container (they're proxies for iterators on p_x_col).
        xit = query.RegisterDecision( x_decision->begin(), x_decision->end(), false, x_decision );    
        
        // We have our x element
        query.RegisterNormalLink( plink, XLink(keyer_xlink.GetChildX(), &*xit) ); // Link into X
        excluded_x.insert( &*xit );        
    }

    // Now handle the p_star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.
    TRACE("seen_star %d size of excluded_x %d\n", !!plan_col.star_plink, excluded_x.size() );
    
    if( plan_col.star_plink )
    {
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( keyer_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() ) );
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
                                          XLink keyer_xlink,
                                          TreePtrInterface *p_x_singular,
		                                  const Plan::Singular &plan_sing ) const
{
    PatternLink sing_plink(this, plan_sing.pattern);
    XLink sing_xlink(keyer_xlink.GetChildX(), p_x_singular);
    query.RegisterNormalLink(sing_plink, sing_xlink); // Link into X
}

// ---------------------------- Normal Linked Queries ----------------------------------                                               

bool StandardAgent::ImplHasNLQ() const
{    
    return true;
}


void StandardAgent::NormalLinkedQuerySequence( const Plan::Sequence &plan_seq,
                                               const SolutionMap *hypothesis_links,
                                               const TheKnowledge *knowledge ) const
{
    INDENT("S");
    ASSERT( planned );
    
    if( hypothesis_links->count(keyer_plink)==0 )
        return; // not attempting baseless queries

    // Get the members of x corresponding to pattern's class
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    
    // We require a co-iteimise because pattern may be a base of X (i.e. 
    // topological wild-carding). It may not be possible in general, but 
    // IS possible if pre-restriction is satisfied.
    if( !IsLocalMatch( keyer_xlink.GetChildX().get() ) )
        return; // Will not be able to itemise due incompatible type
    vector< Itemiser::Element * > keyer_itemised = Itemise( keyer_xlink.GetChildX().get() );   
    SequenceInterface *p_x_seq = dynamic_cast<SequenceInterface *>(keyer_itemised[plan_seq.itemise_index]);

    // Require that every child x link is in the correct container.
    // Note: checking p_x_seq only on non_star_at_front and non_star_at_back
    // is insufficient - they might both be stars.
    for( PatternLink plink : plan_seq.non_stars )  // depends on p_x_seq
    {        
        if( hypothesis_links->count(plink) > 0 ) 
        {
            // definitely a mismatch: there's a plink in the pattern for this
            // sequence so we need it to be non-empty
            if( p_x_seq->empty() )
                throw WrongContainerSequenceMismatch(); 
            XLink keyer_child_front( keyer_xlink.GetChildX(), &(p_x_seq->front()) );
            
            XLink req_xlink = hypothesis_links->at(plink);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );        
            if( !(nugget.my_container_front == keyer_child_front) )
                throw WrongContainerSequenceMismatch(); // Be in the right sequence        
        }
    }
    
    // If the pattern begins with a non-star, constrain the child x to be the 
    // front node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_front ) // independent of p_x_seq
    {
        if( hypothesis_links->count(plan_seq.non_star_at_front) > 0 ) 
        {        
            XLink req_xlink = hypothesis_links->at(plan_seq.non_star_at_front);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );         
            if( req_xlink != nugget.my_container_front )
                throw NotAtFrontMismatch();
        }
    }

    // If the pattern ends with a non-star, constrain the child x to be the 
    // back node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_back ) // independent of p_x_seq
    {
        if( hypothesis_links->count(plan_seq.non_star_at_back) > 0 ) 
        {        
            XLink req_xlink = hypothesis_links->at(plan_seq.non_star_at_back);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );           
            if( req_xlink != nugget.my_container_back )
                throw NotAtBackMismatch();
        }
    }

    // Adjacent pairs of non-stars in the pattern should correspond to adjacent
    // pairs of child x nodes. Only needs the two child x nodes, so binary constraint.
    for( pair<PatternLink, PatternLink> p : plan_seq.adjacent_non_stars ) // independent of p_x_seq
    {
        if( hypothesis_links->count(p.first) > 0 && hypothesis_links->count(p.second) > 0 )
        {
            XLink a_req_xlink = hypothesis_links->at(p.first);
            XLink b_req_xlink = hypothesis_links->at(p.second);
            const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_req_xlink) );        
            if( a_nugget.my_sequence_successor != b_req_xlink )
                 throw NotSuccessorSequenceMismatch();
        }
    }
    
    // Gapped pairs of non-stars in the pattern (i.e. stars in between) should 
    // correspond to pairs of child x nodes that are ordered correctly. Only needs 
    // the two child x nodes, so binary constraint.    
    for( pair<PatternLink, PatternLink> p : plan_seq.gapped_non_stars ) // independent of p_x_seq
    {
        if( hypothesis_links->count(p.first) > 0 && hypothesis_links->count(p.second) > 0 )
        {
            XLink a_req_xlink = hypothesis_links->at(p.first);
            XLink b_req_xlink = hypothesis_links->at(p.second);
            const TheKnowledge::Nugget &a_nugget( knowledge->GetNugget(a_req_xlink) );        
            const TheKnowledge::Nugget &b_nugget( knowledge->GetNugget(b_req_xlink) );        
            if( a_nugget.depth_first_index >= b_nugget.depth_first_index )
                throw NotAfterSequenceMismatch();
        }
    }
}


void StandardAgent::NormalLinkedQueryCollection( const Plan::Collection &plan_col,
                                                 const SolutionMap *hypothesis_links,
                                                 const TheKnowledge *knowledge ) const
{
    INDENT("C");
    bool incomplete = false;

    if( hypothesis_links->count(keyer_plink)==0 )
        return; // not attempting baseless queries

    // Get the members of x corresponding to pattern's class
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    
    // We require a co-iteimise because pattern may be a base of X (i.e. 
    // topological wild-carding). It may not be possible in general, but 
    // IS possible if pre-restriction is satisfied.
    if( !IsLocalMatch( keyer_xlink.GetChildX().get() ) )
        return; // Will not be able to itemise due incompatible type
    vector< Itemiser::Element * > keyer_itemised = Itemise( keyer_xlink.GetChildX().get() );   
    CollectionInterface *p_x_col = dynamic_cast<CollectionInterface *>(keyer_itemised[plan_col.itemise_index]);

    // Require that every child x link is in the correct collection.
    for( PatternLink plink : plan_col.non_stars )  // depends on p_x_col
    {
        if( hypothesis_links->count(plink) > 0 )
        {
            if( p_x_col->empty() )
                throw WrongContainerCollectionMismatch(); 
            XLink keyer_child_front( keyer_xlink.GetChildX(), &*(p_x_col->begin()) );
        
            XLink req_xlink = hypothesis_links->at(plink);
            const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_xlink) );  
            if( nugget.my_container_front != keyer_child_front )
                throw WrongContainerCollectionMismatch(); // Be in the right collection
        }
    }

    // Require that every child x link is different (alldiff).
    set<XLink> x_so_far;
    for( PatternLink plink : plan_col.non_stars ) // independent of p_x_col
    {
        if( hypothesis_links->count(plink) > 0 )
        {
            XLink req_xlink = hypothesis_links->at(plink);
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

// ---------------------------- Symbolic Queries ----------------------------------                                               
                                               
SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQueryPRed() const
{
	set< shared_ptr<BooleanExpression> > s;

    for( const Plan::Singular &plan_sing : plan.singulars )
        s.insert( SymbolicNormalLinkedQuerySingular( plan_sing ) );

    for( const Plan::Collection &plan_col : plan.collections )
        s.insert( SymbolicNormalLinkedQueryCollection( plan_col ) );

    for( const Plan::Sequence &plan_seq : plan.sequences )
        s.insert( SymbolicNormalLinkedQuerySequence( plan_seq ) );

    return MakeLazy<AndOperator>(s);    
}                                  

                                               
SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQuerySequence(const Plan::Sequence &plan_seq) const
{
    
    
    
    auto pattern_query = make_shared<PatternQuery>(this);
    IncrPatternQuerySequence( plan_seq, pattern_query );
    set<PatternLink> nlq_plinks = ToSetSolo( pattern_query->GetNormalLinks() );
    nlq_plinks.insert( keyer_plink );
    auto nlq_lambda = [this, plan_seq](const Expression::EvalKit &kit)
	{
        NormalLinkedQuerySequence( plan_seq, kit.hypothesis_links, kit.knowledge );
	};
	return MakeLazy<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQSequence()");	
}                                  


SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQueryCollection(const Plan::Collection &plan_col) const
{
    auto pattern_query = make_shared<PatternQuery>(this);
    IncrPatternQueryCollection( plan_col, pattern_query );
    set<PatternLink> nlq_plinks = ToSetSolo( pattern_query->GetNormalLinks() );
    nlq_plinks.insert( keyer_plink );
    
    auto nlq_lambda = [this, plan_col](const Expression::EvalKit &kit)
	{
        NormalLinkedQueryCollection( plan_col, kit.hypothesis_links, kit.knowledge );
	};
	return MakeLazy<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQCollection()");	
}                                  


SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQuerySingular(const Plan::Singular &plan_sing) const
{
    auto keyer = MakeLazy<SymbolVariable>(keyer_plink);
    auto sing = MakeLazy<ChildSingularOperator>( this, plan_sing.itemise_index, keyer );
    auto child = MakeLazy<SymbolVariable>(plan_sing.plink);
    return sing == child;
}                                  

// ---------------------------- Regeneration Queries ----------------------------------                                               
                                               
void StandardAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                              const SolutionMap *hypothesis_links,
                                              const TheKnowledge *knowledge ) const
{ 
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    vector< Itemiser::Element * > x_memb = Itemise( keyer_xlink.GetChildX().get() );   

    for( const Plan::Collection &plan_col : plan.collections )
    {
        auto p_x_col = dynamic_cast<CollectionInterface *>(x_memb[plan_col.itemise_index]);
        RegenerationQueryCollection( query, p_x_col, plan_col, hypothesis_links, knowledge );
    }
    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        auto p_x_seq = dynamic_cast<SequenceInterface *>(x_memb[plan_seq.itemise_index]);
        RegenerationQuerySequence( query, p_x_seq, plan_seq, hypothesis_links, knowledge );
    }
}


void StandardAgent::RegenerationQuerySequence( DecidedQueryAgentInterface &query,
                                               SequenceInterface *p_x_seq,
                                               const Plan::Sequence &plan_seq,
                                               const SolutionMap *hypothesis_links,
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
            if( hypothesis_links->count(run->predecessor) == 0 )         
                break; // can't do any more in the current run
            
            XLink pred_xlink = hypothesis_links->at(run->predecessor);
            const TheKnowledge::Nugget &pred_nugget( knowledge->GetNugget(pred_xlink) );                        
            xit = pred_nugget.my_container_it;
            ++xit; // get past the non-star
        }
        else
        {
            xit = p_x_seq->begin();
        }
        
        if( run->successor )
        {
            if( hypothesis_links->count(run->successor) == 0 )         
                break; // can't do any more in the current run
            
            XLink succ_xlink = hypothesis_links->at(run->successor);
            const TheKnowledge::Nugget &succ_nugget( knowledge->GetNugget(succ_xlink) );                        
            xit_star_limit = succ_nugget.my_container_it;
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
            XLink keyer_xlink = hypothesis_links->at(keyer_plink);
            TreePtr<SubSequenceRange> xss( new SubSequenceRange( keyer_xlink.GetChildX(), xit_star_begin, xit_star_end ) );

            // Apply couplings to this Star and matched range
            // Restrict to pre-restriction or pattern_seq restriction
            query.RegisterAbnormalLink( plink, XLink::CreateDistinct(xss) ); // Only used in after-pass AND REPLACE!!
        }
    } 
}


void StandardAgent::RegenerationQueryCollection( DecidedQueryAgentInterface &query,
                                                 CollectionInterface *p_x_col,
                                                 const Plan::Collection &plan_col,
                                                 const SolutionMap *hypothesis_links,
                                                 const TheKnowledge *knowledge ) const
{
    INDENT("C");

    if( plan_col.star_plink )
    {
        // Determine the set of non-star tree pointers 
        SubCollectionRange::ExclusionSet excluded_x;
        for( PatternLink plink : plan_col.non_stars ) // independent of p_x_col
            excluded_x.insert( hypothesis_links->at(plink).GetXPtr() );

        // Now handle the p_star; all the non-star matches are excluded, leaving only the star matches.
        XLink keyer_xlink = hypothesis_links->at(keyer_plink);
        TreePtr<SubCollectionRange> x_subcollection( new SubCollectionRange( keyer_xlink.GetChildX(), p_x_col->begin(), p_x_col->end() ) );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!       
    }    
}

// ---------------------------- Replace stuff ----------------------------------                                               

void StandardAgent::PlanOverlayImpl( PatternLink me_plink, 
                                     PatternLink under_plink )
{
    INDENT("T");
    ASSERT( under_plink.GetChildAgent() );
    TRACE(".PlanOverlayImpl(")(under_plink)(")\n");

    // Loop over all the elements of under and dest that do not appear in pattern or
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from under into dest.
    vector< Itemiser::Element * > my_memb = Itemise(); 
    vector< Itemiser::Element * > under_memb = Itemise( under_plink.GetChildAgent() ); 
    
    // Loop over all the members of under (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<my_memb.size(); i++ )
    {
        ASSERT( my_memb[i] )( "itemise returned null element" );
        ASSERT( under_memb[i] )( "itemise returned null element" );
        
        TRACE("Member %d\n", i );
        // Act only on singular members that are non-null in the pattern (i.e. this) 
        if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_memb[i]) )
        {
            TreePtrInterface *under_singular = dynamic_cast<TreePtrInterface *>(under_memb[i]);
            if( *my_singular )
            {
                ASSERT(*under_singular)("Cannot key ")(*my_singular)(" because correpsonding child of ")(*under_plink.GetChildAgent())(" is nullptr");
                PatternLink my_singular_plink(this, my_singular);
                PatternLink under_singular_plink(this, under_singular);
                
                my_singular_plink.GetChildAgent()->PlanOverlay( my_singular_plink, under_singular_plink );
            }
        }
    }
}


TreePtr<Node> StandardAgent::BuildReplaceImpl( PatternLink me_plink, 
                                               TreePtr<Node> key_node ) 
{
    INDENT("B");

    if( overlay_under_plink )
    {
        // Explicit request for overlay, resulting from use of the Delta agent.
        // The under pattern node is in a different location from over (=this), 
        // but overlay planning has set up overlay_under_plink for us.
        Agent *under_agent = overlay_under_plink.GetChildAgent();
        ASSERT( under_agent );
        TreePtr<Node> under_node = master_scr_engine->GetReplaceKey( overlay_under_plink );
        ASSERT( under_node );
        ASSERT( under_node->IsFinal() ); 
        ASSERT( IsLocalMatch(under_node.get()) );
        return BuildReplaceOverlay( me_plink, under_node );
    }
    else if( key_node ) 
    {
        // Overlay required due to coupling from compare to replace. 
        // The under and over pattern nodes are both this. AndRuleEngine 
        // has keyed this, and due wildcarding, key will be a final node
        // i.e. possibly a subclass of this node.
        ASSERT( key_node->IsFinal() ); 
        ASSERT( IsLocalMatch(key_node.get()) );
        return BuildReplaceOverlay( me_plink, key_node );
    }
    else
    {
        // Free replace pattern, just duplicate it.
        ASSERT( me_plink.GetPattern()->IsFinal() ); 
        return BuildReplaceNormal( me_plink ); 
    }
}


TreePtr<Node> StandardAgent::BuildReplaceOverlay( PatternLink me_plink, 
                                                  TreePtr<Node> under_node )  // overlaying
{
	INDENT("O");
    ASSERT( under_node );
    
    ASSERT( IsLocalMatch(under_node.get()) )
		  (" must be a non-strict superclass of under_node=")
		  (*under_node)
		  (", so that it does not have more members");
    TreePtr<Node> dest;
        
    // Make a new node, we will overlay from pattern, so resulting node will be dirty	
    // Duplicate the underneath node since it is at least as specialised (=non-strict subclass)
    dest = DuplicateNode( under_node, true );

    ASSERT( dest->IsFinal() )("About to build non-final ")(*dest)("\n"); 

    // Loop over the elements of pattern and dest, limited to elements
    // present in pattern, which is a non-strict subclass of under_node and dest. // Hmmm.... superclass?
    // Overlay or overwrite pattern over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > my_memb = Itemise();
    vector< Itemiser::Element * > dest_memb = Itemise( dest.get() ); // Get the members of dest corresponding to pattern's class
    ASSERT( my_memb.size() == dest_memb.size() );
    set< Itemiser::Element * > present_in_overlay; // Repeatability audit: OK since only checking for existance 
    
    TRACE("Copying %d members from pattern=", dest_memb.size())(*this)(" dest=")(*dest)("\n");
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("member %d from pattern\n", i );
        ASSERT( my_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        if( ContainerInterface *my_con = dynamic_cast<ContainerInterface *>(my_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            ASSERT( dest_con )( "itemise for dest didn't match itemise for my_con");
            dest_con->clear();

            TRACE("Copying container size %d from my_con\n", (*my_con).size() );
	        FOREACH( const TreePtrInterface &my_elt, *my_con )
	        {
		        ASSERT( my_elt )("Some element of member %d (", i)(*my_con)(") of ")(*this)(" was nullptr\n");
		        TRACE("Got ")(*my_elt)("\n");
                PatternLink my_elt_plink( this, &my_elt );
				TreePtr<Node> new_elt = my_elt_plink.GetChildAgent()->BuildReplace(my_elt_plink);
                ASSERT(new_elt); 
                if( ContainerInterface *new_sub_con = dynamic_cast<ContainerInterface *>(new_elt.get()) )
                {
                    TRACE("Walking SubContainer length %d\n", new_sub_con->size() );
                    FOREACH( const TreePtrInterface &new_sub_elt, *new_sub_con )
                        dest_con->insert( new_sub_elt );
                }
                else 
                {
                    ASSERT( new_elt->IsFinal() )("Got intermediate node ")(*new_elt);
                    TRACE("inserting ")(*new_elt)(" directly\n");
                    dest_con->insert( new_elt );
                }
	        }
	        present_in_overlay.insert( dest_memb[i] );
        }            
        else if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( dest_singular )( "itemise for target didn't match itemise for pattern");
                       
            if( *my_singular )
            {         
                PatternLink my_singular_plink( this, my_singular );                    
                TreePtr<Node> new_dest_singular = my_singular_plink.GetChildAgent()->BuildReplace(my_singular_plink);
                ASSERT( new_dest_singular );                
                ASSERT( new_dest_singular->IsFinal() );
                *dest_singular = new_dest_singular;
                present_in_overlay.insert( dest_memb[i] );
            }
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }        
    }
    
    // Loop over all the elements of under_node and dest that do not appear in pattern or
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from under_node into dest.
    vector< Itemiser::Element * > under_memb = under_node->Itemise();
    dest_memb = under_node->Itemise( dest.get() ); 

    TRACE("Copying %d members from under_node=", dest_memb.size())(*under_node)(" dest=")(*dest)("\n");
    // Loop over all the members of under_node (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        ASSERT( under_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( present_in_overlay.count(dest_memb[i]) > 0 )
            continue; // already did this one in the above loop

    	TRACE("Member %d from key\n", i );
        if( ContainerInterface *under_con = dynamic_cast<ContainerInterface *>(under_memb[i]) )                
        {
            // Note: we get here when a wildcard is coupled that does not have the container
            // because it is an intermediate node. Eg Scope as a wildcard matching Module does 
            // not have "bases".
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d from key\n", under_con->size() );
	        FOREACH( const TreePtrInterface &under_elt, *under_con )
	        {
		        ASSERT( under_elt ); // present simplified scheme disallows nullptr
		        TreePtr<Node> new_elt = DuplicateSubtree( (TreePtr<Node>)under_elt );
		        if( ContainerInterface *new_sub_con = dynamic_cast<ContainerInterface *>(new_elt.get()) )
		        {
			        TRACE("Walking SubContainer length %d\n", new_sub_con->size() );
		            FOREACH( const TreePtrInterface &new_sub_elt, *new_sub_con )
			            dest_con->insert( new_sub_elt );
           		}
		        else
		        {
                    ASSERT( new_elt->IsFinal() );
			        TRACE("inserting ")(*new_elt)(" directly\n");
			        dest_con->insert( new_elt );
		        }
	        }
        }            
        else if( TreePtrInterface *under_singular = dynamic_cast<TreePtrInterface *>(under_memb[i]) )
        {
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *under_singular );
            *dest_singular = DuplicateSubtree( (TreePtr<Node>)*under_singular );
            ASSERT( *dest_singular );
            ASSERT( (**dest_singular).IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    ASSERT( dest );
    return dest;
}

    
TreePtr<Node> StandardAgent::BuildReplaceNormal( PatternLink me_plink ) 
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
    vector< Itemiser::Element * > my_memb = Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Copying %d members pattern=", dest_memb.size())(*this)(" dest=")(*dest)("\n");
    // Loop over all the members of pattern (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Copying member %d\n", i );
        ASSERT( my_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *my_con = dynamic_cast<ContainerInterface *>(my_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d\n", my_con->size() );
	        FOREACH( const TreePtrInterface &my_elt, *my_con )
	        {
		        ASSERT( my_elt )("Some element of member %d (", i)(*my_con)(") of ")(*this)(" was nullptr\n");
		        TRACE("Got ")(*my_elt)("\n");
                PatternLink my_elt_plink( this, &my_elt );
				TreePtr<Node> new_elt = my_elt_plink.GetChildAgent()->BuildReplace(my_elt_plink);
		        if( ContainerInterface *new_sub_con = dynamic_cast<ContainerInterface *>(new_elt.get()) )
		        {
			        TRACE("Walking SubContainer length %d\n", new_sub_con->size() );
		            FOREACH( const TreePtrInterface &new_sub_elt, *new_sub_con )
			            dest_con->insert( new_sub_elt );  
           		}
		        else
		        {
			        TRACE("inserting ")(*new_elt)(" directly\n");
			        dest_con->insert( new_elt );
		        }
	        }
        }            
        else if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_memb[i]) )
        {
            TRACE("Copying single element\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *my_singular )("Member %d (", i)(*my_singular)(") of ")(*this)(" was nullptr when not overlaying\n");
            PatternLink my_singular_plink( this, my_singular );                    
            TreePtr<Node> new_dest_singular = my_singular_plink.GetChildAgent()->BuildReplace(my_singular_plink);
            *dest_singular = new_dest_singular;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    return dest;
}


Graphable::Block StandardAgent::GetGraphBlockInfo() const
{
	// Inject a non-trivial pre-restriction detector
	Block block = Node::GetGraphBlockInfo();

	// Overwrite link phase depending on the phase we're in
    for( Graphable::SubBlock &sub_block : block.sub_blocks ) 
        for( shared_ptr<Graphable::Link> link : sub_block.links )
            link->phase = phase;

	return block;
}
