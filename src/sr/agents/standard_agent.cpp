#include "../search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "delta_agent.hpp"
#include "standard_agent.hpp"
#include "star_agent.hpp"
#include "scr_engine.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "up/zone_commands.hpp"

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
    const vector< Itemiser::Element * > my_items = algo->Itemise();
    int ii=0;
    ASSERT( sequences.empty() );
    ASSERT( collections.empty() );
    ASSERT( singulars.empty() );
    for( Itemiser::Element *ie : my_items )
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
    //if( phase == IN_REPLACE_ONLY )
    //    return;
    
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
    //if( phase == IN_REPLACE_ONLY )
    //    return;

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
            ASSERTS( phase == IN_REPLACE_ONLY || !p_star )("Only one star allowed in collections when in compare pattern");
            if( !p_star )
            {
				// TODO infrastructure for multiple stars as with sequences (for consistency)
				// and move the assert to regen query which is where it hurts (we have to associate with one star plink)
				// -> better for if we add support for category restriction on stars in collections
				p_star = pe;
				star_plink = plink;
			}
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
    auto pattern_query = make_shared<PatternQuery>();

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

// ---------------------------- Symbolic Queries ----------------------------------                                               
                                               
Lazy<BooleanExpression> StandardAgent::SymbolicNormalLinkedQueryPRed() const
{
	auto expr = MakeLazy<IsLocalMatchOperator>(GetPatternPtr().get(), MakeLazy<SymbolVariable>(keyer_plink));

    for( const Plan::Singular &plan_sing : plan.singulars )
        expr &= SymbolicNormalLinkedQuerySingular( plan_sing );

    for( const Plan::Collection &plan_col : plan.collections )
        expr &= SymbolicNormalLinkedQueryCollection( plan_col );

    for( const Plan::Sequence &plan_seq : plan.sequences )
        expr &= SymbolicNormalLinkedQuerySequence( plan_seq );

    return expr;    
}                                  

                                               
Lazy<BooleanExpression> StandardAgent::SymbolicNormalLinkedQuerySequence(const Plan::Sequence &plan_seq) const
{
	auto expr = MakeLazy<BooleanConstant>(true);

    // Require that every candidate x link is in the correct container. Binary 
    // constraint with keyer and candidate, for each candidate.
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    auto keyer_child_seq_front_expr = MakeLazy<ChildSequenceFrontOperator>(GetArchetypeNode(), plan_seq.itemise_index, keyer_expr);
    for( PatternLink candidate_plink : plan_seq.non_stars )
    {        
        auto candidate_expr = MakeLazy<SymbolVariable>(candidate_plink);
        auto candidate_seq_front_expr = MakeLazy<MyContainerFrontOperator>(candidate_expr);
        expr &= (candidate_seq_front_expr == keyer_child_seq_front_expr);
    }
    
    // If the pattern begins with a non-star, constrain the candidate x to be the 
    // front node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_front ) 
    {
        auto candidate_expr = MakeLazy<SymbolVariable>(plan_seq.non_star_at_front);
        auto candidate_seq_front_expr = MakeLazy<MyContainerFrontOperator>(candidate_expr);
        expr &= ( candidate_expr == candidate_seq_front_expr );
    }
 
    // If the pattern ends with a non-star, constrain the candidate x to be the 
    // back node in its own sequence. A unary constraint.
    if( plan_seq.non_star_at_back ) 
    {
        auto candidate_expr = MakeLazy<SymbolVariable>(plan_seq.non_star_at_back);
        auto candidate_seq_back_expr = MakeLazy<MyContainerBackOperator>(candidate_expr);
        expr &= ( candidate_expr == candidate_seq_back_expr );
    }
    
    // Adjacent pairs of non-stars in the pattern should correspond to adjacent
    // pairs of candidate x links. Only needs the two candidate x nodes, so binary constraint.
    for( pair<PatternLink, PatternLink> p : plan_seq.adjacent_non_stars )
    {
        auto candidate_a_expr = MakeLazy<SymbolVariable>(p.first);
        auto candidate_b_expr = MakeLazy<SymbolVariable>(p.second);
        auto candidate_a_successor_expr = MakeLazy<MySequenceSuccessorOperator>(candidate_a_expr);
        expr &= ( candidate_b_expr == candidate_a_successor_expr );
        // Avoid being pushed off the end since candidate b is not a shim
        expr &= ( candidate_b_expr != MakeLazy<SymbolConstant>(XLink::OffEndXLink) ); 
    }
        
    // Gapped pairs of non-stars in the pattern (i.e. stars in between) should 
    // correspond to pairs of candidate x nodes that are ordered correctly. Only needs 
    // the two candidate x nodes, so binary constraint.    
    for( pair<PatternLink, PatternLink> p : plan_seq.gapped_non_stars ) 
    {
        auto candidate_a_expr = MakeLazy<SymbolVariable>(p.first);
        auto candidate_b_expr = MakeLazy<SymbolVariable>(p.second);
        expr &= ( candidate_a_expr < candidate_b_expr );
    }

    return expr;        
}                                  


SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQueryCollection(const Plan::Collection &plan_col) const
{
    auto expr = MakeLazy<BooleanConstant>(true);

    // Require that every candidate x link is in the correct container. Binary 
    // constraint with keyer and candidate, for each candidate.
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    auto keyer_child_col_front_expr = MakeLazy<ChildCollectionFrontOperator>(GetArchetypeNode(), plan_col.itemise_index, keyer_expr);
    for( PatternLink candidate_plink : plan_col.non_stars )
    {        
        auto candidate_expr = MakeLazy<SymbolVariable>(candidate_plink);
        auto candidate_col_front_expr = MakeLazy<MyContainerFrontOperator>(candidate_expr);
        expr &= (candidate_col_front_expr == keyer_child_col_front_expr);
    }

    // Require that every child x link is different (alldiff). N-ary 
    // constraint on all candidates
    if( plan_col.non_stars.size() >= 2 )
    {
        list< shared_ptr<SymbolExpression> > candidate_exprs;
        for( PatternLink candidate_plink : plan_col.non_stars ) 
            candidate_exprs.push_back( MakeLazy<SymbolVariable>(candidate_plink) );
        expr &= MakeLazy<IsAllDiffOperator>( candidate_exprs );        
    }

    // Require that there are no leftover x, if no star in pattern. 
    // Unary constraint on keyer.
    if( !plan_col.star_plink )
    {
        auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
        expr &= MakeLazy<IsChildCollectionSizedOperator>(GetArchetypeNode(), plan_col.itemise_index, keyer_expr, plan_col.non_stars.size());
    }

    return expr;
}                                  


SYM::Lazy<SYM::BooleanExpression> StandardAgent::SymbolicNormalLinkedQuerySingular(const Plan::Singular &plan_sing) const
{
    auto keyer = MakeLazy<SymbolVariable>(keyer_plink);
    auto keyer_sing_expr = MakeLazy<SingularChildOperator>( GetArchetypeNode(), plan_sing.itemise_index, keyer );
    auto candidate_expr = MakeLazy<SymbolVariable>(plan_sing.plink);
    return keyer_sing_expr == candidate_expr;
}                                  

// ---------------------------- Regeneration Queries ----------------------------------                                               
                                               
void StandardAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                              const SolutionMap *hypothesis_links,
                                              const XTreeDatabase *x_tree_db ) const
{ 
    INDENT("Q");

    // Get the members of x corresponding to pattern's class
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    ASSERT( keyer_xlink != XLink::MMAX_Link );
    vector< Itemiser::Element * > x_items = Itemise( keyer_xlink.GetChildTreePtr().get() );   

    for( const Plan::Collection &plan_col : plan.collections )
    {
        auto p_x_col = dynamic_cast<CollectionInterface *>(x_items[plan_col.itemise_index]);
        RegenerationQueryCollection( query, p_x_col, plan_col, hypothesis_links, x_tree_db );
    }
    for( const Plan::Sequence &plan_seq : plan.sequences )
    {
        auto p_x_seq = dynamic_cast<SequenceInterface *>(x_items[plan_seq.itemise_index]);
        RegenerationQuerySequence( query, p_x_seq, plan_seq, hypothesis_links, x_tree_db );
    }
}


void StandardAgent::RegenerationQuerySequence( DecidedQueryAgentInterface &query,
                                               SequenceInterface *p_x_seq,
                                               const Plan::Sequence &plan_seq,
                                               const SolutionMap *hypothesis_links,
                                               const XTreeDatabase *x_tree_db ) const
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
            const LinkTable::Row &pred_row( x_tree_db->GetRow(pred_xlink) );                        
            ASSERT( pred_row.containment_context == DBWalk::IN_SEQUENCE );
            xit = pred_row.my_container_it;
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
            const LinkTable::Row &succ_row( x_tree_db->GetRow(succ_xlink) );  
            ASSERT( succ_row.containment_context == DBWalk::IN_SEQUENCE );
            xit_star_limit = succ_row.my_container_it;
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
            auto xss = MakeTreeNode<SubSequenceRange>( keyer_xlink.GetChildTreePtr(), xit_star_begin, xit_star_end );

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
                                                 const XTreeDatabase *x_tree_db ) const
{
    INDENT("C");

    if( plan_col.star_plink )
    {
        // Determine the set of non-star tree pointers 
        SubCollectionRange::ExclusionSet excluded_x;
        for( PatternLink plink : plan_col.non_stars ) // independent of p_x_col
            excluded_x.insert( hypothesis_links->at(plink).GetTreePtrInterface() );

        // Now handle the p_star; all the non-star matches are excluded, leaving only the star matches.
        XLink keyer_xlink = hypothesis_links->at(keyer_plink);
        auto x_subcollection = MakeTreeNode<SubCollectionRange>( keyer_xlink.GetChildTreePtr(), p_x_col->begin(), p_x_col->end() );
        x_subcollection->SetExclusions( excluded_x );                                                             
        query.RegisterAbnormalLink( plan_col.star_plink, XLink::CreateDistinct(x_subcollection) ); // Only used in after-pass AND REPLACE!!       
    }    
}

// ---------------------------- Replace ----------------------------------                                               

void StandardAgent::MaybeChildrenPlanOverlay( PatternLink me_plink, 
                                              PatternLink under_plink ) 
{
    INDENT("T");
    ASSERT( under_plink.GetChildAgent() );
    TRACE(".MaybeChildrenPlanOverlay(")(under_plink)(")\n");

    // Loop over all the elements of under and dest that do not appear in pattern or
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from under into dest.
    vector< Itemiser::Element * > my_items = Itemise(); 
    vector< Itemiser::Element * > under_items = Itemise( under_plink.GetChildAgent() ); 
    
    // Loop over all the members of under (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<my_items.size(); i++ )
    {
        ASSERT( my_items[i] )( "itemise returned null element" );
        ASSERT( under_items[i] )( "itemise returned null element" );
        
        TRACE("Member %d\n", i );
        // Act only on singular members that are non-null in the pattern (i.e. this) 
        if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_items[i]) )
        {
            TreePtrInterface *under_singular = dynamic_cast<TreePtrInterface *>(under_items[i]);
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


Agent::ReplaceExprPtr StandardAgent::GenReplaceExprImpl( const ReplaceKit &kit, 
                                                      PatternLink me_plink, 
                                                      XLink key_xlink ) 
{
    INDENT("B");

    if( overlay_under_plink )
    {
        // Explicit request for overlay, resulting from use of the Delta agent.
        // The under pattern node is in a different location from over (=this), 
        // but overlay planning has set up overlay_under_plink for us.
        XLink under_xlink = my_scr_engine->GetReplaceKey( overlay_under_plink );
        return GenFreeZoneExprOverlay( kit, me_plink, under_xlink );
    }
    else if( key_xlink ) 
    {
        // Overlay required due to coupling from compare to replace. 
        // The under and over pattern nodes are both this. AndRuleEngine 
        // has keyed this, and due wildcarding, key will be a final node
        // i.e. possibly a subclass of this node.
        return GenFreeZoneExprOverlay( kit, me_plink, key_xlink );
    }
    else
    {
        // Free replace pattern, just duplicate it.
        ASSERT( me_plink.GetPattern()->IsFinal() ); 
        return GenFreeZoneExprNormal( kit, me_plink ); 
    }
}


Agent::ReplaceExprPtr StandardAgent::GenFreeZoneExprOverlay( const ReplaceKit &kit, 
                                                             PatternLink me_plink, 
                                                             XLink under_xlink )  // overlaying
{
	INDENT("O");
    ASSERT( under_xlink );
    TreePtr<Node> under_node = under_xlink.GetChildTreePtr();
    
    ASSERT( under_node->IsFinal() )
		  (*under_node)
		  (" must be a final class.");
    ASSERT( IsSubcategory(*under_node) ) 
		  (*under_node)
		  (" must be a non-strict subclass of ")
		  (*this)
		  (", so that it has a super-set of members");
        
    // If I am the same type as under, duplicate me (and dest's local 
    // data members will come from me) otherwise duplicate under (and 
    // they will appear to come from under). #593 will improve on this.
    // Make a new node, we will overlay from pattern, so resulting node will be dirty.    
    // Use of DuplicateNode()/CloneNode() ensures correct behaviour with identifiers. 
    TreePtr<Node> dest;
    if( under_node->IsSubcategory(*GetPatternPtr()) ) 
        dest = AgentCommon::CloneNode();
    else
        dest = Duplicate::DuplicateNode(under_node);

	// We "invent" dest, because of information coming from this pattern node.
	dest->SetInventedHere();

    // Stuff for creating commands
    list<Agent::ReplaceExprPtr> child_commands;    
    FreeZone zone = FreeZone::CreateSubtree(dest);

    // Loop over all the elements of under_node and dest that do not appear in pattern or
    // appear in pattern but are nullptr TreePtr<>s. Duplicate from under_node into dest.
    vector< Itemiser::Element * > under_items = under_node->Itemise();
    vector< Itemiser::Element * > dest_items = under_node->Itemise( dest.get() ); 
    ASSERT( under_items.size() == dest_items.size() );

    // Loop over the child elements of me (=over) and dest, limited to elements
    // present in me, which is a non-strict superclass of under_node and dest.
    // Overlay or overwrite pattern over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > my_items = Itemise();
    vector< Itemiser::Element * > dest_items_in_me = Itemise( dest.get() ); // Get the members of dest corresponding to pattern's class
    ASSERT( my_items.size() == dest_items_in_me.size() );        
    
    TRACE("Copying %d members from under_node=", dest_items.size())(*under_node)(" dest=")(*dest)("\n");
    // i tracks items in under/dest, j tracks items in me
	bool in_me;
    for( int i=0, j=0; i<dest_items.size(); i++, in_me && j++ )
    {
        ASSERT( under_items[i] )( "itemise returned null element" );
        ASSERT( dest_items[i] )( "itemise returned null element" );

        in_me = j < dest_items_in_me.size() && dest_items[i]==dest_items_in_me[j];
        
        bool should_overlay;
        if( in_me )
        {
			should_overlay = true; // in me...
			if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_items[j]) )
				if( !*my_singular )
					should_overlay = false; // but is a NULL singular
		}
		else
        {
			should_overlay = false; // not in me (I'm a super-category)
		}
                    
		if( should_overlay )
		{
		    ASSERT( my_items[j] )( "itemise returned null element" );
			ASSERT( dest_items_in_me[j] )( "itemise returned null element" );
		}
		
    	TRACE("Member %d from key\n", i );
        if( ContainerInterface *under_container = dynamic_cast<ContainerInterface *>(under_items[i]) )                
        {
    		ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_items[i]);
			ASSERT( dest_con )( "itemise for dest didn't match itemise for my_con");
    		dest_con->clear();
    		
			if( should_overlay )
			{	
				ContainerInterface *my_con = dynamic_cast<ContainerInterface *>(my_items[j]);
				ASSERT(my_con);
				
				TRACE("Copying container size %d from my_con\n", (*my_con).size() );
				for( const TreePtrInterface &my_elt : *my_con )
				{
					// Make a placeholder in the dest container for the updater to point to
					ContainerInterface::iterator dest_it = dest_con->insert( ContainerTerminus::MakePlaceholder() );
					zone.AddTerminus( make_shared<ContainerTerminus>(dest_con, dest_it) );     
					
					ASSERT( my_elt )("Some element of member %d (", j)(*my_con)(") of ")(*this)(" was nullptr\n");
					TRACE("Got ")(*my_elt)("\n");
					PatternLink my_elt_plink( this, &my_elt );
					child_commands.push_back( my_elt_plink.GetChildAgent()->GenReplaceExpr(kit, my_elt_plink) );				
				}
			}	
			else
			{
				TRACE("Copying container size %d from key\n", under_container->size() );
				for( const TreePtrInterface &under_elt : *under_container )
				{
					// Make a placeholder in the dest container for the updater to point to
					ContainerInterface::iterator dest_it = dest_con->insert( ContainerTerminus::MakePlaceholder() );
					zone.AddTerminus( make_shared<ContainerTerminus>(dest_con, dest_it) );     

					ASSERT( under_elt ); // present simplified scheme disallows nullptr
					TreeZone under_zone = TreeZone::CreateSubtree(XLink(under_node, &under_elt) );
					child_commands.push_back( make_shared<DupMergeTreeZoneOperator>(move(under_zone)) );		
				}
			}
        }            
        else if( TreePtrInterface *under_singular = dynamic_cast<TreePtrInterface *>(under_items[i]) )
        {
			TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
			ASSERT( dest_singular )( "itemise for target didn't match itemise for pattern");
			zone.AddTerminus( make_shared<SingularTerminus>(dest_singular) );            

			if( should_overlay )
			{
				TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_items[j]);		
				ASSERT(	my_singular );
				ASSERT( *my_singular ); // Should not have marked this one for overlay if NULL
				PatternLink my_singular_plink( this, my_singular );                    
				child_commands.push_back( my_singular_plink.GetChildAgent()->GenReplaceExpr(kit, my_singular_plink) );           
			}		
			else
			{
				ASSERT( *under_singular );            
				TreeZone under_zone = TreeZone::CreateSubtree(XLink(under_node, under_singular) );
				child_commands.push_back( make_shared<DupMergeTreeZoneOperator>(move(under_zone)) );			
			}
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return make_shared<MergeFreeZoneOperator>( move(zone), move(child_commands) );         
}

Agent::ReplaceExprPtr StandardAgent::GenFreeZoneExprNormal( const ReplaceKit &kit, 
                                                            PatternLink me_plink ) 
{
	INDENT("N");
 
    ASSERT( IsFinal() )("Trying to build non-final ")(*this); 

	// Make a new node, force dirty because from pattern
    // Use clone here because we never want to place an Agent object in the output program tree.
    // Identifiers that have multiple references in the pattern will be coupled, and  
    // after the first hit, GenerateCommandOverlay() will handle the rest and it uses Duplicate()
    TreePtr<Node> dest = AgentCommon::CloneNode();
    ASSERT( dest->IsFinal() )(*this)(" trying to build non-final ")(*dest)("\n"); 

	// We "invent" dest, because of information coming from this pattern node.
	dest->SetInventedHere();

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. dest must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > my_items = Itemise();
    vector< Itemiser::Element * > dest_items = dest->Itemise(); 

    // Stuff for creating commands
    list<Agent::ReplaceExprPtr> child_commands;
    FreeZone zone = FreeZone::CreateSubtree(dest);

    TRACE("Copying %d members pattern=", dest_items.size())(*this)(" dest=")(*dest)("\n");
    // Loop over all the members of pattern (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_items.size(); i++ )
    {
    	TRACE("Copying member %d\n", i );
        ASSERT( my_items[i] )( "itemise returned null element" );
        ASSERT( dest_items[i] )( "itemise returned null element" );
        
        if( ContainerInterface *my_con = dynamic_cast<ContainerInterface *>(my_items[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_items[i]);
            dest_con->clear();
            
            TRACE("Copying container size %d\n", my_con->size() );
	        for( const TreePtrInterface &my_elt : *my_con )
	        {
		        ASSERT( my_elt )("Some element of member %d (", i)(*my_con)(") of ")(*this)(" was nullptr\n");
		        TRACE("Got ")(*my_elt)("\n");
		        
                // Make a placeholder in the dest container for the updater to point to
                ContainerInterface::iterator dest_it = dest_con->insert( ContainerTerminus::MakePlaceholder() );
                zone.AddTerminus( make_shared<ContainerTerminus>(dest_con, dest_it) );    

                PatternLink my_elt_plink( this, &my_elt );
				child_commands.push_back( my_elt_plink.GetChildAgent()->GenReplaceExpr(kit, my_elt_plink) );               
            }
        }            
        else if( TreePtrInterface *my_singular = dynamic_cast<TreePtrInterface *>(my_items[i]) )
        {
            TRACE("Copying single element\n");
            ASSERT( *my_singular )("Member %d (", i)(*my_singular)(") of ")(*this)(" was nullptr when not overlaying\n");            
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
            zone.AddTerminus( make_shared<SingularTerminus>(dest_singular) );            

            PatternLink my_singular_plink( this, my_singular );                    
			child_commands.push_back( my_singular_plink.GetChildAgent()->GenReplaceExpr(kit, my_singular_plink) );           
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }       
    }
    
    return make_shared<MergeFreeZoneOperator>( move(zone), move(child_commands) );     
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
