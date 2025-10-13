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

using namespace CPPTree; // TODO should not need
using namespace VN;


Render::Render( string of ) :
    outfile( of )
{
}


string Render::RenderToString( shared_ptr<CompareReplace> pattern )
{
    return RenderToString( pattern->GetSearchComparePattern() ) +
           "\n꩜\n" +
           RenderToString( pattern->GetReplacePattern() );
}


string Render::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    DefaultTransUtils utils(context);
    Render::Kit kit { &utils };

    // Make the identifiers unique (does its own tree walk)
    unique_ids = UniquifyIdentifiers::UniquifyAll( kit, context );

    return RenderIntoProduction( kit, root, Syntax::Production::PROGRAM );
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


string Render::RenderIntoProduction( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod )
{	
    INDENT("R");
    if( !node )
		return RenderNullPointer( kit, surround_prod );
		
    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    string s, ss;
    Syntax::Production node_prod = node->GetMyProduction();
    bool do_boot = Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(surround_prod);    
    bool semicolon = Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::CONDITION) &&
                     Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::PROTOTYPE);  
    bool do_init = surround_prod == Syntax::Production::INITIALISER;
    if( ReadArgs::use.count("c") )
		s += SSPrintf("\n// %s Surround %d node %d (%s) from %p boot: %s semcolon: %s init: %s\n", 
					 Tracer::GetPrefix().c_str(), 
					 Syntax::GetPrecedence(surround_prod), 
					 Syntax::GetPrecedence(node_prod),
					 Trace(node).c_str(), 
					 RETURN_ADDR(),
					 do_boot ? "yes" : "no",
					 semicolon ? "yes" : "no",
					 do_init ? "yes" : "no" );
           
    switch(node_prod)
    {
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
                s += "{\n";

            if( do_boot || semicolon )
				surround_prod = Syntax::Production::BOOT_STMT_DECL;
			s += Dispatch( kit, node, surround_prod );
			
            if( semicolon )
                s += ";\n";
            if( do_boot )
                s += "}\n";            
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
			s += Dispatch( kit, node, surround_prod );

            if( do_boot )
                s += ")";            
            if( semicolon )
                s += ";\n";
            break;
        }
        
        default: 
        {
            if( semicolon )
				surround_prod = Syntax::Production::BOOT_EXPR;
			s += Dispatch( kit, node, surround_prod );
			
            if( semicolon )
                s += ";\n";            
			break;
		}
    }
    
    return s;
}


string Render::RenderNullPointer( const Render::Kit &kit, Syntax::Production surround_prod )
{	
	// Assume NULL means we're in a pattern, and it represents a wildcard
	// Note: we'd better not supply NULL, or we'll recurse forever.
	return RenderIntoProduction( kit, MakeTreeNode<Node>(), surround_prod );
}


string Render::Dispatch( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod )
{
	(void)surround_prod;
	return RenderAny( kit, node );       
}


string Render::RenderAny( const Render::Kit &kit, TreePtr<Node> node, unsigned enables )
{
	string s = node->GetRender();
	if( !s.empty() )
		return s;
	
	s = "🞊"+TYPE_ID_NAME(*node);
	
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
                scon.push_back( RenderIntoProduction( kit, TreePtr<Node>(p), Syntax::Production::COMMA_SEP ) );
            }
            sitems.push_back( Join( scon, ", ") );
        }            
        else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(items[i]) )
        {
            sitems.push_back( RenderIntoProduction( kit, TreePtr<Node>(*singular), Syntax::Production::BARE_STATEMENT ) );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
        
    s += Join( sitems, "; ", "⦑", "⦒" ); 
    
    // We can't change the production returned by GetMyProduction(), so try instead to render in 
    // accordance with whatever the node returned.
    if( Syntax::GetPrecedence(node->GetMyProduction()) < Syntax::GetPrecedence(Syntax::Production::CONDITION) &&
        Syntax::GetPrecedence(node->GetMyProduction()) >= Syntax::GetPrecedence(Syntax::Production::BOOT_STMT_DECL))
		s += ";\n"; // Statement-ize it

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
        // There is a scope but our id us not in it, maybe it was undeclared?
        return nullptr;
    }
}


bool Render::IsDeclared( const Render::Kit &kit, TreePtr<Identifier> id )
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
    return "«"+fname+"() error: "+me.What()+"»";
}
