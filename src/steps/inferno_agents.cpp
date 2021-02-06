#include "inferno_agents.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "sr/agents/agent.hpp"
#include "sr/agents/standard_agent.hpp"
#include "sr/scr_engine.hpp"
#include "sr/search_replace.hpp"
#include "sr/link.hpp"

using namespace CPPTree;

//---------------------------------- BuildIdentifierAgent ------------------------------------    

Graphable::Block BuildIdentifierAgent::GetGraphBlockInfo() const
{
	// The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
	// the printf format string that controls the name of the generated identifier inside it.
	// TODO indicate whether it's building instance, label or type identifier
    Block block;
	block.bold = true;
	block.title = "BuildIdentifier"; 
	block.symbol = "'"+format+"'"; // text from program code, so use single quotes
	block.shape = "parallelogram";
    block.block_type = Graphable::NODE;
    return block;
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
        TreePtr<SpecificIdentifier> si = DynamicTreePtrCast<SpecificIdentifier>( n );
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

TreePtr<Node> BuildInstanceIdentifierAgent::BuildReplaceImpl() 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        CouplingKey keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(GetKey().GetChildX());   
}                                                   

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<Node> BuildTypeIdentifierAgent::BuildReplaceImpl() 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        CouplingKey keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificTypeIdentifier>( new CPPTree::SpecificTypeIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(GetKey().GetChildX());   
}                                                   

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<Node> BuildLabelIdentifierAgent::BuildReplaceImpl() 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        CouplingKey keylink = XLink::CreateDistinct( TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) ) );
        SetKey( keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(GetKey().GetChildX());   
}                                                   

//---------------------------------- IdentifierByNameAgent ------------------------------------    

Graphable::Block IdentifierByNameAgent::GetGraphBlockInfo() const
{
	// The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
	// the string that must be matched inside it.
	// TODO indicate whether it's matching instance, label or type identifier
    // Update: PreRestriction indicator seems to be doing that now
    Block block;
	block.bold = true;
    block.title = "IdentifierByName";
	block.symbol = "'" + name + "'";
	block.shape = "trapezium";
    block.block_type = Graphable::NODE;
    return block;
}


shared_ptr<PatternQuery> IdentifierByNameAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void IdentifierByNameAgent::RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                                 XLink base_xlink ) const                
{
    string newname = name; 
    TreePtr<Node> base_x = base_xlink.GetChildX(); // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( auto si_x = DynamicTreePtrCast<CPPTree::SpecificIdentifier>(base_x) )
    {
        TRACE("Comparing ")(si_x->GetRender())(" with ")(newname);
        if( si_x->GetRender() == newname )
        {
            TRACE(" : same\n");
            return;
        }
        TRACE(" : different\n");
    }
    throw Mismatch();  
}                                

//---------------------------------- NestedAgent ------------------------------------    

shared_ptr<PatternQuery> NestedAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, &terminus) );
	if( depth )
		pq->RegisterNormalLink( PatternLink(this, &depth) ); // local
    return pq;
}


void NestedAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                       XLink base_xlink ) const                          
{
    INDENT("N");
    
    // Do the teleporty bit
    TeleportAgent::RunDecidedQueryPRed(query, base_xlink);    
    
    string s;
    // Keep advancing until we get nullptr, and remember the last non-null position
    int i = 0;
    XLink xlink = base_xlink;
    while( XLink next_xlink = Advance(xlink, &s) )
        xlink = next_xlink;
            
    // Compare the last position with the terminus pattern
    query.RegisterNormalLink( PatternLink(this, &terminus), xlink ); // Link into X
}

    
map<PatternLink, XLink> NestedAgent::RunTeleportQuery( XLink base_xlink ) const
{
    map<PatternLink, XLink> tp_links;
    
    // Compare the depth with the supplied pattern if present
    if( depth )
    {
        string s;
        // Keep advancing until we get nullptr, and remember the last non-null position
        int i = 0;
        XLink xlink = base_xlink;
        while( XLink next_xlink = Advance(xlink, &s) )
            xlink = next_xlink;

        TreePtr<Node> cur_depth( new SpecificString(s) );
        XLink new_xlink = XLink::CreateDistinct(cur_depth); // cache will un-distinct
        tp_links[PatternLink(this, &depth)] = new_xlink;
    }
    
    return tp_links;
}    


Graphable::Block NestedAgent::GetGraphBlockInfo() const
{
    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "plaintext";
    block.block_type = Graphable::NODE;
    if( terminus )
        block.sub_blocks.push_back( { "terminus", 
                                      "", 
                                      false,
                                      { { terminus, 
                                          &terminus,
                                          THROUGH, 
                                          {},
                                          {PatternLink(this, &terminus).GetShortName()} } } } );
    if( depth )
        block.sub_blocks.push_back( { "depth", 
                                      "", 
                                      false,
                                      { { depth, 
                                          &depth,
                                          THROUGH, 
                                          {},
                                          {PatternLink(this, &depth).GetShortName()} } } } );
    return block;
}

//---------------------------------- NestedArrayAgent ------------------------------------    

XLink NestedArrayAgent::Advance( XLink xlink, 
                                 string *depth ) const
{
    if( auto a = TreePtr<Array>::DynamicCast(xlink.GetChildX()) )         
        return XLink(a, &(a->element)); // TODO support depth string (or integer)
    else
        return XLink();
}

//---------------------------------- NestedSubscriptLookupAgent ------------------------------------    

XLink NestedSubscriptLookupAgent::Advance( XLink xlink, 
                                           string *depth ) const
{
    if( auto s = DynamicTreePtrCast<Subscript>(xlink.GetChildX()) )            
    {
        *depth += "S";
        return XLink(s, &(s->operands.front())); // the base, not the index
    }
    else if( auto l = DynamicTreePtrCast<Lookup>(xlink.GetChildX()) )            
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

TreePtr<Node> BuildContainerSizeAgent::BuildReplaceImpl() 
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
        CouplingKey keylink = XLink::CreateDistinct( MakePatternPtr<SpecificInteger>(size) );
		SetKey( keylink );
	}
	// Note that the keylink could have been set via coupling - but still not
	// likely to do anything sensible, so explicitly check
	return DuplicateSubtree(GetKey().GetChildX());   
}                                                   


Graphable::Block BuildContainerSizeAgent::GetGraphBlockInfo() const
{

    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "egg";
    block.block_type = Graphable::NODE;
    if( container )
        block.sub_blocks.push_back( { "container", 
                                      "", 
                                      false,
                                      { { (TreePtr<Node>)container, 
                                          nullptr,
                                          THROUGH, 
                                          {},
                                          {} } } } );
    return block;
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
    
    
void IsLabelReachedAgent::RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                               XLink base_xlink ) const
{
	INDENT("L");
	ASSERT( pattern );
	
	// TODO Flushable mechanism removed - flush every time for safety (if
	// this code ever gets used again). This may be slow!
	// x is nominally the label id, at the position of this node
	// y is nominally the goto expression, coupled in
	FlushCache();
	
	TreePtr<Node> child_x = AsAgent(pattern)->GetKey().GetChildX(); // TODO a templates version that returns same type as pattern, so we don't need to convert here?
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

Graphable::Block IsLabelReachedAgent::GetGraphBlockInfo() const
{

    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "egg";
    block.block_type = Graphable::NODE;
    if( pattern )
        block.sub_blocks.push_back( { "pattern", 
                                      "", 
                                      false,
                                      { { pattern, 
                                          &pattern,
                                          THROUGH, 
                                          {},
                                          {PatternLink(this, &pattern).GetShortName()} } } } );
    return block;
}
