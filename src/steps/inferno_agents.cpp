#include "inferno_agents.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "sr/agents/agent.hpp"
#include "sr/agents/standard_agent.hpp"
#include "sr/scr_engine.hpp"
#include "sr/search_replace.hpp"
#include "sr/link.hpp"

using namespace CPPTree;

//---------------------------------- BuildIdentifier ------------------------------------    

shared_ptr<PatternQuery> BuildIdentifierAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void BuildIdentifierAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                                XLink x ) const             
{ 
    query.Reset();
}   


void BuildIdentifierAgent::GetGraphNodeAppearance( bool *bold, string *text, string *shape ) const
{
	// The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
	// the printf format string that controls the name of the generated identifier inside it.
	// TODO indicate whether it's building instance, label or type identifier
	*bold = true;
	*shape = "parallelogram";
	*text = format;
}


string BuildIdentifierAgent::GetNewName()
{
    INDENT("B");
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    vector<string> vs;
    bool all_same = true;
    FOREACH( TreePtr<Node> source, sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        TreePtr<Node> n = AsAgent(source)->BuildReplace();
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( n );
        TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
        ASSERT( si )("BuildIdentifier: ")(*n)(" should be a kind of SpecificIdentifier (format is %s)", format.c_str());
        string s = si->GetRender();
        if( !vs.empty() )
            all_same = all_same && (s == vs.back());
        vs.push_back( s );
    }

    // Optional functionality: when every identifier has the same name, just return that
    // name. Handy for "merging" operations.
    if( (flags & BYPASS_WHEN_IDENTICAL) && all_same )
        return vs[0];  

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( vs.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), vs[0].c_str() );
        case 2:
            return SSPrintf( format.c_str(), vs[0].c_str(), vs[1].c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}

//---------------------------------- BuildInstanceIdentifierAgent ------------------------------------    

TreePtr<Node> BuildInstanceIdentifierAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keylink.GetChildX());   
}                                                   

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<Node> BuildTypeIdentifierAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificTypeIdentifier>( new CPPTree::SpecificTypeIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keylink.GetChildX());   
}                                                   

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<Node> BuildLabelIdentifierAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keylink.GetChildX());   
}                                                   

//---------------------------------- IdentifierByName ------------------------------------    

void IdentifierByNameAgent::GetGraphNodeAppearance( bool *bold, string *text, string *shape ) const
{
	// The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
	// the string that must be matched inside it.
	// TODO indicate whether it's matching instance, label or type identifier
	*bold = true;
	*shape = "trapezium";
	*text = name;
}


shared_ptr<PatternQuery> IdentifierByNameAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void IdentifierByNameAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                                 XLink x ) const                
{
    query.Reset();
    string newname = name; 
    TreePtr<Node> nx = x.GetChildX(); // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( TreePtr<CPPTree::SpecificIdentifier> si = dynamic_pointer_cast<CPPTree::SpecificIdentifier>(nx) )
    {
        TRACE("Comparing ")(si->GetRender())(" with ")(newname);
        if( si->GetRender() == newname )
        {
            TRACE(" : same\n");
            return;
        }
        TRACE(" : different\n");
    }
    throw Mismatch();  
}                                


//---------------------------------- Nested ------------------------------------    

shared_ptr<PatternQuery> NestedAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, &terminus) );
	if( depth )
		pq->RegisterNormalLink( PatternLink(this, &depth) ); // local
    return pq;
}


void NestedAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                       XLink x ) const                          
{
    INDENT("N");
    query.Reset();
    
    string s;
    // Keep advancing until we get nullptr, and remember the last non-null position
    int i = 0;
    XLink xt = x;
    while( XLink tt = Advance(xt, &s) )
    {
        xt = tt;
    } 
            
    // Compare the last position with the terminus pattern
    query.RegisterNormalLink( PatternLink(this, &terminus), xt ); // Link into X
    
    // Compare the depth with the supplied pattern if present
    if( depth )
    {
        auto op = [&](XLink x) -> XLink
        {
            TreePtr<Node> cur_depth( new SpecificString(s) );
            XLink new_xlink = XLink::CreateDistinct(cur_depth); // cache will un-distinct
            return master_scr_engine->UniquifyDomainExtension(new_xlink);
        };
        // note: not caching the recursive algorithm because we
        // need terminus from it. See #153 for discussion
        query.RegisterNormalLink( PatternLink(this, &depth), cache( x, op ) );  // Generated Link (string)
    }
}    


XLink NestedArrayAgent::Advance( XLink x, 
                                 string *depth ) const
{
    if( auto a = dynamic_pointer_cast<Array>(x.GetChildX()) )         
        return XLink(a, &(a->element)); // TODO support depth string (or integer)
    else
        return XLink();
}


XLink NestedSubscriptLookupAgent::Advance( XLink x, 
                                           string *depth ) const
{
    if( auto s = dynamic_pointer_cast<Subscript>(x.GetChildX()) )            
    {
        *depth += "S";
        return XLink(s, &(s->operands.front())); // the base, not the index
    }
    else if( auto l = dynamic_pointer_cast<Lookup>(x.GetChildX()) )            
    {
        *depth += "L";
        return XLink(l, &(l->member)); 
    }
    else
    {
        return XLink();
    }
}

//---------------------------------- BuildContainerSizeAgent ------------------------------------    

shared_ptr<PatternQuery> BuildContainerSizeAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void BuildContainerSizeAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                                   XLink x ) const
{ 
    query.Reset(); 
}   


TreePtr<Node> BuildContainerSizeAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
	INDENT("%");
	if( !GetKey() )
	{
		ASSERT( container );
		TreePtr<Node> n = AsAgent( container )->BuildReplace();
		ASSERT( n );
		ContainerInterface *n_container = dynamic_cast<ContainerInterface *>(n.get());
		ASSERT( n_container );
		int size = n_container->size();
        keylink = XLink::CreateDistinct( MakePatternPtr<SpecificInteger>(size) );
		SetKey( keylink );
	}
	// Note that the keylink could have been set via coupling - but still not
	// likely to do anything sensible, so explicitly check
	return DuplicateSubtree(keylink.GetChildX());   
}                                                   

//---------------------------------- IsLabelReachedAgent ------------------------------------    

void IsLabelReachedAgent::FlushCache() const 
{
    ASSERT(0);
    cache.clear();
}


shared_ptr<PatternQuery> IsLabelReachedAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}
    
    
void IsLabelReachedAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                               XLink xx ) const
{
	INDENT("L");
	ASSERT( pattern );
    query.Reset();
	
	// TODO Flushable mechanism removed - flush every time for safety (if
	// this code ever gets used again). This may be slow!
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
	FlushCache();
	
	TreePtr<Node> n = AsAgent(pattern)->GetKey().GetChildX(); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
	if( !n )
		n = pattern;
	TreePtr<Expression> y = dynamic_pointer_cast<Expression>( n );
	ASSERT( y )("IsLabelReachedAgent saw pattern coupled to ")(n)(" but an Expression is needed\n"); 
	ASSERT( xx );
	TreePtr<Node> nxx = xx.GetChildX();
	TreePtr<LabelIdentifier> x = dynamic_pointer_cast<LabelIdentifier>( nxx );
	ASSERT( x )("IsLabelReachedAgent at ")(xx)(" but is of type LabelIdentifier\n"); 
	TRACE("Can label id ")(x)(" reach expression ")(y)("?\n");

	set< TreePtr<InstanceIdentifier> > uf;        
    if( !CanReachExpr(&uf, x, y) )
        throw Mismatch();  
	TRACE("I reakon ")(x)(" reaches ")(y)("\n"); 
}                 


bool IsLabelReachedAgent::CanReachExpr( set< TreePtr<InstanceIdentifier> > *f,
								   TreePtr<LabelIdentifier> x, 
					               TreePtr<Expression> y ) const // y is expression. Can it yield label x?
{
	INDENT("X");
	bool r = false;
	if( TreePtr<LabelIdentifier> liy = dynamic_pointer_cast<LabelIdentifier>(y) )
    {
		r = liy->IsLocalMatch( x.get() ); // y is x, so yes
    }
	else if( TreePtr<InstanceIdentifier> iiy = dynamic_pointer_cast<InstanceIdentifier>( y ) )
    {
		r = CanReachVar(f, x, iiy );
    }
	else if( TreePtr<Ternop> ty = dynamic_pointer_cast<Ternop>( y ) )
    {
        Sequence<Expression>::iterator ops_it = ty->operands.begin();
        ++ops_it; // only the choices, not the condition
		r = CanReachExpr(f, x, *ops_it);
        ++ops_it;
		r = r || CanReachExpr(f, x, *ops_it); 
    }
	else if( TreePtr<Comma> cy = dynamic_pointer_cast<Comma>( y ) )
    {
		r = CanReachExpr(f, x, cy->operands.back()); // second operand
    }
	else if( TreePtr<Subscript> sy = dynamic_pointer_cast<Subscript>( y ) ) // subscript as r-value
	{
        r = CanReachExpr(f, x, sy->operands.front()); // first operand
    }
	else if( dynamic_pointer_cast<Dereference>( y ) )
    {
        ASSERTFAIL("IsLabelReachedAgent used on expression that is read from memory, cannot figure out the answer\n");
    }
		
	TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
	return r;        
}    


bool IsLabelReachedAgent::CanReachVar( set< TreePtr<InstanceIdentifier> > *f,
                                       TreePtr<LabelIdentifier> x, 
				                       TreePtr<InstanceIdentifier> y ) const // y is instance identifier. Can expression x be assigned to it?
{
	INDENT(" ");
	Reaching rr( x, y );
	if( cache.count(rr) > 0 )
	{
		TRACE("cache hit yeah yeah\n");
		return cache[rr];
	}
	bool r = false;        
	Walk e( master_scr_engine->GetOverallMaster()->GetContext() ); 
	
	if( f->count(y) > 0 )
		return false; // already processing this identifier, so we have a loop
					  // so don't recurse further
					  
	f->insert(y);                           

	TRACE("Looking for assignment like ")(*y)(" = ")(*x)("\n");

	FOREACH( const TreePtrInterface &n, e )
	{
		if( TreePtr<Assign> a = dynamic_pointer_cast<Assign>((TreePtr<Node>)n) )
		{
			TreePtr<Expression> lhs = a->operands.front();
			if( TreePtr<Subscript> slhs = dynamic_pointer_cast<Subscript>( lhs ) ) // subscript as l-value 
				lhs = slhs->operands.front();
			TRACE("Examining assignment: ")(*lhs)(" = ")(*a->operands.back())("\n"); 
			if( lhs == y )
			{
				if( CanReachExpr( f, x, a->operands.back() ) )
				{
					r = true;                        
					break; // early out, since we have the info we need
				}
			}
		}
	}
	
	f->erase(y);
	return r;
}

