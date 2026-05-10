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
#include "vn/lang/sort_decls.hpp"
#include "vn/agents/agent.hpp"
#include "clang/Parse/DeclSpec.h"
#include "vn/lang/uniquify_identifiers.hpp"
#include "cpprender.hpp"

using namespace std;
using namespace CPPTree;

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    ( string( "\n❌" ) + \
      string( V ) + \
      string( " not supported in " ) + \
      string( __func__ ) + \
      string( "()❌\n" ) )

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? TYPE_ID_NAME(*P) : "<nullptr>" )

// Don't like the layout of rendered code?
// There's a .clang-format in repo root.


// For #400 make methods that return strings try-functions
// and use this for the catch clause.
#define DEFAULT_CATCH_CLAUSE \
    catch( const ::Mismatch &e ) \
    { \
        return RenderMismatchException( __func__, e ); \
    } 


CppRender::CppRender( string of ) :
	Render( GetDefaultPolicy(), of ) // default policy when rendering to pure C++
{
}


string CppRender::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
    utils = make_unique<DefaultTransUtils>(context);
    trans_kit = TransKit{ utils.get() };
            
    using namespace placeholders;

    // Make the identifiers unique (does its own tree walk). Be strict about undeclared
    // identifiers - to rename them would be unsafe because we assume there's
    // a declaration ouside of our tree. This actually gets other nodes too but 
    // we'll only look up identifiers.
    UniquifyNames::Policy un_policy {
	    .name_getter = &Syntax::GetIdentifierName, 
		.include_single_parent = true,
		.include_multi_parent = true,
		.include_designation_named_identifiers = false,
		.preserve_undeclared_ids = true
	};
	
	UniquifyNames identifiers_uniqifier(un_policy); 

    unique_identifier_names = identifiers_uniqifier.UniquifyAll( trans_kit, context );
    
    Syntax::Policy top_policy = default_policy;
    
    nodes_not_rendered_to_c = 0;
    string s = DoRender( &root, Syntax::Production::PROGRAM, top_policy );
    if( nodes_not_rendered_to_c > 0 )
        s = SSPrintf("#error %d nodes could not be rendered to C\n", nodes_not_rendered_to_c) + s;
    return s;
}	


Syntax::Policy CppRender::GetDefaultPolicy()
{
	Syntax::Policy policy;
	
	// Insert braces to disambiguate stratements eg in case of if/else ambiguity
	policy.boot_statements_using_braces = true;
	
	// This can be necessary to resolve dependencies or at least produce readable code
	policy.can_split_instances = true;
		
	// Render constructors and destructors per C++ standard
	policy.use_vn_xstructor_symbol = false;
	
	// Just use type ids directly (aka abstract declarators)
	policy.disambiguate_type_id = false;

	// Allow invisible things to be rendered as empty strings
	policy.refuse_invisibles = false;

	return policy;
}


Syntax::Production CppRender::GetNodeProduction( TreePtr<Node> node, Syntax::Policy policy ) const
{
	try
	{
		return node->GetMyProduction(this, policy);       
	}
	catch( Syntax::Refusal &r )
	{
		ASSERT(false)(node)(" unknown production");
	}
	ASSERTFAIL();
}


string CppRender::OnRefusal( Syntax::Refusal &ex, TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{
	// If render was unsuccessful, TRY AGAIN but this time with node_prod set to 
	// EXPLICIT_NODE which means the render will be explicit (i.e. with ⯁) and
	// won't throw.
	nodes_not_rendered_to_c++;
	return AccomodateInit(node, Syntax::Production::EXPLICIT_NODE, surround_prod, policy) + " // " + ex.What() + "\n"; 
}


string CppRender::GetUniqueIdentifierName( TreePtr<Node> id ) const 
{
	ASSERT( unique_identifier_names.count(id) > 0 )
	      (id)
	      (" (\"%s\") missing from unique_identifier_names", id->GetIdentifierName().c_str() );
	return unique_identifier_names.at(id);
}


string CppRender::DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                             Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                             TreePtr<CPPTree::Constancy> constant )
{ 		
	auto type_as_type = TreePtr<CPPTree::Type>::DynamicCast(type);
	ASSERT( type_as_type )(type)(" needs to be a Type"); // Must supply a type

	return type_as_type->GetRenderTypeAndDeclarator( this, declarator, declarator_prod, surround_prod, policy, constant );		
}

