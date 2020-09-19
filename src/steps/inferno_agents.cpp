#include "inferno_agents.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

using namespace CPPTree;

//---------------------------------- BuildIdentifier ------------------------------------    

void BuildIdentifierAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
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

//---------------------------------- IdentifierByName ------------------------------------    

void IdentifierByNameAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
	// the string that must be matched inside it.
	// TODO indicate whether it's matching instance, label or type identifier
	*bold = true;
	*shape = "trapezium";
	*text = name;
}


bool IdentifierByNameAgent::IsMatch( const TreePtrInterface &x ) const
{
    string newname = name; 
    TreePtr<Node> nx = x; // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( TreePtr<CPPTree::SpecificIdentifier> si = dynamic_pointer_cast<CPPTree::SpecificIdentifier>(nx) )
    {
        TRACE("IsMatch comparing ")(si->GetRender())(" with ")(newname);
        if( si->GetRender() == newname )
        {
            TRACE(" : same\n");
            return true;
        }
        TRACE(" : different\n");
    }
    return false;
}

//---------------------------------- Nested ------------------------------------    

shared_ptr<PatternQuery> NestedAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( &terminus );
	if( depth )
		pq->RegisterNormalLink( &depth ); 
    return pq;
}


void NestedAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                       TreePtr<Node> x ) const                          
{
    INDENT("N");
    query.Reset();
    
    string s;
    // Keep advancing until we get NULL, and remember the last non-null position
    int i = 0;
    TreePtr<Node> xt = x;
    while( TreePtr<Node> tt = Advance(xt, &s) )
    {
        xt = tt;
    } 
            
    // Compare the last position with the terminus pattern
    query.RegisterNormalLink( &terminus, xt ); // Link into X
    
    // Compare the depth with the supplied pattern if present
    if( depth )
    {
        TreePtr<Node> cur_depth( new SpecificString(s) );
        query.RegisterNormalLink( &depth, cur_depth );  // Generated Link (string)
    }
}    


TreePtr<Node> NestedArray::Advance( TreePtr<Node> x, 
                                    string *depth ) const
{
    if( auto a = dynamic_pointer_cast<Array>(x) )         
        return a->element; // TODO support depth string (or integer)
    else
        return nullptr;
}


TreePtr<Node> NestedSubscriptLookup::Advance( TreePtr<Node> x, 
                                              string *depth ) const
{
    if( auto s = dynamic_pointer_cast<Subscript>(x) )            
    {
        *depth += "S";
        return s->operands.front(); // the base, not the index
    }
    else if( auto l = dynamic_pointer_cast<Lookup>(x) )            
    {
        *depth += "L";
        return l->member; 
    }
    else
    {
        return nullptr;
    }
}

//---------------------------------- BuildContainerSize ------------------------------------    

TreePtr<Node> BuildContainerSize::BuildReplaceImpl( TreePtr<Node> keynode ) 
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
		keynode = MakePatternPtr<SpecificInteger>(size);
		SetKey( keynode );
	}
	// Note that the keynode could have been set via coupling - but still not
	// likely to do anything sensible, so explicitly check
	return DuplicateSubtree(keynode);   
}                                                   

//---------------------------------- IsLabelReached ------------------------------------    

void IsLabelReached::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                          TreePtr<Node> xx ) const
{
	INDENT("L");
	ASSERT( pattern );
    query.Reset();
	
	// TODO Flushable mechanism removed - flush every time for safety (if
	// this code ever gets used again). This may be slow!
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
	FlushCache();
	
	TreePtr<Node> n = AsAgent(pattern)->GetKey(); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
	if( !n )
		n = pattern;
	TreePtr<Expression> y = dynamic_pointer_cast<Expression>( n );
	ASSERT( y )("IsLabelReached saw pattern coupled to ")(*n)(" but an Expression is needed\n"); 
	ASSERT( xx );
	TreePtr<Node> nxx = xx;
	TreePtr<LabelIdentifier> x = dynamic_pointer_cast<LabelIdentifier>( nxx );
	ASSERT( x )("IsLabelReached at ")(*xx)(" but is of type LabelIdentifier\n"); 
	TRACE("Can label id ")(*x)(" reach expression ")(*y)("?\n");

	set< TreePtr<InstanceIdentifier> > uf;        
    if( !CanReachExpr(&uf, x, y) )
        throw Mismatch();  
	TRACE("I reakon ")(*x)(" reaches ")(*y)("\n"); 
}                 


bool IsLabelReached::CanReachExpr( set< TreePtr<InstanceIdentifier> > *f,
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
        ASSERTFAIL("IsLabelReached used on expression that is read from memory, cannot figure out the answer\n");
    }
		
	TRACE("I reakon ")(*x)(r?" does ":" does not ")("reach ")(*y)("\n"); 
	return r;        
}    


bool IsLabelReached::CanReachVar( set< TreePtr<InstanceIdentifier> > *f,
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

	FOREACH( TreePtr<Node> n, e )
	{
		if( TreePtr<Assign> a = dynamic_pointer_cast<Assign>(n) )
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

