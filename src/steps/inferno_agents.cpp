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

PatternQueryResult NestedAgent::PatternQuery() const
{
    PatternQueryResult r;
	r.AddLink( false, AsAgent(terminus) );
	if( depth )
		r.AddLink( false, AsAgent(depth) );
    return r;
}


void NestedAgent::DecidedQuery( QueryAgentInterface &query,
                                const TreePtrInterface *px ) const                          
{
    INDENT("N");
    query.Reset();
    
    string s;
    // Keep advancing until we get NULL, and remember the last non-null position
    TreePtr<Node> xt = *px;
    int i = 0;
    while( TreePtr<Node> tt = Advance(xt, &s) )
    {
        xt = tt;
    } 
            
    // Compare the last position with the terminus pattern
    // TODO I don't think a local link should be needed here?
    query.AddLocalLink( false, AsAgent(terminus), xt );
    
    // Compare the depth with the supplied pattern if present
    if( depth )
    {
        TreePtr<Node> cur_depth( new SpecificString(s) );
        query.AddLocalLink( false, AsAgent(depth), cur_depth );
    }
}    


TreePtr<Node> NestedArray::Advance( TreePtr<Node> n, string *depth ) const
{
    if( TreePtr<Array> a = dynamic_pointer_cast<Array>(n) )         
        return a->element; // TODO support depth string (or integer)
    else
        return TreePtr<Node>();
}


TreePtr<Node> NestedSubscriptLookup::Advance( TreePtr<Node> n, string *depth ) const
{
    if( TreePtr<Subscript> s = dynamic_pointer_cast<Subscript>(n) )            
    {
        *depth += "S";
        return s->operands.front(); // the base, not the index
    }
    else if( TreePtr<Lookup> l  = dynamic_pointer_cast<Lookup>(n) )            
    {
        *depth += "L";
        return l->member; 
    }
    else
    {
        return TreePtr<Node>();
    }
}

//---------------------------------- BuildContainerSize ------------------------------------    

TreePtr<Node> BuildContainerSize::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
	INDENT("%");
	if( !GetCoupled() )
	{
		ASSERT( container );
		TreePtr<Node> n = AsAgent( container )->BuildReplace();
		ASSERT( n );
		ContainerInterface *n_container = dynamic_cast<ContainerInterface *>(n.get());
		ASSERT( n_container );
		int size = n_container->size();
		keynode = MakePatternPtr<SpecificInteger>(size);
		DoKey( keynode );
	}
	// Note that the keynode could have been set via coupling - but still not
	// likely to do anything sensible, so explicitly check
	return DuplicateSubtree(keynode);   
}                                                   

//---------------------------------- IsLabelReached ------------------------------------    

void IsLabelReached::DecidedQuery( QueryAgentInterface &query,
                                   const TreePtrInterface *pxx ) const
{
	INDENT("L");
	ASSERT( pattern );
    ASSERT(pxx);
    query.Reset();
	
	// TODO Flushable mechanism removed - flush every time for safety (if
	// this code ever gets used again). This may be slow!
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
	FlushCache();
	
	TreePtr<Node> n = AsAgent(pattern)->GetCoupled(); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
	if( !n )
		n = pattern;
	TreePtr<Expression> y = dynamic_pointer_cast<Expression>( n );
	ASSERT( y )("IsLabelReached saw pattern coupled to ")(*n)(" but an Expression is needed\n"); 
	ASSERT( *pxx );
	TreePtr<Node> nxx = *pxx;
	TreePtr<LabelIdentifier> x = dynamic_pointer_cast<LabelIdentifier>( nxx );
	ASSERT( x )("IsLabelReached at ")(**pxx)(" but is of type LabelIdentifier\n"); 
	TRACE("Can label id ")(*x)(" reach expression ")(*y)("?\n");

	Set< TreePtr<InstanceIdentifier> > uf;        
	query.AddLocalMatch( CanReachExpr(&uf, x, y) );
	TRACE("I reakon ")(*x)(query.IsLocalMatch()?" does ":" does not ")("reach ")(*y)("\n"); 
}                 


bool IsLabelReached::CanReachExpr( Set< TreePtr<InstanceIdentifier> > *f,
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


bool IsLabelReached::CanReachVar( Set< TreePtr<InstanceIdentifier> > *f,
			            	      TreePtr<LabelIdentifier> x, 
				                  TreePtr<InstanceIdentifier> y ) const // y is instance identifier. Can expression x be assigned to it?
{
	INDENT(" ");
	Reaching rr( x, y );
	if( cache.IsExist(rr) )
	{
		TRACE("cache hit yeah yeah\n");
		return cache[rr];
	}
	bool r = false;        
	Walk e( *(engine->GetOverallMaster()->pcontext) ); 
	
	if( f->IsExist(y) )
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

