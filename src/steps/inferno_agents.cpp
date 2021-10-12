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

Graphable::Block BuildIdentifierAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
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

TreePtr<Node> BuildInstanceIdentifierAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("%");
    if( !keynode )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        keynode = TreePtr<CPPTree::SpecificInstanceIdentifier>( new CPPTree::SpecificInstanceIdentifier( newname ) );
        CouplingKey keylink(XLink::CreateDistinct( keynode ), KEY_PRODUCER_7 );
        master_scr_engine->SetReplaceKey( this, keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keynode);   
}                                                   

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<Node> BuildTypeIdentifierAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl 
        string newname = GetNewName();
        keynode = TreePtr<CPPTree::SpecificTypeIdentifier>( new CPPTree::SpecificTypeIdentifier( newname ) );
        CouplingKey keylink( XLink::CreateDistinct( keynode ), KEY_PRODUCER_7 );
        master_scr_engine->SetReplaceKey( this, keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keynode);   
}                                                   

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<Node> BuildLabelIdentifierAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("%");
    if( !GetKey() )
    {
        // Call the soft pattern impl
        string newname = GetNewName();
        keynode = TreePtr<CPPTree::SpecificLabelIdentifier>( new CPPTree::SpecificLabelIdentifier( newname ) );
        CouplingKey keylink( XLink::CreateDistinct( keynode ), KEY_PRODUCER_7 );
        master_scr_engine->SetReplaceKey( this, keylink );
    }
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(keynode);   
}                                                   

//---------------------------------- IdentifierByNameAgent ------------------------------------    

Graphable::Block IdentifierByNameAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
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


void IdentifierByNameAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
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


Graphable::Block NestedAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{
    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "plaintext";
    block.block_type = Graphable::NODE;
    if( terminus )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(terminus.get()),
                  list<string>{},
                  list<string>{PatternLink(this, &terminus).GetShortName()},
                  phase,
                  SpecialBase::IsNonTrivialPreRestriction(&terminus) );
        block.sub_blocks.push_back( { "terminus", 
                                      "", 
                                      false,
                                      { link } } );
    }
    if( depth )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(depth.get()),
                  list<string>{},
                  list<string>{PatternLink(this, &depth).GetShortName()},
                  phase,
                  SpecialBase::IsNonTrivialPreRestriction(&depth) );
        block.sub_blocks.push_back( { "depth", 
                                      "", 
                                      false,
                                      { link } } );
    }
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

TreePtr<Node> BuildContainerSizeAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
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
        keynode = MakePatternPtr<SpecificInteger>(size); // Not sure about using MakePattenPtr here
        CouplingKey keylink( XLink::CreateDistinct( keynode ), KEY_PRODUCER_7 );
		master_scr_engine->SetReplaceKey( this, keylink );
	}
	// Note that the keylink could have been set via coupling - but still not
	// likely to do anything sensible, so explicitly check
	return DuplicateSubtree(keynode);   
}                                                   


Graphable::Block BuildContainerSizeAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{

    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "egg";
    block.block_type = Graphable::NODE;
    if( container )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(container.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  SpecialBase::IsNonTrivialPreRestriction(&container) );
        block.sub_blocks.push_back( { "container", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}

