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

using namespace CPPTree; // TODO should not need
using namespace VN;


Render::Render( string of ) :
    outfile( of )
{
}


string Render::RenderToString( shared_ptr<CompareReplace> pattern )
{
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = pattern->GetSearchComparePattern(); 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;
    kit = RenderKit{ utils.get(),
		             bind(&Render::RenderIntoProduction, this, _1, _2)  };

    // Make the identifiers unique (does its own tree walk)
    unique_ids = UniquifyNames::UniquifyAll( kit, context, true );

	if( pattern->GetSearchComparePattern() == pattern->GetReplacePattern() )
		return "꩜" + kit.render( pattern->GetSearchComparePattern(), Syntax::Production::VN_PREFIX );
	else
        ASSERTFAIL();
}


string Render::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;
    kit = RenderKit{ utils.get(),
		             bind(&Render::RenderIntoProduction, this, _1, _2)  };

    // Make the identifiers unique (does its own tree walk)
    unique_ids = UniquifyNames::UniquifyAll( kit, context, false );

    return kit.render( root, Syntax::Production::PROGRAM );
}


void Render::WriteToFile( string s )
{
    if( outfile.empty() )
    {
        puts( s.c_str() );
    }
    else
    {
        FILE *fp = fopen( outfile.c_str(), "wt" );
        ASSERT( fp )( "Cannot open output file \"%s\"", outfile.c_str() );
        fputs( s.c_str(), fp );
        fclose( fp );
    }        
}


string Render::RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod )
{	
    INDENT("R");
    string s;
    
    if( !node )
		return RenderNullPointer( surround_prod );	
					
    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    Syntax::Production node_prod;
   	if( dynamic_cast<const SpecialBase *>(node.get()) )
		node_prod = Agent::TryAsAgentConst(node)->GetAgentProduction();
	else
		node_prod = node->GetMyProduction();
	if( ReadArgs::use.count("c") )
		s += SSPrintf("\n// Prefix \"%s\" surround: %d node: %d (%s) from %p\n", 
					 Tracer::GetPrefix().c_str(), 
					 Syntax::GetPrecedence(surround_prod), 
					 Syntax::GetPrecedence(node_prod),
					 Trace(node).c_str(), 
					 RETURN_ADDR() );		
					 
	// If we got a VN prefix, take no action. It will be handled when we reach something else.
	if( node_prod == Syntax::Production::VN_PREFIX )
		return s + Dispatch( node, surround_prod );
		
    bool do_boot = Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(surround_prod);    
    bool semicolon = Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::CONDITION) &&
                     Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::PROTOTYPE);  
    bool do_init = surround_prod == Syntax::Production::INITIALISER;

    if( ReadArgs::use.count("c") )
		s += SSPrintf("\n// Boot: %s semcolon: %s init: %s\n", 
					 do_boot ? "yes" : "no",
					 semicolon ? "yes" : "no",
					 do_init ? "yes" : "no" );
           
    switch(node_prod)
    {
        case Syntax::Production::BOOT_VN...Syntax::Production::TOP_VN: // Expression productions at different precedences
        {
            // If current production has too-high precedence, boot back down using parentheses
            if( do_boot )
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
					  ("VN-braces won't achieve high enough precedence for surrounding production\n")
					  ("Node: ")(node)("\n")
					  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			// Deal with expression in initialiser production by prepending =
			ASSERT( !do_init );
			ASSERT( !semicolon );
            if( do_boot )
                s += "⦑";//+SSPrintf("/*Surr: %d, node: %d*/", (int)surround_prod, (int)node_prod);

			if( do_boot )
				surround_prod = Syntax::Production::BOOT_VN;
			s += Dispatch( node, surround_prod );

            if( do_boot )
                s += "⦒";            
            break;
        }

        case Syntax::Production::BOOT_STMT_DECL...Syntax::Production::TOP_STMT_DECL: // Statement productions at different precedences
        {
            // If current production has too-high precedence, boot back down using braces
            if( do_boot )
            {
				// Braces can actually work in expressions, eg in {}. The nodes are STATEMENT_SEQ and we boot to BOOT_STMT_DECL
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACED) ||			
				        Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::TOP_STMT_DECL) )
					  ("Braces won't achieve high enough precedence for surrounding statement production\n")
                      ("Node: ")(node)("\n")
                      ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
					  ("Braces won't achieve high enough precedence for surrounding expressional or higher production\n")
                      ("Node: ")(node)("\n")
                      ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
			}
            if( do_boot )
                s += "{ ";

            if( do_boot || semicolon )
				surround_prod = Syntax::Production::BOOT_STMT_DECL;
			s += Dispatch( node, surround_prod );
			
            if( semicolon )
                s += "; ";
            if( do_boot )
                s += "} ";            
            break;
        }

        case Syntax::Production::BOOT_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences
        {
            // If current production has too-high precedence, boot back down using parentheses
            if( do_boot )
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
					  ("Parentheses won't achieve high enough precedence for surrounding production\n")
					  ("Node: ")(node)("\n")
					  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			// Deal with expression in initialiser production by prepending =
			if( do_init )
				s += " = ";
            if( do_boot )
                s += "(";

			if( do_boot )
				surround_prod = Syntax::Production::BOOT_EXPR;
			else if( do_init )
				surround_prod = Syntax::Production::ASSIGN;
            else if( semicolon )
				surround_prod = Syntax::Production::BOOT_EXPR;
			s += Dispatch( node, surround_prod );

            if( do_boot )
                s += ")";            
            if( semicolon )
                s += "; ";
            break;
        }
        
        default: 
        {
            if( semicolon )
				surround_prod = Syntax::Production::BOOT_EXPR;
			s += Dispatch( node, surround_prod );
			
            if( semicolon )
                s += "; ";            
			break;
		}
    }
    
    return s;
}


string Render::RenderNullPointer( Syntax::Production surround_prod )
{	
	(void)kit;
	(void)surround_prod;
	// Assume NULL means we're in a pattern, and it represents a wildcard
	// Note: we'd better not supply NULL, or we'll recurse forever.
	return RenderIntoProduction( MakeTreeNode<Node>(), surround_prod );
}


string Render::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod )
{
	(void)surround_prod;
	if( dynamic_cast<const SpecialBase *>(node.get()) )
		return RenderSpecial( node, surround_prod ); 
	return RenderAny( node );       
}


string Render::RenderSpecial( TreePtr<Node> node, Syntax::Production surround_prod )
{
	const Agent *agent = Agent::TryAsAgentConst(node);
	return agent->GetRender( kit, surround_prod );
}


string Render::RenderAny( TreePtr<Node> node, unsigned enables )
{
	string s = node->GetRenderTerminal();
	if( !s.empty() )
		return s;
	
	s = "⁜" + TYPE_ID_NAME(*node);
	
	list<string> sitems;
    vector< Itemiser::Element * > items = node->Itemise();
    for( vector< Itemiser::Element * >::size_type i=0; i<items.size(); i++ )
    {
		if( (enables & (1U << i))==0 )
			continue;
			
        //TRACE("Duplicating member %d\n", i );
        ASSERT( items[i] )( "itemise returned null element" );
        
        if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(items[i]) )                
        {
			list<string> scon;
            for( const TreePtrInterface &p : *con )
            {
                ASSERT( p ); // present simplified scheme disallows nullptr
                scon.push_back( RenderIntoProduction( TreePtr<Node>(p), Syntax::Production::VN_SEP ) );
            }
            sitems.push_back( Join( scon, "︙ ") );
        }            
        else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(items[i]) )
        {
            sitems.push_back( RenderIntoProduction( TreePtr<Node>(*singular), Syntax::BoostPrecedence( Syntax::Production::VN_SEP ) ) );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
        
    s += Join( sitems, "┆ ", "⦑", "⦒" ); 
    
    // We can't change the production returned by GetMyProduction(), so try instead to render in 
    // accordance with whatever the node returned.
    if( Syntax::GetPrecedence(node->GetMyProduction()) < Syntax::GetPrecedence(Syntax::Production::CONDITION) &&
        Syntax::GetPrecedence(node->GetMyProduction()) >= Syntax::GetPrecedence(Syntax::Production::BOOT_STMT_DECL))
		s += "; "; // Statement-ize it

	return s;
}


TreePtr<Scope> Render::TryGetScope( TreePtr<Identifier> id )
{
    if( scope_stack.empty() ) 
        return nullptr; // We aren't even in any scopes

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
        DeclarationOf().TryApplyTransformation( kit, id );
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
