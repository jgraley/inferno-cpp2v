#include "tree/cpptree.hpp"
#include "tree/localtree.hpp"
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
							AccomodateInit( node, 
							                Syntax::Production::VN_DESIGNATE, 
							                designation_policy ) );

	ASSERT( pattern->GetSearchComparePattern() == pattern->GetReplacePattern() || !pattern->GetReplacePattern() )
	   	  (pattern->GetSearchComparePattern())
	   	  (" should equal ")
	   	  (pattern->GetReplacePattern())
	   	  (" or be NULL");
	   	  
	Syntax::Policy top_pattern_policy = default_policy;
	commands.push_back( "꩜" + 
	                    DoRender( pattern->GetSearchComparePattern(), 
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
	policy.refuse_map_argumentation = false;

	// Don't insert {} into patterns because they imply Compound or StatementExpression
	// or ArrayLiteral or RecordLiteral. Use () instead, which are purely for disamiguation.
	policy.boot_statements_using_braces = false;
	
	// Need to separate members from statements in Compound
	policy.compound_uses_vn_separator = true;
	
	// Don't bother trying to use * and && with goto-a-variable
	policy.goto_uses_ref_and_deref = false;
	
	// Rendering local node types that don't have their own syntax will just
	// use the parent class's render which will be parsed back as the parent class.
	policy.refuse_local_node_types = true;
	
	return policy;
}


string Render::DoRender( TreePtr<Node> node, Syntax::Production surround_prod )
{
	return DoRender( node, surround_prod, default_policy );
}


string Render::DoRender( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{	
    INDENT("R");
    string s;
		
	if( ReadArgs::use.count("c") )
		s += SSPrintf("\n//%s Node %s called from %p\n", 
				      Tracer::GetPrefix().c_str(), 
					  Traceable::TypeIdName(*node).c_str(), // No serial numbers because we diff these
					  RETURN_ADDR() );

	if( unique_coupling_names.count(node) > 0 )			
	{
		s += unique_coupling_names.at(node);
		// Does the designation need a swemicolon?
		// TODO duplicating AccomodateSemicolon(): resolve by having these act on strings rather than nodes
		if( Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_SURR_SEMICOLON) &&
            Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) )
            s += ";\n";
		return s;
	}
	else 
		return s + AccomodateInit(node, surround_prod, policy);
}


string Render::AccomodateInit( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);

	string s;
	if( ReadArgs::use.count("c") )
		s += SSPrintf("\n//%s Node %s, surround prod: %d, node prod: %d\n", 
					  Tracer::GetPrefix().c_str(), 
					  Traceable::TypeIdName(*node).c_str(), // No serial numbers because we diff these
					  Syntax::GetPrecedence(surround_prod), 
					  Syntax::GetPrecedence(node_prod) );		

    if( node_prod == Syntax::Production::INITIALISER || node_prod == Syntax::Production::COMPOUND ||
		surround_prod != Syntax::Production::INITIALISER )
		return s + AccomodateBoot(node, surround_prod, policy );

	// Deal with expression in initialiser production by prepending =
    switch(node_prod)
    {
        case Syntax::Production::BOTTOM_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences			
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Add init assignment, surround prod to ASSIGN\n");
			return s + " = " + AccomodateBoot(node, Syntax::Production::ASSIGN, policy );
			
		default:
			return s + AccomodateBoot(node, surround_prod, policy ); 
	}
}


string Render::AccomodateBoot( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);
							 		
    if( !(Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(surround_prod)) )
		return AccomodateSemicolon( node, surround_prod, policy );
	string s;

    switch(node_prod)
    {
        case Syntax::Production::BOOT...Syntax::Production::TOP_STMT_DECL: // Statement productions at different precedences
			if( policy.boot_statements_using_braces )
			{
				// Braces can actually work in expressions, eg in {}. The nodes are STATEMENT_SEQ and we boot to BOTTOM_STMT_DECL
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
					s += SSPrintf("// Booting statement, surround prod to BOTTOM_STMT_DECL\n");
					
				return s + "{\n " + 
					   AccomodateSemicolon( node, Syntax::Production::BOOT, policy ) +	
					   "}\n";         
			}   
			[[fallthrough]]; // ELSE FALL THOROUGH INTO PARENS CASE
			
        case Syntax::Production::BOTTOM_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences
            // If current production has too-high precedence, boot back down using parentheses
			ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACKETED) )
				  ("Parentheses won't achieve high enough precedence for surrounding production\n")
				  ("Node: ")(node)("\n")
				  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Booting expression, surround prod to BOTTOM_EXPR\n");

            s += "( " +
				   AccomodateSemicolon( node, Syntax::Production::BOOT, policy ) +
				   " )";            
				   
			// Node prod is now effectively BRACKETED... do we now need a semicolon?
			if( Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_SURR_SEMICOLON) &&
				Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) )
				s += ";\n";		
			
			return s;
        
        default:        
			return AccomodateSemicolon( node, surround_prod, policy );         
    }
}

							
string Render::AccomodateSemicolon( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
	string s;

    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);
		
	bool semicolon = false;					 		    
    if( Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_SURR_SEMICOLON) &&
        Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) &&
        Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_NODE_SEMICOLON) &&
        node_prod != Syntax::Production::COMPOUND )
        semicolon = true;
        
    if( (Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::BOTTOM_EXPR) &&
         Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_EXPR)) ||
        (Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::BOTTOM_VN) &&
         Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_VN)) )
        if( Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_NODE_SEMICOLON) &&
			Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_STMT_DECL) &&
			node_prod != Syntax::Production::COMPOUND )
			semicolon = true;       
          
    if( !semicolon )
         return AccomodatePreRestriction( node, surround_prod, policy );

	if( ReadArgs::use.count("c") )
		s += SSPrintf("// Adding semicolon, surround prod to BARE_STATEMENT\n");

	switch( surround_prod )
	{
		case Syntax::Production::BOTTOM_STMT_DECL:
		case Syntax::Production::STATEMENT:
		case Syntax::Production::STATEMENT_LOW...Syntax::Production::STATEMENT_HIGH:
			surround_prod = Syntax::Production::BARE_STATEMENT;
			break;
			
		case Syntax::Production::DECLARATION:
			surround_prod = Syntax::Production::BARE_DECLARATION;
			break;
			
		default: // Expr cases -> expr decays to statement
			surround_prod = Syntax::Production::BARE_STATEMENT;
			break;
			
	}

 	return s + AccomodatePreRestriction( node, surround_prod, policy ) +
		   ";\n";                                  
}


string Render::AccomodatePreRestriction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{   
    if( !node )
		return RenderNullPointer( surround_prod );	
			
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
	return "‽" + 
	       GetInnermostTemplateParam(TYPE_ID_NAME(*archetype_node)) + 
	       " " +
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

	return RenderNodeExplicit(node, surround_prod, policy);
}		


string Render::RenderNodeExplicit( shared_ptr<const Node> node, Syntax::Production, Syntax::Policy policy )
{
	//bool need_a_type = surround_prod >= Syntax::Production::BOOT_TYPE && 
	//                   surround_prod <= Syntax::Production::TOP_TYPE;
    string s = "⯁";
    list<string> sitems;    

    s += GetInnermostTemplateParam(TYPE_ID_NAME(*node));

	if( ReadArgs::use.count("c") )
		s += policy.force_incomplete_records ? "/* force incomplete */" : "/* no force incomplete */";
    
    if( !policy.force_incomplete_records )
    {
		vector< Itemiser::Element * > items = node->Itemise();
		for( vector< Itemiser::Element * >::size_type i=0; i<items.size(); i++ )
		{
			ASSERT( items[i] )( "itemise returned null element" );
			
			if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(items[i]) )                
			{
				if( con->size() == 1 )
					sitems.push_back( DoRender( TreePtr<Node>(con->front()), Syntax::Production::VN_SEP_ITEMS ) );
				else
				{
					list<string> scon;
					for( const TreePtrInterface &p : *con )
					{
						ASSERT( p ); 
						scon.push_back( DoRender( TreePtr<Node>(p), Syntax::Production::VN_SEP_ITEMS ) );
					}
					if( GetTotalSize(scon) > Syntax::GetLineBreakThreshold() )
						sitems.push_back( Join( scon, "🞄\n", "", "") );
					else
						sitems.push_back( Join( scon, "🞄", "", "") );
				}
			}            
			else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(items[i]) )
			{
				string ss = DoRender( TreePtr<Node>(*singular), Syntax::Production::VN_SEP_ITEMS );
				sitems.push_back( ss );
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
	if( !node )
		return Syntax::Production::NULLPTR;
		
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
			string DoRender( TreePtr<Node>, 
										 Syntax::Production ) final { return "fake"; } 
			string DoRender( TreePtr<Node>, 
										 Syntax::Production, 
										 Syntax::Policy ) final { return "fake"; } 
			string RenderScopeResolvingPrefix( TreePtr<Node> ) final { return ""; } 
			string GetUniqueIdentifierName( TreePtr<Node> ) const final { return "fake"; }
		    string RenderNodeExplicit( shared_ptr<const Node>, 
									   Syntax::Production, 
		                               Syntax::Policy ) final { return "fake"; } 	
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
