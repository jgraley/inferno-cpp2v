//---------------------------------- IsLabelReachedAgent ------------------------------------    

void IsLabelReachedAgent::FlushCache() const 
{
    ASSERT(0);
    cache.clear();
}


void IsLabelReachedAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                               XLink base_xlink ) const
{
	INDENT("L");
	ASSERT( pattern );
	
	// TODO Flushable mechanism removed - flush every time for safety (if
	// this code ever gets used again). This may be slow!
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
	FlushCache();
	
	TreePtr<Node> child_x = base_xlink.GetChildX(); // TODO a templated version that returns same type as pattern, so we don't need to convert here?
	if( !child_x )
		child_x = pattern;
	TreePtr<Expression> e_child_x = DynamicTreePtrCast<Expression>( child_x );
	ASSERT( e_child_x )("IsLabelReachedAgent saw pattern coupled to ")(child_x)(" but an Expression is needed\n"); 
	ASSERT( base_xlink );
	TreePtr<Node> base_x = base_xlink.GetChildX();
	TreePtr<LabelIdentifier> lid_x = DynamicTreePtrCast<LabelIdentifier>( base_x );
	ASSERT( lid_x )("IsLabelReachedAgent at ")(base_xlink)(" but is of type LabelIdentifier\n"); 
	TRACE("Can label id ")(lid_x)(" reach expression ")(e_child_x)("?\n");

	set< TreePtr<InstanceIdentifier> > uf;        
    if( !CanReachExpr(&uf, lid_x, e_child_x) )
        throw Mismatch();  
	TRACE("I reakon ")(lid_x)(" reaches ")(e_child_x)("\n"); 
}                 


bool IsLabelReachedAgent::CanReachExpr( set< TreePtr<InstanceIdentifier> > *f,
								   TreePtr<LabelIdentifier> x, 
					               TreePtr<Expression> y ) const // y is expression. Can it yield label x?
{
	INDENT("X");
	bool r = false;
	if( TreePtr<LabelIdentifier> liy = DynamicTreePtrCast<LabelIdentifier>(y) )
    {
		r = liy->IsLocalMatch( x.get() ); // y is x, so yes
    }
	else if( TreePtr<InstanceIdentifier> iiy = DynamicTreePtrCast<InstanceIdentifier>( y ) )
    {
		r = CanReachVar(f, x, iiy );
    }
	else if( TreePtr<Ternop> ty = DynamicTreePtrCast<Ternop>( y ) )
    {
        Sequence<Expression>::iterator ops_it = ty->operands.begin();
        ++ops_it; // only the choices, not the condition
		r = CanReachExpr(f, x, *ops_it);
        ++ops_it;
		r = r || CanReachExpr(f, x, *ops_it); 
    }
	else if( TreePtr<Comma> cy = DynamicTreePtrCast<Comma>( y ) )
    {
		r = CanReachExpr(f, x, cy->operands.back()); // second operand
    }
	else if( TreePtr<Subscript> sy = DynamicTreePtrCast<Subscript>( y ) ) // subscript as r-value
	{
        r = CanReachExpr(f, x, sy->operands.front()); // first operand
    }
	else if( DynamicTreePtrCast<Dereference>( y ) )
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
		if( TreePtr<Assign> a = DynamicTreePtrCast<Assign>((TreePtr<Node>)n) )
		{
			TreePtr<Expression> lhs = a->operands.front();
			if( TreePtr<Subscript> slhs = DynamicTreePtrCast<Subscript>( lhs ) ) // subscript as l-value 
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

Graphable::Block IsLabelReachedAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{

    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "egg";
    block.block_type = Graphable::NODE;
    if( pattern )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(pattern.get()),
                  list<string>{},
                  list<string>{PatternLink(this, &pattern).GetShortName()},
                  phase,
                  SpecialBase::IsNonTrivialPreRestriction(&pattern) );
        block.sub_blocks.push_back( { "pattern", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}
