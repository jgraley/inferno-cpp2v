#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "helpers/walk.hpp"
#include "helpers/simple_duplicate.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "sort_decls.hpp"
#include "render.hpp"
#include "clang/Parse/DeclSpec.h"
#include "uniquify_identifiers.hpp"
#include "search_replace.hpp"
#include "agents/special_agent.hpp"
#include "agents/standard_agent.hpp"
#include "indenter.hpp"

using namespace CPPTree; // TODO should not need
using namespace VN;


Render::Render( string output_x_path_ ) :
	default_policy( GetDefaultPolicy() ),
    output_x_path( output_x_path_ )
{
}


Render::Render( Syntax::Policy default_policy_, string output_x_path_ ) :
	default_policy( default_policy_ ),
    output_x_path( output_x_path_ )
{
}


string Render::RenderToString( shared_ptr<CompareReplace> pattern, bool lowering_for_render )
{
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = pattern->GetSearchComparePattern(); 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;

    // Make the hinted coupling names unique. Only bother with true couplings
    // (more than one parent) and don't worry about declarations.
    UniquifyNames::Policy un_policy 
    {
	    .name_getter = &Syntax::GetDesignationNameHint, 
		.include_single_parent = false,
		.include_multi_parent = true,
		.include_designation_named_identifiers = true,
		.preserve_undeclared_ids = false
	};
    UniquifyNames coupling_names_uniqifier(un_policy);
    trans_kit = TransKit{ utils.get() };
    unique_coupling_names = coupling_names_uniqifier.UniquifyAll( trans_kit, context );
	incoming_links_map = coupling_names_uniqifier.GetIncomingLinksMap();
	
	Syntax::Policy designation_policy = default_policy;
	string s;
	if( ReadArgs::use.count("c") )
		s += Trace(unique_coupling_names) + "\n\n";

	list<string> commands;

	if( lowering_for_render )
		commands.push_back("‡"); // TODO this will be the general attribute syntax, presumably with VN braces and accepting textual attributes

	for( TreePtr<Node> node : coupling_names_uniqifier.GetNodesInDepthFirstPostOrder() )	
		commands.push_back( unique_coupling_names.at(node) + 
							" ⪮ " + 
							RenderMaybeSemicolon( node, 
							                      Syntax::Production::VN_DESIGNATE, 
							                      designation_policy ) );

	ASSERT( pattern->GetSearchComparePattern() == pattern->GetReplacePattern() || !pattern->GetReplacePattern() )
	   	  (pattern->GetSearchComparePattern())
	   	  (" should equal ")
	   	  (pattern->GetReplacePattern())
	   	  (" or be NULL");
	   	  
	Syntax::Policy top_pattern_policy = default_policy;
	commands.push_back( "꩜" + 
	                    RenderIntoProduction( pattern->GetSearchComparePattern(), 
	                                          Syntax::Production::PREFIX, 
	                                          top_pattern_policy ) );
    
    s += Join( commands, "⨟\n" );
    indenter.AddLinesFromString(s);
    indenter.DoIndent();
    return indenter.GetString();
}


void Render::WriteToFile( string s )
{
    if( output_x_path.empty() )
    {
        puts( s.c_str() );
    }
    else
    {
        FILE *fp = fopen( output_x_path.c_str(), "wt" );
        ASSERT( fp )( "Cannot open output file \"%s\"", output_x_path.c_str() );
        fputs( s.c_str(), fp );
        fclose( fp );
    }        
}


Syntax::Policy Render::GetDefaultPolicy()
{
	Syntax::Policy policy;
	// our render allows eg (type1 ∧ type2) for grouping (aka parser booting) but
	// this conflicts with c-style cast syntax.
	policy.refuse_c_style_cast = true; 
	
	// constructor syntax will probably ambiguate
	policy.detect_and_render_constructor = false;
	
	// Permit map args with their non-C syntax
	policy.refuse_call_if_map_args = false;
	return policy;
}


string Render::RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod )
{
	return RenderIntoProduction( node, surround_prod, default_policy );
}


string Render::RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{	
    INDENT("R");
    string s;
    
    if( !node )
		return RenderNullPointer( surround_prod );	
					
	if( ReadArgs::use.count("c") )
		s += SSPrintf("\n// %s Node %s called from %p\n", 
				      Tracer::GetPrefix().c_str(), 
					  Traceable::TypeIdName(*node).c_str(), // No serial numbers because we diff these
					  RETURN_ADDR() );

	if( unique_coupling_names.count(node) > 0 )			
		return s + unique_coupling_names.at(node);		
	else 
		return s + RenderMaybeInitAssignment(node, surround_prod, policy);
}


string Render::RenderMaybeInitAssignment( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);

	string s;
	if( ReadArgs::use.count("c") )
		s += SSPrintf(" // Surround prod: %d, %s prod: %d\n", 
					  Syntax::GetPrecedence(surround_prod), 
					  Traceable::TypeIdName(*node).c_str(), // No serial numbers because we diff these
					  Syntax::GetPrecedence(node_prod) );		

    if( !(surround_prod == Syntax::Production::INITIALISER) )
		return s + RenderMaybeBoot(node, surround_prod, policy );

	// Deal with expression in initialiser production by prepending =
    switch(node_prod)
    {
        case Syntax::Production::BOOT_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences			
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Add init assignment, surround prod to ASSIGN\n");
			return s + " = " + RenderMaybeBoot(node, Syntax::Production::ASSIGN, policy );
			
		default:
			return s + RenderMaybeBoot(node, surround_prod, policy ); 
	}
}


string Render::RenderMaybeBoot( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);
							 		
    if( !(Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(surround_prod)) )
		return RenderMaybeSemicolon( node, surround_prod, policy );
	string s;

    switch(node_prod)
    {
        case Syntax::Production::BOOT_STMT_DECL...Syntax::Production::TOP_STMT_DECL: // Statement productions at different precedences
			// Braces can actually work in expressions, eg in {}. The nodes are STATEMENT_SEQ and we boot to BOOT_STMT_DECL
			ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACED) ||			
					Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::TOP_STMT_DECL) )
				  ("Braces won't achieve high enough precedence for surrounding statement production\n")
				  ("Node: ")(node)("\n")
				  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
			ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACKETED) )
				  ("Braces won't achieve high enough precedence for surrounding expressional or higher production\n")
				  ("Node: ")(node)("\n")
				  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
				  
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Booting statement, surround prod to BOOT_STMT_DECL\n");
				
            return "{\n " + 
                   RenderMaybeSemicolon( node, Syntax::Production::BOOT_STMT_DECL, policy ) +	
				   "\n} ";            

        case Syntax::Production::BOOT_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences
            // If current production has too-high precedence, boot back down using parentheses
			ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACKETED) )
				  ("Parentheses won't achieve high enough precedence for surrounding production\n")
				  ("Node: ")(node)("\n")
				  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Booting expression, surround prod to BOOT_EXPR\n");

            return "(\n" +
				   RenderMaybeSemicolon( node, Syntax::Production::BOOT_EXPR, policy ) +
				   "\n)";            
        
        default:        
			return RenderMaybeSemicolon( node, surround_prod, policy );         
    }
}

							
string Render::RenderMaybeSemicolon( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
	string s;

    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);
							 		    
    if( !(Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) &&
          Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_NODE_SEMICOLON) ) )
         return MaybeRenderPreRestriction( node, surround_prod, policy );
                 
	if( ReadArgs::use.count("c") )
		s += SSPrintf("// Adding semicolon, surround prod to BARE_STATEMENT\n");

	switch( surround_prod )
	{
		case Syntax::Production::BOOT_STMT_DECL:
		case Syntax::Production::STATEMENT:
		case Syntax::Production::STATEMENT_LOW...Syntax::Production::STATEMENT_HIGH:
			surround_prod = Syntax::Production::BARE_STATEMENT;
			break;
			
		case Syntax::Production::DECLARATION:
			surround_prod = Syntax::Production::BARE_DECLARATION;
			break;
			
		default:
			ASSERT(false)
			      ("Adding semicolon but not sure what bare production should be\n")
			      ("node: ")(node)("\n")
			      ("node_prod: ")((int)node_prod)("\n")
			      ("surround_prod: ")((int)surround_prod);
			
	}

 	return MaybeRenderPreRestriction( node, surround_prod, policy ) +
		   ";\n ";                                  
}


string Render::MaybeRenderPreRestriction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
	const Agent *agent = Agent::TryAsAgentConst(node);
	if( !agent )
		return Dispatch( node, surround_prod, policy );
		
	auto pspecial = dynamic_cast<const SpecialBase *>(agent);
	if( !pspecial )
		return Dispatch( node, surround_prod, policy );
	
	bool prerestricted = false;
	ASSERT( incoming_links_map.count(node)>0 )(incoming_links_map)("\nNode: ")(node);
	for( const TreePtrInterface *tpi : incoming_links_map.at(node) )
		prerestricted |= agent->IsNonTrivialPreRestriction(tpi);
		
	if(!prerestricted)
		return Dispatch( node, surround_prod, policy );
		
	// We need the archetype because otherwise we'll just get the name 
	// of the special agent. We might be able to extract node name out
	// of the template args, but using an archetype like this has precedent
	// in the graph plotter.
	TreePtr<Node> archetype_node = agent->GetArchetypeNode();

	// This assumes no action is required in order to render a prefix operation
	return "‽【" + 
	       GetInnermostTemplateParam(TYPE_ID_NAME(*archetype_node)) + 
	       "】" +
	       Dispatch( node, Syntax::Production::PREFIX, policy );	
}


string Render::RenderNullPointer( Syntax::Production surround_prod )
{	
	(void)surround_prod;
	// Assume NULL means we're in a pattern, and it represents a wildcard
	// Note same symbol as Stuff nodes etc but this is a terminal not a prefix
	// so a risk of ambiguity here.
	return "☆";
}


string Render::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{	
	try
	{
		if( const Agent *agent = Agent::TryAsAgentConst(node) )
			return agent->GetAgentRender( this, surround_prod );
	}
	catch( Syntax::Refusal & ) {}
	
	try 
	{ 
		return node->GetRender(this, surround_prod, policy); 
	}
	catch( Syntax::Refusal & ) {}

	return RenderNodeExplicit(node, surround_prod, default_policy);
}		


string Render::RenderNodeExplicit( shared_ptr<const Node> node, Syntax::Production, Syntax::Policy policy )
{
	//bool need_a_type = surround_prod >= Syntax::Production::BOOT_TYPE && 
	//                   surround_prod <= Syntax::Production::TOP_TYPE;
    string s = "⯁";
    list<string> sitems;    

    sitems.push_back( GetInnermostTemplateParam(TYPE_ID_NAME(*node)) );
	
    if( !policy.force_incomplete_records )
    {
		vector< Itemiser::Element * > items = node->Itemise();
		for( vector< Itemiser::Element * >::size_type i=0; i<items.size(); i++ )
		{
			ASSERT( items[i] )( "itemise returned null element" );
			
			if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(items[i]) )                
			{
				if( con->size() == 1 )
					sitems.push_back( RenderIntoProduction( TreePtr<Node>(con->front()), Syntax::Production::VN_SEP_ITEMS ) );
				else
				{
					list<string> scon;
					for( const TreePtrInterface &p : *con )
					{
						ASSERT( p ); 
						scon.push_back( RenderIntoProduction( TreePtr<Node>(p), Syntax::Production::COMMA_SEP ) );
					}
					if( GetTotalSize(scon) > Syntax::GetLineBreakThreshold() )
						sitems.push_back( Join( scon, ",\n", "", "") );
					else
						sitems.push_back( Join( scon, ", ", "", "") );
				}
			}            
			else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(items[i]) )
			{
				sitems.push_back( RenderIntoProduction( TreePtr<Node>(*singular), Syntax::Production::VN_SEP_ITEMS ) );
			}
			else
			{
				ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
			}
		}   
	}
    
    if( GetTotalSize(sitems) > Syntax::GetLineBreakThreshold() )
		s += Join( sitems, "⚬\n", "【\n", "\n】" );   
	else 
		s += Join( sitems, " ⚬ ", "【", "】" );    

	return s;
}


string Render::RenderScopeResolvingPrefix( TreePtr<Node> )
{
	ASSERTFAIL("VN renderer doesn't do scope resolution");
}


string Render::GetUniqueIdentifierName( TreePtr<Node> ) const 
{
	ASSERTFAIL("VN renderer never renders identifiers directly");
}


Syntax::Production Render::GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const
{
	try
	{
		if( const Agent *agent = Agent::TryAsAgentConst(node) )
			return agent->GetAgentProduction();
	}
	catch( Syntax::Refusal & ) {}
		
	try 
	{ 
		// A lot of nodes have GetMyProduction() but not GetRender(). If GetRender() is not
		// implemented, we'll generate explicit (⯁) form, which is EXPLICIT_NODE.
		// Passing in the real renderer would cause unwanted side-effects.
		struct FakeRenderer : RendererInterface
		{
			string RenderIntoProduction( TreePtr<Node>, 
										 Syntax::Production ) final { return "fake"; } 
			string RenderIntoProduction( TreePtr<Node>, 
										 Syntax::Production, 
										 Syntax::Policy ) final { return "fake"; } 
			string RenderScopeResolvingPrefix( TreePtr<Node> ) final { return ""; } 
			string GetUniqueIdentifierName( TreePtr<Node> ) const final { return "fake"; }
		} fake_renderer;
		(void)node->GetRender(&fake_renderer, surround_prod, policy);
		
		return node->GetMyProduction(this, policy); 
	}
	catch( Syntax::Refusal & ) {}

	return Syntax::Production::EXPLICIT_NODE;     
}


TreePtr<Scope> Render::TryGetScope( TreePtr<Node> node ) const
{       
    auto id = TreePtr<CPPTree::Identifier>::DynamicCast(node);
    if( !id )
        return nullptr; // GetScope() is language-specific and only works on identifiers
		
    try
    {
        return GetScope( context, id );
    }
    catch( ScopeNotFoundMismatch & )
    {
        // There is a scope but our id is not in it, maybe it was undeclared?
        return nullptr;
    }
    catch( ScopeOnNonSpecificMismatch & )
    {
        // Trying to get scope on eg Identifier during pattern rendering.
        return nullptr;
    }
}


bool Render::IsDeclared( TreePtr<Identifier> id )
{
    try
    {
        DeclarationOf().TryApplyTransformation( trans_kit, id );
        return true;
    }
    catch(DeclarationOf::DeclarationNotFound &)
    {
        return false;               
    }
} 


string Render::RenderMismatchException( string fname, const Mismatch &me )
{
    return "❌"+fname+"() error: "+me.What()+"❌";
}
