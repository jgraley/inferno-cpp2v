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
		s += "/* Unique coupling names: " + Trace(unique_coupling_names) + " */\n\n";

	list<string> commands;

	if( lowering_for_render )
		commands.push_back("‡"); // TODO this will be the general attribute syntax, presumably with VN braces and accepting textual attributes

	for( TreePtr<Node> node : coupling_names_uniqifier.GetNodesInDepthFirstPostOrder() )	
	{
		// Use the incoming links to determine whether we should render as a type or not.
		designation_policy.pointer_archetype = MakeTreeNode<Node>();	

		set<const TreePtrInterface *> ils = incoming_links_map.at(node);		
		for( const TreePtrInterface *il : ils )
		{
			TreePtr<Node> arch = il->MakeValueArchetype();
			if( dynamic_pointer_cast<CPPTree::Type>(arch) ) // Prioritise Type since we don't have eg NonType
				designation_policy.pointer_archetype = arch;
		}		
			
		commands.push_back( unique_coupling_names.at(node) + 
							" ⪮ " + 
							AccomodateInit( node, Syntax::Production::VN_DESIGNATE_RHS, designation_policy ) );
		// Alternative for declarator desaignators
		//commands.push_back( " ⪮ " + 
		//					AccomodateBootTypeAndDeclarator( node, 
		//					                           unique_coupling_names.at(node), 
		//					                           Syntax::Production::PURE_IDENTIFIER, 
		//					                           Syntax::Production::VN_DESIGNATE_RHS, 
		//					                           designation_policy,
		//					                           false ) );
	}

	ASSERT( pattern->GetSearchComparePattern() == pattern->GetReplacePattern() || !pattern->GetReplacePattern() )
	   	  (pattern->GetSearchComparePattern())
	   	  (" should equal ")
	   	  (pattern->GetReplacePattern())
	   	  (" or be NULL");
	   	  
	Syntax::Policy top_pattern_policy = default_policy;
	commands.push_back( "꩜" + 
	                    DoRenderPreserve( pattern->GetSearchComparePattern(), 
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
		
	// No parse for this at the momnet, could be hard to parse TODO try
	policy.refuse_statement_expression = true;
	
	// constructor syntax will probably ambiguate
	policy.detect_and_render_constructor = false;
	
	// Permit map args with their non-C syntax
	policy.convert_argumentation_to_seq = false;

	// Don't insert {} into patterns because they imply Compound or StatementExpression
	// or ArrayInitialiser or RecordInitialiser. Use () instead, which are purely for disamiguation.
	policy.boot_statements_using_braces = false;
	
	// Need to separate members from statements in Compound
	policy.compound_uses_vn_separator = true;
	
	// Don't bother trying to use * and && with goto-a-variable
	policy.goto_uses_ref_and_deref = false;
	
	// Rendering local node types that don't have their own syntax will just
	// use the parent class's render which will be parsed back as the parent class,
	// which is an error. Or, the render is ambiguous in vn files.
	policy.refuse_local_node_types = true;
	
	policy.full_render_code_unit = false;
	
	// #877 we don't need to generate public: for nodes without an access when
	// rendering to VN, because the parser will ignore the current_access for
	// these nodes anyway - there is nothting it can do with the access value.
	policy.missing_access_to_public = false;
	
	return policy;
}


string Render::DoRender( const TreePtrInterface *tpi, 
                            Syntax::Production surround_prod, 
                            Syntax::Policy policy )
{
	policy.pointer_archetype = tpi->MakeValueArchetype();
	return DoRenderPreserve( (TreePtr<Node>)(*tpi), surround_prod, policy );
}


string Render::DoRenderPreserve( TreePtr<Node> node, 
                                 Syntax::Production surround_prod, 
                                 Syntax::Policy policy )
{	
    INDENT(">");
    string s;
		
	if( ReadArgs::use.count("c") )
		s += SSPrintf("\n//%s DoRenderPreserve Node %s called from %p\n", 
				      Tracer::GetPrefix().c_str(), 
					  node ? Traceable::TypeIdName(*node).c_str() : "NULL", // No serial numbers because we diff these
					  RETURN_ADDR() );

	if( unique_coupling_names.count(node) > 0 )			
	{
		s += unique_coupling_names.at(node);
		// These duplicate the accomodations partially: they depend on surround_prod but not the node. To 
		// push them through the same functions, we should not provide the node as it's now irrelevent
		// syntactially. Maybe pass node_prod in separately - this can be chosen to get the right accomodations
		// and differentiate from NULL node meaning Nop.
		if( surround_prod == Syntax::Production::DIRECT_INIT )
			s = "= " + s;
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
		s += SSPrintf("\n//  %s Node %s, surround prod: %d, node prod: %d\n", 
					  Tracer::GetPrefix().c_str(), 
					  node ? Traceable::TypeIdName(*node).c_str() : "NULL", // No serial numbers because we diff these
					  Syntax::GetPrecedence(surround_prod), 
					  Syntax::GetPrecedence(node_prod) );		

	// we only act inside an DIRECT_INIT production and never on already-matching production
	if( surround_prod != Syntax::Production::DIRECT_INIT || node_prod == surround_prod )
		return AccomodateBoot(node, surround_prod, policy ); 

	// Deal with expression in initialiser production by prepending =
	if( node_prod != Syntax::Production::COMPOUND ) // no = for COMPOUND
	{
		if( ReadArgs::use.count("c") )
			s += SSPrintf("// Add init assignment, surround prod to ASSIGN\n");
		return s + "= " + AccomodateBoot(node, Syntax::Production::ASSIGN, policy );
	}

	return s + AccomodateBoot(node, surround_prod, policy ); 
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
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) ||			
						Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::TOP_STMT_DECL) )
					  ("Braces won't achieve high enough precedence for surrounding statement production\n")
					  ("Node: ")(node)("\n")
					  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
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
			ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
				  ("Parentheses won't achieve high enough precedence for surrounding production\n")
				  ("Node: ")(node)("\n")
				  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			if( ReadArgs::use.count("c") )
				s += SSPrintf("// Booting expression, surround prod to BOTTOM_EXPR\n");

            s += "( " +
				   AccomodateSemicolon( node, Syntax::Production::BOOT, policy ) +
				   " )";            
				   
			// Node prod is now effectively PARENTHESISED... do we now need a semicolon?
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
	string explain;

    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    Syntax::Production node_prod = GetNodeProduction(node, surround_prod, policy);
		
	bool semicolon = false;					 		    
    if( Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_SURR_SEMICOLON) &&
        Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::MAX_SURR_SEMICOLON) &&
        Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_NODE_SEMICOLON) &&
        node_prod != Syntax::Production::COMPOUND )
    {
		explain += SSPrintf("/* Adding semicolon, reason 1, %d %d */", Syntax::GetPrecedence(surround_prod), Syntax::GetPrecedence(node_prod));
        semicolon = true;
    }
        
    if( (Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::BOTTOM_EXPR) &&
         Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_EXPR)) ||
        (Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::BOTTOM_VN) &&
         Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_VN)) )
        if( Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::MIN_NODE_SEMICOLON) &&
			Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(Syntax::Production::TOP_STMT_DECL) &&
			node_prod != Syntax::Production::COMPOUND )
		{
			explain += SSPrintf("/* Adding semicolon, reason 2, %d %d */", Syntax::GetPrecedence(surround_prod), Syntax::GetPrecedence(node_prod));
			semicolon = true;
		}			
          
    if( !semicolon )
		explain += SSPrintf("/* Not adding semicolon, %d %d */", Syntax::GetPrecedence(surround_prod), Syntax::GetPrecedence(node_prod));	
	
	if( !ReadArgs::use.count("c") )
		explain = ""; // shush!
	
    if( semicolon )
		return s + AccomodatePreRestriction( node, Syntax::Production::BARE_STMT_DECL, policy ) + explain + ";\n";            
	else
        return s + AccomodatePreRestriction( node, surround_prod, policy ) + explain;		                      
}


string Render::AccomodatePreRestriction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{   
    if( !node )
		return RenderNullPointer(surround_prod, policy);	
			
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
	       RenderNodeTypeName(archetype_node) + 
	       " " +
	       Dispatch( node, Syntax::Production::PREFIX, policy );	
}


string Render::RenderNullPointer(Syntax::Production surround_prod, Syntax::Policy policy)
{	
	if( auto arch = dynamic_pointer_cast<Qualifier>( policy.pointer_archetype ) )
		return Dispatch( (TreePtr<Node>)arch, surround_prod, policy );
	else if( dynamic_pointer_cast<Type>( policy.pointer_archetype ) )
		return "⍑☆";
	else 
		return "☆";
}


string Render::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{	
	try
	{
		if( const Agent *agent = Agent::TryAsAgentConst(node) )
			return agent->GetAgentRender( this, surround_prod, policy );
	}
	catch( Syntax::Refusal & ) {}
	
	try 
	{ 
		return node->GetRender(this, surround_prod, policy); 
	}
	catch( Syntax::Refusal & ) {}

	return RenderNodeExplicit(node, surround_prod, policy);
}		


list<string> Render::PopulateItemStrings( shared_ptr<const Node> node, Syntax::Policy policy )
{
    list<string> sitems;    

   	// Special case for specific identifiers
	if( auto node_sid = dynamic_pointer_cast<const CPPTree::SpecificIdentifier>( node ) )
	{
		sitems.push_back( "\"" + node_sid->GetIdentifierName() + "\"");
		return sitems;
	}		
    
    if( !policy.force_incomplete_records )
    {
		vector< Itemiser::Element * > items = node->Itemise();
		for( vector< Itemiser::Element * >::size_type i=0; i<items.size(); i++ )
		{
			ASSERT( items[i] )( "itemise returned null element" );
			
			if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(items[i]) )                
			{
				if( con->size() == 1 )
					sitems.push_back( DoRender( &(con->front()), Syntax::Production::VN_SEP_ITEMS, policy ) );
				else
				{
					list<string> scon;
					for( const TreePtrInterface &p : *con )
					{
						ASSERT( p ); 
						scon.push_back( DoRender( &p, Syntax::Production::VN_SEP_ITEMS, policy ) );
					}
					if( GetTotalSize(scon) > Syntax::GetLineBreakThreshold() )
						sitems.push_back( Join( scon, "🞄\n", "", "") );
					else
						sitems.push_back( Join( scon, "🞄", "", "") );
				}
			}            
			else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(items[i]) )
			{
				string ss = DoRender( singular, Syntax::Production::VN_SEP_ITEMS, policy );
				sitems.push_back( ss );
			}
			else
			{
				ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
			}
		}   
	}

	return sitems;
}


string Render::RenderNodeTypeName( shared_ptr<const Node> node )
{
	list<string> parts = Split( GetInnermostTemplateParam(TYPE_ID_NAME(*node)), "::" );
	
	if( parts.front()==DEFAULT_NODE_NAMESPACE )
		parts.pop_front();		
			
    return Join( parts, "::" );    
}


string Render::RenderNodeExplicit( shared_ptr<const Node> node, Syntax::Production, Syntax::Policy policy )
{
    string s = "⯁";
    			
    s += RenderNodeTypeName(node); 
   
	if( ReadArgs::use.count("c") )
		s += policy.force_incomplete_records ? "/* force incomplete */" : "/* no force incomplete */";
    
    list<string> sitems = PopulateItemStrings( node, policy );    
    if( GetTotalSize(sitems) > Syntax::GetLineBreakThreshold() )
		s += Join( sitems, "⚬\n", "⦅\n", "\n⦆" );   
	else 
		s += Join( sitems, " ⚬ ", "⦅", "⦆" );    

	return s;
}


string Render::RenderScopeResolvingPrefix( TreePtr<Node>, Syntax::Policy )
{
	ASSERTFAIL("VN renderer doesn't do scope resolution");
}


string Render::GetUniqueIdentifierName( TreePtr<Node> ) const 
{
	ASSERTFAIL("VN renderer never renders identifiers directly");
}


string Render::DoRenderTypeAndDeclarator( const TreePtrInterface *tpi, string declarator, 
                                          Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                          bool constant ) 
{
	policy.pointer_archetype = tpi->MakeValueArchetype(); // pointer_archetype applies to the type
	return DoRenderTypeAndDeclaratorPreserve( (TreePtr<Node>)(*tpi), declarator, declarator_prod, surround_prod, policy, constant );
}

                                          
string Render::DoRenderTypeAndDeclaratorPreserve( TreePtr<Node> type, string declarator, 
                                                  Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                                  bool constant ) 
{
	if( !type )
	{
		// ☆ would be way too ambiguous, a wild declaration would just be ☆ ☆ for example. So don't render NULL.
		type = MakeTreeNode<Type>();
	}
	
	if( unique_coupling_names.count(type) > 0 )					
		return unique_coupling_names.at(type) + " " + declarator;	
	else 
		return AccomodateBootTypeAndDeclarator(type, declarator, declarator_prod, surround_prod, policy, constant);
}


string Render::AccomodateBootTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                                Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                                bool constant ) 
{
	ASSERT(type);
    try
    {
		// Production passed in here comes from the current value of the delcarator string, not surrounding production.
		Syntax::Production prod_surrounding_declarator = type->GetOperandInDeclaratorProduction();
		ASSERT( Syntax::GetPrecedence(prod_surrounding_declarator) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) ); // Can't satisfy this production's precedence demand using parentheses
		ASSERT( Syntax::GetPrecedence(declarator_prod) >= Syntax::GetPrecedence(Syntax::Production::BOTTOM_EXPR) ); // Can't put this node into parentheses
		bool parenthesise = Syntax::GetPrecedence(declarator_prod) < Syntax::GetPrecedence(prod_surrounding_declarator);  
		// Apply to object rather than recursing, because this is declarator    
		if( parenthesise )
			declarator = "(" + declarator + ")";
    }
    catch( Syntax::Unimplemented & )
    {
		// Assume that type also throws on RenderTypeAndDeclarator, and won't do anything
		//ASSERT( false )(type)(" \"")(declarator)("\" type node prod=")((int)GetNodeProduction(type, Syntax::Production::SPACE_SEP_TYPE, policy));
	}
        
    return DispatchTypeAndDeclarator( type, declarator, declarator_prod, surround_prod, policy, constant );
}


string Render::DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                          Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                          bool constant )
{
	try
	{
		// Agents refuse in general to deal with declarators so if
		// there is a pointer etc it'll default into an anonymous type
		// which may require precedence booting. 
		if( auto agent = Agent::TryAsAgentConst(type) )
		{
			if( !dynamic_cast<const StandardAgent *>(agent) )
				return //"/*Agent*/"+
					   string(constant?"const ":"") + 
					   DoRenderPreserve(type, Syntax::Production::SPACE_SEP_TYPE, policy) + 
					   (declarator != "" ? " "+declarator : "");
		}
	}
	catch( Syntax::Refusal & ) {}
	
	auto type_as_type = TreePtr<CPPTree::Type>::DynamicCast(type);
	if( !type_as_type )
	{
		return //"/*Non-type*/" +	
			   string(constant?"const ":"") + 
			   DoRenderPreserve(type, Syntax::Production::SPACE_SEP_TYPE, policy) +
			   (declarator != "" ? " "+declarator : "");
	}
	
	try 
	{ 
		return type_as_type->GetRenderTypeAndDeclarator( this, declarator, declarator_prod, surround_prod, policy, constant ); 
	}
	catch( Syntax::Refusal & ) 
	{
		// This part duplicates Type::GetRenderTypeAndDeclarator
		// We wouln't want to call a virtual on a NULL pointer, so the
		// common part could be here or a static on Type
		return //"/*GRTaD() fail*/" +	
			   string(constant?"const ":"") + 
			   DoRenderPreserve(type, Syntax::Production::SPACE_SEP_TYPE, policy) +
			   (declarator != "" ? " "+declarator : "");
	}	
}                                          
               

// We will deal with NULL, Agents and nodes that refuse to render given surround_prod and policy
Syntax::Production Render::GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const 
{
	if( !node )
		return Syntax::Production::NULLPTR;
		
	try
	{
		if( const Agent *agent = Agent::TryAsAgentConst(node) )
			return agent->GetAgentProduction();
	}
	catch( Syntax::Refusal & ) 
	{
		// Still might work a regular node so fall through
	}	

	try
	{
		// A lot of nodes have GetMyProduction() but not GetRender(). If GetRender() is not
		// implemented, we'll generate explicit (⯁) form, which is EXPLICIT_NODE.
		// Passing in the real renderer would cause unwanted side-effects.
		struct FakeRenderer : RendererInterface
		{
			string DoRender( const TreePtrInterface *, 
					 		 Syntax::Production, 
							 Syntax::Policy ) final { return "fake"; } 
			string DoRenderPreserve( TreePtr<Node>, 
									 Syntax::Production, 
									 Syntax::Policy ) final { return "fake"; } 
			string DoRenderTypeAndDeclarator( const TreePtrInterface *, string, 
											  Syntax::Production, Syntax::Production, Syntax::Policy,
											  bool ) final { return "fake"; }
			string DoRenderTypeAndDeclaratorPreserve ( TreePtr<Node>, string, 
									         		   Syntax::Production, Syntax::Production, Syntax::Policy,
											           bool ) final { return "fake"; }
			string RenderScopeResolvingPrefix( TreePtr<Node>, Syntax::Policy ) final { return ""; } 
			string GetUniqueIdentifierName( TreePtr<Node> ) const final { return "fake"; }
			string RenderNodeExplicit( shared_ptr<const Node>, 
									   Syntax::Production, 
									   Syntax::Policy ) final { return "fake"; } 	
			TreePtr<Node> TryGetScope( TreePtr<Node> ) const final { return nullptr; }
			const TransKit *GetTransKit() const override { return nullptr; }
		} fake_renderer;

		// Can it render?
		(void)surround_prod;
		(void) node->GetRender(&fake_renderer, surround_prod, policy); 
		
		return node->GetMyProduction(this, policy); 
	}
	catch( Syntax::Refusal & ) 
	{
		// Out of ideas so it will have to render explicitly
		return Syntax::Production::EXPLICIT_NODE;
	}	    
}


TreePtr<Node> Render::TryGetScope( TreePtr<Node> node ) const
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


const TransKit *Render::GetTransKit() const
{
	return &trans_kit;
}
