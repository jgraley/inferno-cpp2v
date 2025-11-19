#include "build_identifier_agent.hpp"
#include "tree/cpptree.hpp"
#include "agents/agent.hpp"
#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "link.hpp"
#include "lang/render.hpp"

// Not pulling in SYM because it clashes with CPPTree
//using namespace SYM;
using namespace CPPTree;

//---------------------------------- BuildIdentifierAgent ------------------------------------    

TreePtr<Node> BuildIdentifierAgent::BuildNewSubtree(const SCREngine *acting_engine)
{
    return BuildSpecificIdentifier( GetNewName(acting_engine) ); 
}


string BuildIdentifierAgent::GetNewName(const SCREngine *acting_engine)
{
    INDENT("B");
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    list<string> ls;
    bool all_same = true;
    for( TreePtrInterface &source : sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        PatternLink source_plink(&source);
        TreePtr<Node> new_identifier = source_plink.GetChildAgent()->BuildForBuildersAnalysis(source_plink, acting_engine);
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( new_identifier );
        TreePtr<SpecificIdentifier> si = DynamicTreePtrCast<SpecificIdentifier>( new_identifier );
        ASSERT( si )("BuildIdentifier: ")(*new_identifier)(" should be a kind of SpecificIdentifier (format is %s)", format.c_str());
        string s = si->GetRenderTerminal();
        if( !ls.empty() )
            all_same = all_same && (s == ls.back());
        ls.push_back( s );
    }

	// Empty format string gives default behaviour
    if( format.empty() )
    {		
		// Optional functionality: when every identifier has the same name, just return that
		// name. Handy for "merging" operations.
		if( ls.empty() )
			return "unnamed"; // TODO caps for labels
		else if( all_same )
			return ls.front();  
		else
			return Join( ls, "_" );
	}	

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( ls.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), ls.front().c_str() );
        case 2:
            return SSPrintf( format.c_str(), ls.front().c_str(), next(ls.begin())->c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}


Syntax::Production BuildIdentifierAgent::GetAgentProduction() const
{
	return Syntax::Production::PRIMITIVE_EXPR;
}


string BuildIdentifierAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;

	// List the sources as a pseudo-item
	list<string> ls_sources;
	Sequence<Node> scopy = sources;
	for( TreePtrInterface &source : scopy )
		ls_sources.push_back( kit.render( (TreePtr<Node>)source, Syntax::Production::COMMA_SEP ) );
	string sources = Join( ls_sources, ", ");
		
	// Add in the subtype name and format to make a pseudo-itemisation
	list<string> ls_production;
	ls_production.push_back(GetIdentifierSubTypeName());
	ls_production.push_back(format); // can be empty, but this is syntactically ok with ⚬
	ls_production.push_back( sources );
	
	return Join( ls_production, "⚬", "⧇【", "】");
} 
  
    
string BuildIdentifierAgent::GetCouplingNameHint() const
{
	string t = GetIdentifierSubTypeName();
	transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return tolower(c); });
	return "new_" + t + "_id";
} 


bool BuildIdentifierAgent::IsFixedType() const
{
    return true;
} 


Graphable::NodeBlock BuildIdentifierAgent::GetGraphBlockInfo() const
{
    // The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
    // the printf format string that controls the name of the generated identifier inside it.
    // TODO indicate whether it's building instance, label or type identifier
    NodeBlock block;
    block.bold = true;
    block.title = "⧇"+format; // text from program code, so use single quotes
    block.shape = "parallelogram";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = Node::GetSubblocks(const_cast<Sequence<Node> *>(&sources), phase);
    return block;
}

TreePtr<Node> BuildIdentifierAgent::TryMakeFromDestignatedType( string type_ns, string type_name, string format )
{
#define NODE(NS, NAME) \
	if( #NS==type_ns && #NAME==type_name ) \
		return MakeTreeNode<Build##NAME##IdentifierAgent>(format); \
	else
#include "tree/identifier_names.inc"	
#undef NODE
		return nullptr;
}	

//---------------------------------- BuildInstanceIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildInstanceIdentifierAgent::BuildSpecificIdentifier(string format) const
{
    return MakeTreeNode<CPPTree::SpecificInstanceIdentifier>( format ); 
}


string BuildInstanceIdentifierAgent::GetIdentifierSubTypeName() const
{
	return "Instance";
}    

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildTypeIdentifierAgent::BuildSpecificIdentifier(string format) const
{
    return MakeTreeNode<CPPTree::SpecificTypeIdentifier>( format ); 
}


string BuildTypeIdentifierAgent::GetIdentifierSubTypeName() const
{
	return "Type";
}    

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildLabelIdentifierAgent::BuildSpecificIdentifier(string format) const
{
    return MakeTreeNode<CPPTree::SpecificLabelIdentifier>( format ); 
}


string BuildLabelIdentifierAgent::GetIdentifierSubTypeName() const
{
	return "Label";
}    

//---------------------------------- BuildPreprocesorIdentifierAgent ------------------------------------    

TreePtr<CPPTree::SpecificIdentifier> BuildPreprocessorIdentifierAgent::BuildSpecificIdentifier(string format) const
{
    return MakeTreeNode<CPPTree::SpecificPreprocessorIdentifier>( format ); 
}


string BuildPreprocessorIdentifierAgent::GetIdentifierSubTypeName() const
{
	return "Preprocesor";
}    
