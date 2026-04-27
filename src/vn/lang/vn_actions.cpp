#include "vn_actions.hpp"

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
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include "vn/agents/all.hpp"
#include "tree/node_names.hpp"
#include "vn_commands.hpp"
#include "vn_recogniser.hpp"

#include <iostream>
#include <fstream>
#include <cctype>
#include <typeinfo>

//using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

static NodeEnum GetNodeEnum( list<string> typ, any loc );


VNLangActions::VNLangActions() :
	node_names( make_unique<AvailableNodeData>() )
{
}

VNLangActions::~VNLangActions()
{
	// Keep me: compiler can see complete VNLangScanner and VNLangParser here
}


void VNLangActions::OnVNScript( Command::List top_level_commands_ )
{
	top_level_commands = top_level_commands_;
}


TreePtr<Node> VNLangActions::OnStar( TreePtr<Node> restriction )
{
	auto node = MakeTreeNode<StarAgent>();
	node->restriction = restriction;
	return node;
}


TreePtr<Node> VNLangActions::OnBuildSize( TreePtr<Node> container )
{
	auto node = MakeTreeNode<BuildContainerSizeAgent>();
	node->container = container;
	return node;
}


TreePtr<Node> VNLangActions::OnStuff( TreePtr<Node> terminus, TreePtr<Node> recurse_restriction, Limit limit )
{
	string note = "\nNote: Only 《=1... as a depth condition is supported at present (TODO).";
	if( limit.cond.empty() )
	{
		auto stuff = MakeTreeNode<StuffAgent>();
		stuff->terminus = terminus;
		stuff->recurse_restriction = recurse_restriction;
		return stuff;
	}
	else if( limit.cond=="=" )
	{
		unsigned num = stoul(limit.num_text);
		if( num == 1 )
		{
			auto child = MakeTreeNode<ChildAgent>();
			child->terminus = terminus;
			return child;
		}
		else
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(limit.num_loc), 
				"⩨ depth value " + limit.num_text + " not supported." + note);		
		
	}
	else
		throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(limit.cond_loc), 
			"⩨ depth condition " + DiagQuote(limit.cond) + " not supported." + note);		
}


TreePtr<Node> VNLangActions::OnDelta( TreePtr<Node> through, TreePtr<Node> overlay )
{
	auto delta = MakeTreeNode<DeltaAgent>();
	delta->through = through;
	delta->overlay = overlay;
	return delta;
}


TreePtr<Node> VNLangActions::OnEmbeddedCommands( list<shared_ptr<Command>> commands )
{
	TreePtr<Node> node;
	TRACE("Decaying embedded commands: ")(commands)("\n");
	for( shared_ptr<Command> c : commands )
	{
		node = c->DecayToPattern(node, this); // node can be NULL for singular wildcard
		// TODO could generalise to things that can Execute from within SCREngine
	}
	return node;
}


static TreePtr<Node> MakeStandardAgent(NodeEnum ne)
{
	switch(ne)
	{
#define NODE(NS, NAME) \
	case NodeEnum::NS##_##NAME: \
		return MakeTreeNode<StandardAgentWrapper<NS::NAME>>(); 
#include "tree/node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "tree/operator_data.inc"
#undef NODE
	}
	
	// By design we should have a case for every value of the node enum
	ASSERT(false)("Invalid value for node enum value %d", ne); 
	ASSERTFAIL();
}


static TreePtr<Node> MakeStandardAgentFromTypeID(const type_info &ti)
{
#define NODE(NS, NAME) \
	if( ti == typeid(NS::NAME) ) \
		return MakeTreeNode<StandardAgentWrapper<NS::NAME>>(); \
	else
#include "tree/node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "tree/operator_data.inc"
#undef NODE
		
		// By design we should have a case for every value of the node enum
		ASSERT(false)("Could not find node for type info: ")(Traceable::CPPFilt(ti.name()));  // be the last else clause
		
	ASSERTFAIL();
}


TreePtr<Node> VNLangActions::OnExplicitNode( const AvailableNodeData::Block *block, any node_name_loc, Itemisation src_itemisation )
{
	auto leaf_block = dynamic_cast<const AvailableNodeData::NodeBlock *>(block);
	NodeEnum ne = leaf_block->node_enum.value();
	TreePtr<Node> dest = MakeStandardAgent(ne);
    YY::VNLangParser::location_type prev_loc = any_cast<YY::VNLangParser::location_type>(src_itemisation.loc);
	
	// Special case for specific identifiers
	if( auto dest_sid = TreePtr<CPPTree::SpecificIdentifier>::DynamicCast( dest ) )
	{
		if( src_itemisation.items.size() != 1  )
			throw YY::VNLangParser::syntax_error( 
				prev_loc,
				SSPrintf("%s expects a string, but %d items given.",
				         DiagQuote(Traceable::TypeIdName( *dest )).c_str(),
				         src_itemisation.items.size()));
		if( src_itemisation.items.front().nodes.size() != 1 )
			throw YY::VNLangParser::syntax_error( 
				prev_loc,
				SSPrintf("%s expects a string, but %d nodes given.",
				         DiagQuote(Traceable::TypeIdName( *dest )).c_str(),
				         src_itemisation.items.front().nodes.size()));
		auto string_node = TreePtr<CPPTree::String>::DynamicCast(src_itemisation.items.front().nodes.front());
		dest_sid->name = string_node->GetString();
		return dest_sid;
	}		
	
	// The detination's itemisation "pulls" items from the source and we require a match (for now). 
	// Then for containers, the source will "push" as many elements as it has, or just one
	// for singular items.
	list<Item>::const_iterator src_it = src_itemisation.items.begin();
    vector< Itemiser::Element * > dest_items = dest->Itemise();   
    
    // Allow an empty itemisation to masquerade as one item being an empty container
    if( src_itemisation.items.empty() && // nothing given
		dest_items.size() == 1 && // Expecting one item
		dynamic_cast<ContainerInterface *>(dest_items.front()) ) // Expected item is a container
		return dest; // It's all good, just leave the container empty
    
    string counts_msg = SSPrintf("%s expects %d %s, but %d %s given.",
                             DiagQuote(Traceable::TypeIdName( *dest )).c_str(),
                             dest_items.size(),
                             dest_items.size()==1?"item":"items",
                             src_itemisation.items.size(),
                             src_itemisation.items.size()==1?"was":"were");
    for( Itemiser::Element *dest_item : dest_items )
    {
		if( src_it == src_itemisation.items.end() )
			throw YY::VNLangParser::syntax_error( 
				prev_loc, 
				"In ⯁, insufficient items given. " + counts_msg );
		const Item &src_item = *src_it;
        if( SequenceInterface *dest_seq = dynamic_cast<SequenceInterface *>(dest_item) ) // TODO could roll together as Container?
            for( TreePtr<Node> src : src_item.nodes )
				dest_seq->insert( src );				
        else if( CollectionInterface *dest_col = dynamic_cast<CollectionInterface *>(dest_item) )
            for( TreePtr<Node> src : src_item.nodes )
				dest_col->insert( src );	
        else if( TreePtrInterface *dest_sing = dynamic_cast<TreePtrInterface *>(dest_item) )
        {
			if( src_item.nodes.size() == 1 )
			{
				*dest_sing = src_item.nodes.front();
			}
			else if( auto optional_arch = TreePtr<CPPTree::Qualifier>::DynamicCast(dest_sing->MakeValueArchetype()) )
			{
				if( src_item.nodes.empty() )
					*dest_sing = optional_arch->GetDefaultNode(nullptr);
				else
					throw YY::VNLangParser::syntax_error( 
						any_cast<YY::VNLangParser::location_type>(src_item.loc),
						SSPrintf("In ⯁, optional singular item requires zero or one sub-pattern but %d were given.",
						src_item.nodes.size() ) ); 
			}
			else
			{
				throw YY::VNLangParser::syntax_error( 
					any_cast<YY::VNLangParser::location_type>(src_item.loc),
					SSPrintf("In ⯁, non-optional singular item requires exactly one sub-pattern but %d were given.",
					src_item.nodes.size() ) ); 
			}
            
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
        src_it++;
        prev_loc = any_cast<YY::VNLangParser::location_type>(src_item.loc);
    }
		                             
	if( src_it != src_itemisation.items.end() )
		throw YY::VNLangParser::syntax_error( 
			any_cast<YY::VNLangParser::location_type>(src_it->loc), 
			"In ⯁, excess items given. " + counts_msg);

	return dest;
}


TreePtr<Node> VNLangActions::OnRestrict( const AvailableNodeData::Block *block, any node_name_loc, TreePtr<Node> target, any target_loc )
{
	auto node_block = dynamic_cast<const AvailableNodeData::NodeBlock *>(block);
	NodeEnum ne = node_block->node_enum.value();
	Agent *agent = Agent::TryAsAgent(target);
	ASSERT( agent )("We are parsing a pattern so everything should be agents");
		
	auto pspecial = dynamic_cast<SpecialBase *>(agent);
	if( pspecial )
	{
		pspecial->pre_restriction_archetype_node = node_names->MakeNode(ne);
		pspecial->pre_restriction_archetype_ptr = node_names->MakeTreePtr(ne);
	}

	return target;
}


TreePtr<Node> VNLangActions::OnTypeSpecifierSeq( multiset<string> specifiers, any loc )
{
	ASSERT( specifiers.size() >= 1 ); // this would be a bug in the parser
	TreePtr<CPPTree::Type> type;
	TreePtr<CPPTree::SpecificFloatSemantics> float_sem;
	int width_bits = 0;
	bool is_signed = true;
	if( specifiers.extract("void") )
		type = MakeTreeNode<StandardAgentWrapper<CPPTree::Void>>();
	else if( specifiers.extract("bool") )
		type = MakeTreeNode<StandardAgentWrapper<CPPTree::Boolean>>();
	else if( specifiers.extract("float") )
		float_sem = MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificFloatSemantics>>(TypeDb::float_semantics);
	else if( specifiers.extract("double") )
	{
		if( specifiers.extract("long") )
			float_sem = MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificFloatSemantics>>(TypeDb::long_double_semantics);
		else
			float_sem = MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificFloatSemantics>>(TypeDb::double_semantics);
	}
	else if( specifiers.extract("char") )	
	{
		is_signed = TypeDb::char_default_signed; // start with the default signedness for char
		width_bits = TypeDb::char_bits; 
	}
	else // try for an integral type based on int
	{
		is_signed = true; // int-based types are signed by default
		if( specifiers.extract("int") )			
			width_bits = TypeDb::integral_bits[0]; // int
								
		if( specifiers.extract("short") )	
			width_bits = TypeDb::integral_bits[1]; // short
		else if( specifiers.extract("long") )	
		{
			width_bits = TypeDb::integral_bits[2]; // long
			if( specifiers.extract("long") )	// second long
				width_bits = TypeDb::integral_bits[3]; // long long
		}
	}		
		
	if( width_bits > 0 )
	{
		if( specifiers.extract("signed") )	
			is_signed = true;
		else if( specifiers.extract("unsigned") )	
			is_signed = false;
			
	    TreePtr<CPPTree::Integral> i;
		if (is_signed)
			i = MakeTreeNode<StandardAgentWrapper<CPPTree::Signed>>();
		else
			i = MakeTreeNode<StandardAgentWrapper<CPPTree::Unsigned>>();

		i->width = MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificInteger>>( width_bits );
		type = i;
	}
	else if( float_sem )
	{
		auto f = MakeTreeNode<StandardAgentWrapper<CPPTree::Floating>>();
		f->semantics = float_sem;
		type = f;
	}

	if( !type )
		throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc), 
			"Unknown type specifier sequence: "+Trace(specifiers));	
			
	// Should have "gobbled up" all the specifiers using extract()
	if( specifiers.size() > 0 )
		throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc), 
			"Unknown additional type specifiers: "+Trace(specifiers));	
	
	return type;
}


TreePtr<Node> VNLangActions::OnPrefixOperator( string tok, TreePtr<Node> operand )
{
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) \
    if( tok==TEXT ) \
    { \
		auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::NAME>>(); \
		node->operands.push_back(operand); \
        return node; \
	}
#include "tree/operator_data.inc"
	{
		ASSERTFAIL(); // TODO use operator_data.inc
	}
}


TreePtr<Node> VNLangActions::OnPostfixOperator( string tok, TreePtr<Node> operand )
{
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) \
    if( tok==TEXT ) \
    { \
		auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::NAME>>(); \
		node->operands.push_back(operand); \
        return node; \
	}
#include "tree/operator_data.inc"
	{
		ASSERTFAIL(); // TODO use operator_data.inc
	}
}


TreePtr<Node> VNLangActions::OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right )
{
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) \
    if( tok==TEXT ) \
    { \
		auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::NAME>>(); \
		node->operands.push_back(left); \
		node->operands.push_back(right); \
        return node; \
	}
#include "tree/operator_data.inc"
	{
		ASSERTFAIL(); // TODO use operator_data.inc
	}
}


TreePtr<Node> VNLangActions::OnIntegralLiteral( string text, any loc )
{
	// Normalise to upper case
    transform(text.begin(), text.end(), text.begin(), ::toupper);
    
    bool uns = text.find("U") != std::string::npos;
    bool lng2 = text.find("LL") != std::string::npos;
    bool lng = (text.find("L") != std::string::npos) && !lng2;
    
    return CreateIntegralLiteral(uns, lng2, lng, stoull(text), loc);
}


TreePtr<Node> VNLangActions::OnStringLiteral( wstring wvalue )
{
	string value = Unquote(ToASCII(wvalue));
	return MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificString>>(value);
}


TreePtr<Node> VNLangActions::OnBoolLiteral( bool value )
{
	if( value )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::True>>();
	else
		return MakeTreeNode<StandardAgentWrapper<CPPTree::False>>();
}


TreePtr<Node> VNLangActions::OnCast( TreePtr<Node> type, any type_loc, TreePtr<Node> operand, any operand_loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Cast>>();
	node->operand = operand;	
	node->type = type;
	return node;
}


TreePtr<Node> VNLangActions::OnConditionalOperator( TreePtr<Node> condition, TreePtr<Node> expr_then, TreePtr<Node> expr_else )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::ConditionalOperator>>();
	node->condition = condition;	
	node->expr_then = expr_then;
	node->expr_else = expr_else;
	return node;
}
	

TreePtr<Node> VNLangActions::OnSubscript( TreePtr<Node> destination, TreePtr<Node> index )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Subscript>>();
	node->destination = destination;	
	node->index = index;
	return node;
}
	

TreePtr<Node> VNLangActions::OnCompound( list<TreePtr<Node>> members, any memb_loc, list<TreePtr<Node>> statements, any stmt_loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Compound>>();
	for( TreePtr<Node> member : members )
		node->members.insert(member);
	for( TreePtr<Node> statement : statements )
		node->statements.insert(statement);
	return node;
}


TreePtr<Node> VNLangActions::OnCodeUnit( list<TreePtr<Node>> members, any memb_loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::CodeUnit>>();
	for( TreePtr<Node> member : members )
		node->members.insert(member);
	return node;
}


TreePtr<Node> VNLangActions::OnArrayInitialiser( list<TreePtr<Node>> elements )
{
	// Fill out the array literal
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::ArrayInitialiser>>();
	for( TreePtr<Node> value : elements )
		node->elements.insert( value );
	return node;
}


TreePtr<Node> VNLangActions::OnLabel( TreePtr<Node> identifier, any loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::LabelDeclaration>>();
	node->identifier = identifier;
	return node;
}


TreePtr<Node> VNLangActions::OnCase( TreePtr<Node> value, any loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Case>>();
	node->value = value;
	return node;
}


TreePtr<Node> VNLangActions::OnRangeCase( TreePtr<Node> value_lo, any lo_loc, TreePtr<Node> value_hi, any hi_loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::RangeCase>>();
	node->value_lo = value_lo;
	node->value_hi = value_hi;
	return node;
}


TreePtr<Node> VNLangActions::OnDefault( any loc )
{
	return MakeTreeNode<StandardAgentWrapper<CPPTree::Default>>();
}


TreePtr<Node> VNLangActions::OnNop( any loc )
{
	return MakeTreeNode<StandardAgentWrapper<CPPTree::Nop>>();
}


TreePtr<Node> VNLangActions::OnFuncOnType( string keyword, any keyword_loc, TreePtr<Node> type_arg, any type_arg_loc )
{
	TreePtr<CPPTree::TemplateExpression> node;
	if( keyword=="sizeof" )
		node = MakeTreeNode<StandardAgentWrapper<CPPTree::SizeOf>>();
	else if( keyword=="alignof" )
		node = MakeTreeNode<StandardAgentWrapper<CPPTree::AlignOf>>();
	else
		ASSERTFAIL();
		
	node->argument = type_arg;
	
	return node;
}


TreePtr<Node> VNLangActions::OnNormalTerminalKeyword( string keyword, any keyword_loc )
{
	if( keyword=="this" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::This>>();
	
	ASSERTFAIL();
}

TreePtr<Node> VNLangActions::OnSimpleStmt( string keyword, any keyword_loc)
{
	if( keyword=="break" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Break>>();
	else if( keyword=="continue" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Continue>>();
	
	ASSERTFAIL(); // internal error: parser should not call this unless recogniser produced PRIMITIVE_STMT_KEYWORD		
}


TreePtr<Node> VNLangActions::OnSpaceSepStmt( string keyword, any keyword_loc, TreePtr<Node> operand, any operand_loc )
{
	if( keyword=="return" )
	{
		auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Return>>();
		ret->return_value = operand;
		return ret;
	}
	else if( keyword=="goto" )
	{
		auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Goto>>();
		ret->destination = operand;		
		return ret;
	}
	
	ASSERTFAIL(); // internal error: parser should not call this unless recogniser produced SPACE_SEP_STMT_KEYWORD		
}

TreePtr<Node> VNLangActions::OnArgsBodyStmt( string keyword, any keyword_loc, list<TreePtr<Node>> args, any args_loc, TreePtr<Node> body, any body_loc )
{
	if( keyword=="switch" )
	{
		if( args.size() != 1 )
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(args_loc),
				keyword + " requires 1 argument inside ().");
				
		auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Switch>>();
		ret->condition = args.front();
		ret->body = body;
		return ret;
	}
	else if( keyword=="for" )
	{
		if( args.size() != 3 )
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(args_loc),
				keyword + " requires 3 argument inside ().");
				
		auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::For>>();
		auto it = args.begin();
		ret->initialisation = *it++;
		ret->condition = *it++;
		ret->increment = *it++;
		ret->body = body;
		return ret;
	}
	
	ASSERTFAIL(); // internal error: parser should not call this unless recogniser produced ARGS_BODY_STMT_KEYWORD		
}


TreePtr<Node> VNLangActions::OnArgsBodyChainStmt( string keyword, any keyword_loc, 
												  list<TreePtr<Node>> args, any args_loc, 
												  TreePtr<Node> body, any body_loc,
												  string chain_keyword, any chain_keyword_loc, 
												  TreePtr<Node> chain_body, any chain_body_loc )
{
	if( keyword=="if" )
	{
		if( args.size() != 1 )
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(args_loc),
				keyword + " requires 1 argument inside ().");
				
		auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::If>>();
		ret->condition = args.front();
		ret->body = body;
		if( chain_keyword=="else" )		
			ret->body_else = chain_body;
		else if( chain_keyword=="" )		
			ret->body_else = MakeTreeNode<StandardAgentWrapper<CPPTree::Nop>>();
		else
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(chain_keyword_loc),
				keyword + " requires else (optional).");
		return ret;
	}
	ASSERTFAIL(); // internal error: parser should not call this unless recogniser produced ARGS_BODY_STMT_KEYWORD		
}


TreePtr<Node> VNLangActions::OnWhile( TreePtr<Node> arg, any arg_loc, TreePtr<Node> body, any body_loc )
{
	auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::While>>();
	ret->condition = arg;
	ret->body = body;
	return ret;
}


TreePtr<Node> VNLangActions::OnDo( TreePtr<Node> body, any body_loc, TreePtr<Node> arg, any arg_loc )
{
	auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Do>>();
	ret->condition = arg;
	ret->body = body;
	return ret;
}


TreePtr<Node> VNLangActions::OnFunction( TreePtr<Node> return_type, list<TreePtr<Node>> params )
{
	auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Function>>();
	for( auto p : params )
		ret->params.insert(p);
	ret->return_type = return_type;
	return ret;
}


TreePtr<Node> VNLangActions::OnConstructorType( list<TreePtr<Node>> params )
{
	auto ret = MakeTreeNode<StandardAgentWrapper<CPPTree::Constructor>>();
	for( auto p : params )
		ret->params.insert(p);
	return ret;
}


BlockAndGnomon VNLangActions::MakeScopeGnomonForNode( const AvailableNodeData::Block *block ) const
{
	auto nb = dynamic_cast<const AvailableNodeData::NodeBlock *>(block);
	ASSERT( nb );
	ASSERT( nb->node_enum );
	NodeEnum ne = nb->node_enum.value();
	// No need for StandardAgent or TreePtr, we're just analysing here
	shared_ptr<Node> node = node_names->MakeNode(ne);
	ASSERT( node );
	shared_ptr<Gnomon> gnomon;
	if( dynamic_pointer_cast<CPPTree::CodeUnit>(node) )
		gnomon = make_shared<GlobalScopeGnomon>();
	else if( dynamic_pointer_cast<CPPTree::SequentialScope>(node) )
		gnomon = make_shared<LocalScopeGnomon>();
	else if( dynamic_pointer_cast<CPPTree::Enum>(node) )
		gnomon = make_shared<EnumeratorScopeGnomon>();
	else if( auto record = dynamic_pointer_cast<CPPTree::Record>(node) )
	{
		TreePtr<Node> init_access = record->GetInitialAccess();
		ASSERT( init_access ); // Records must always specify an initial access
		gnomon = make_shared<FieldScopeGnomon>( MakeStandardAgentFromTypeID( typeid(*init_access) ) );
	}
	else if( dynamic_pointer_cast<CPPTree::CallableParams>(node) )
		gnomon = make_shared<ParameterScopeGnomon>();
	else
		gnomon = make_shared<UnknownScopeGnomon>("non-scope explicit node " + nb->What());	
		
	// DeclScope and Scope are too broad, as they apply to more than one of the above categories	
		
	return { block, gnomon };
}


void VNLangActions::OnAccessSpec( any loc, TreePtr<Node> access )
{
	// We'll create one of a range of final nodes, all subclassing Instance, based on the current scope for declarations
	shared_ptr<ScopeGnomon> spg = declaration_scope_gnomons.TryLockTop();	
	if( auto fspg = dynamic_cast<FieldScopeGnomon *>(spg.get()) ) 	
	{
		stringstream ss;
		ss << any_cast<YY::VNLangParser::location_type>(loc);
		FTRACE("Access spec for gnomon ")(fspg)(" becomes ")(access)(" at ")(ss.str())("\n");
		fspg->current_access = access;
	}
	else
		return; // Let the declaration produce an error message
}


TreePtr<Node> VNLangActions::OnInstance( any loc, const list<QualifierData> &quals_pre, TreePtr<Node> type, TreePtr<Node> declarator )
{	
	string note = 
		"\nNote: scope may be a surrounding code unit, compound, struct/class body,"
		"\nparams list, explicit scope node or pre-restriction to a declaration node type";			
	
	// TODO process the qualifiers in one loop at the top, with lots of checking. Check for:
	// - wrong qualifier eg an access spec
	// - duplication/conflict of qualifiers (<=1 in each category)
	bool static_ = false;
	for( const QualifierData &q : quals_pre )
		if( q.cat == QualCat::STATIC )
			static_ = true;

	// We'll create one of a range of final nodes, all subclassing Instance, based on the current scope for declarations
	shared_ptr<ScopeGnomon> spg = declaration_scope_gnomons.TryLockTop();	
	TreePtr<CPPTree::Instance> instance;
	if( static_ )
	{
		if( dynamic_cast<const ParameterScopeGnomon *>(spg.get()) )
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(loc),
				"static is not allowed for parameters.");
		if( dynamic_cast<const GlobalScopeGnomon *>(spg.get()) )
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(loc),
				"static is not supported at code unit level.");
		// Remaining scopes are field and local. VN is a resolved form of C/C++
		// so we don't need to constrain scope. We can just call these global 
		// if static was specified.
		instance = MakeTreeNode<StandardAgentWrapper<CPPTree::Global>>(); 
	}	
	else if( !spg ) 
		throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Cannot disambiguate declaration because no surrounding scope." + note );
	else if( auto usg = dynamic_cast<const UnknownScopeGnomon *>(spg.get()) ) 
		throw YY::VNLangParser::syntax_error(
			any_cast<YY::VNLangParser::location_type>(loc),
			"Cannot disambiguate declaration under " + usg->reason + "." + note );
	else if( auto psg = dynamic_cast<const PrerestrictScopeGnomon *>(spg.get()) ) 
	{
		auto nb = dynamic_cast<const AvailableNodeData::NodeBlock *>(psg->block);
		ASSERT( nb );
		ASSERT( nb->node_enum );
		NodeEnum ne = nb->node_enum.value();
		TreePtr<Node> node = MakeStandardAgent(ne);
		ASSERT( node );
		instance = TreePtr<CPPTree::Instance>::DynamicCast(node);
		if( !instance )
			throw YY::VNLangParser::syntax_error(
						any_cast<YY::VNLangParser::location_type>(loc),
						"nearest prerestrict " + nb->What() + " cannot disambiguate an instance declaration" + note); // TODO it could if the pre-restriction was to eg a Record etc			
	}
	else if( dynamic_cast<const ParameterScopeGnomon *>(spg.get()) )
		instance = MakeTreeNode<StandardAgentWrapper<CPPTree::Parameter>>();
	else if( auto fspg = dynamic_cast<FieldScopeGnomon *>(spg.get()) )
	{ 
		auto field = MakeTreeNode<StandardAgentWrapper<CPPTree::Field>>();
		for( const QualifierData &q : quals_pre )
		{
			if( q.cat == QualCat::NODE )
			{
				if( auto vq = TreePtr<CPPTree::Virtuality>::DynamicCast(q.node) )
					field->virt = vq;
					
				if( auto aq = TreePtr<CPPTree::AccessSpec>::DynamicCast(q.node) )
					throw YY::VNLangParser::syntax_error(
						any_cast<YY::VNLangParser::location_type>(loc),
						"Java-like access spec detected: " + string(DiagQuote(Traceable::TypeIdName( *aq )).c_str()) );
			}
		}
		if( !field->virt ) // absence of a vituality means non-virtual, for wild use ⯁Virtuality⦅⦆
			field->virt = MakeTreeNode<StandardAgentWrapper<CPPTree::NonVirtual>>();		
			
		ASSERT( fspg->current_access );

		stringstream ss;
		ss << any_cast<YY::VNLangParser::location_type>(loc);
		FTRACE("I'm putting in ")(fspg->current_access)(" at ")(ss.str())("\n");
		field->access = fspg->current_access; // Don't duplicate the subtree - we want coupling behaviour

		instance = field; // TODO store current access in the gnomon after #890
	}
	else if( dynamic_cast<const EnumeratorScopeGnomon *>(spg.get()) ) 
		instance = MakeTreeNode<StandardAgentWrapper<CPPTree::Global>>();  //TODO change to Enumerator when do enums
	else if( dynamic_cast<const GlobalScopeGnomon *>(spg.get()) ) 
		instance = MakeTreeNode<StandardAgentWrapper<CPPTree::Global>>(); 
	else if( dynamic_cast<const LocalScopeGnomon *>(spg.get()) ) 
		instance = MakeTreeNode<StandardAgentWrapper<CPPTree::Local>>(); 
	else 
		ASSERT(false)("Unknown gnomon: ")(spg);

	// Now fill in fields derived from the declarator
	Declarators::Result declarator_result = Declarators::Declarator::DoReduce(declarator, type);
	switch( declarator_result.outcome )
	{
		case Declarators::Result::CONCRETE:
		case Declarators::Result::WILDCARD:
		{
			// NOTE: innermost id == NULL => ☆ (not abstract)	
			instance->type = declarator_result.type;
			instance->identifier = declarator_result.leaf;
			break;
		}

		default:
			throw YY::VNLangParser::syntax_error(
				any_cast<YY::VNLangParser::location_type>(loc),
				"Expected concrete declaration but got abstract.");
	}
	
	instance->constancy = MakeTreeNode<StandardAgentWrapper<CPPTree::NonConst>>();
	for( const QualifierData &q : quals_pre )
		if( q.cat == QualCat::NODE )
			if( auto cq = TreePtr<CPPTree::Constancy>::DynamicCast(q.node) )
				instance->constancy = cq;
	
	// If indeed there is an initialiser, call OnInstanceInit() to over-ride this
	instance->initialiser = MakeTreeNode<StandardAgentWrapper<CPPTree::Uninitialised>>();

	return instance;
}


TreePtr<Node> VNLangActions::OnInstanceInit( TreePtr<Node> instance, any instance_loc, TreePtr<Node> init )
{
	auto o = TreePtr<CPPTree::Instance>::DynamicCast(instance);
	ASSERT(o);
	
	o->initialiser = init;
	
	return o;
}


TreePtr<Node> VNLangActions::OnAbDeclType( TreePtr<Node> type, TreePtr<Node> declarator, any declarator_loc )
{
	Declarators::Result result = Declarators::Declarator::DoReduce(declarator, type);
	switch( result.outcome )
	{
		case Declarators::Result::ABSTRACT:
			return result.type;	
		
		default:
			ASSERTFAIL(); // internal error because abstract decls are parsed separately
	}
}


TreePtr<Node> VNLangActions::OnInheritanceRecord( any loc, string keyword, TreePtr<Node> id, list<TreePtr<Node>> bases, list<TreePtr<Node>> members )
{
	TreePtr<CPPTree::InheritanceRecord> node;
	if( keyword=="class" )
		node = MakeTreeNode<StandardAgentWrapper<CPPTree::Class>>();
	else if( keyword=="struct" )
		node = MakeTreeNode<StandardAgentWrapper<CPPTree::Struct>>();
	else if( keyword=="union" )
		node = MakeTreeNode<StandardAgentWrapper<CPPTree::Union>>();
	else
		ASSERTFAIL()
	
	node->identifier = id;
	for( TreePtr<Node> base : bases )
		node->bases.insert( base );				
	
	for( TreePtr<Node> member : members )
		node->members.insert( member );		
	
	return node;
}


TreePtr<Node> VNLangActions::OnBase( TreePtr<Node> access, TreePtr<Node> type )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Base>>();	
	node->access = access; // TODO could now be user error eg "const" qualifier used as an access spec.
	node->record = type;
	return node;
}


TreePtr<Node> VNLangActions::OnBase( TreePtr<Node> type )
{
	// TODO zipping syntax:
	// ★ ★ parses to unrestricted Star
	//   ★  would be Star restricted to Base(default access, ☆)

	// if Star was given without an access, make it apply to the whole base. 
	// (this is the only legal way to interpret Star here)
	Agent *agent = Agent::AsAgent(type);
	if( agent->IsSubContainer() )
		return type;
		
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Base>>();	
	auto arch = dynamic_pointer_cast<CPPTree::Qualifier>(node->access.MakeValueArchetype());
	node->access = arch->GetDefaultNode(type);
	node->record = type;
	return node;
}


TreePtr<Node> VNLangActions::OnQualifierNodeKeyword( string keyword )
{
	if( keyword=="const" ) // todo Mutable node
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Const>>();
	else if( keyword=="public" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Public>>();
	else if( keyword=="private" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Private>>();
	else if( keyword=="protected" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Protected>>();
	else if( keyword=="virtual" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::Virtual>>();
	else 
		ASSERTFAIL();
}


TreePtr<Node> VNLangActions::OnIdValuePair( TreePtr<Node> key, any id_loc, TreePtr<Node> value )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::IdValuePair>>();
	
	node->key = key;
	node->value = value;
	return node;
}	


TreePtr<Node> VNLangActions::OnMapArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments )
{
	auto call = MakeTreeNode<StandardAgentWrapper<CPPTree::Call>>();
	auto args = MakeTreeNode<StandardAgentWrapper<CPPTree::MapArgumentation>>();
	call->callee = callee;
	call->argumentation = args;
	for( auto argument : arguments )
		args->arguments.insert( argument );
	return call;
}	


TreePtr<Node> VNLangActions::OnSeqArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments )
{
	auto call = MakeTreeNode<StandardAgentWrapper<CPPTree::Call>>();
	auto args = MakeTreeNode<StandardAgentWrapper<CPPTree::SeqArgumentation>>();
	call->callee = callee;
	call->argumentation = args;
	for( auto argument : arguments )
		args->arguments.insert( argument );
	return call;
}	


TreePtr<Node> VNLangActions::OnMapArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments )
{
	auto call = MakeTreeNode<StandardAgentWrapper<CPPTree::ConstructInit>>();
	auto args = MakeTreeNode<StandardAgentWrapper<CPPTree::MapArgumentation>>();
	call->constructor_id = constructor_id;
	call->argumentation = args;
	for( auto argument : arguments )
		args->arguments.insert( argument );
	return call;
}	


TreePtr<Node> VNLangActions::OnSeqArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments )
{
	auto call = MakeTreeNode<StandardAgentWrapper<CPPTree::ConstructInit>>();
	auto args = MakeTreeNode<StandardAgentWrapper<CPPTree::SeqArgumentation>>();
	call->constructor_id = constructor_id;
	call->argumentation = args;
	for( auto argument : arguments )
		args->arguments.insert( argument );
	return call;
}	


TreePtr<Node> VNLangActions::OnLookup( TreePtr<Node> object, TreePtr<Node> member, any member_loc )
{
	(void)member_loc;
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Lookup>>();
	node->object = object;
	node->member = member;
	return node;
}


TreePtr<Node> VNLangActions::OnIdByName( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?
	auto leaf_block = dynamic_cast<const AvailableNodeData::NodeBlock *>(block);
	NodeEnum ne = leaf_block->node_enum.value();

	string name = Unquote(ToASCII(wname));

	auto m = AvailableNodeData().GetEnumToNameMap();
	auto l = m.at(ne);
	string idt_ns = l.front();
	string idt_name = l.back();	
	ASSERT( !idt_name.empty() ); // internal error because we get a NodeEnum from the recogniser
	
	TreePtr<Node> ibn_node = IdentifierByNameAgent::TryMakeFromDestignatedType( idt_ns, idt_name, name );
	ASSERT( ibn_node )("%s::%s could not make id by name agent", idt_ns, idt_name);
	return ibn_node;
}


TreePtr<Node> VNLangActions::OnBuildId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wformat, any name_loc, Item sources )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?
	auto leaf_block = dynamic_cast<const AvailableNodeData::NodeBlock *>(block);
	NodeEnum ne = leaf_block->node_enum.value();

	// Format is "" if omitted otherwise a quoted string with the quotes still on
	string format = wformat.empty() ? "" : Unquote(ToASCII(wformat));
		
	auto m = AvailableNodeData().GetEnumToNameMap();
	auto l = m.at(ne);
	string idt_ns = l.front();
	string idt_name = l.back();	
	ASSERT( !idt_name.empty() ); // internal error because we get a NodeEnum from the recogniser
	
	TreePtr<Node> bia_node = BuildIdentifierAgent::TryMakeFromDestignatedType( idt_ns, idt_name, format ); 
	ASSERT( bia_node );
	
	for( TreePtr<Node> src : sources.nodes )
		dynamic_cast<BuildIdentifierAgent *>(bia_node.get())->sources.insert( src );						
	return bia_node;	
}


TreePtr<Node> VNLangActions::OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc )
{
	TreePtr<TransformOfAgent> to_agent;
	if( kind == "TypeOf" )
	{
		to_agent = MakeTreeNode<TransformOfAgent>( &TypeOf::instance );
	}
	else if( kind == "DeclarationOf" )
	{
		to_agent = MakeTreeNode<TransformOfAgent>( &DeclarationOf::instance );
	}
	else if( kind == "TypeDeclarationOf" )
	{
		to_agent = MakeTreeNode<TransformOfAgent>( &TypeDeclarationOf::instance );
	}
	else
	{
		ASSERT(false)("Unknown name ")(kind)(" was recognised but not handled here");
	}
	to_agent->pattern = pattern;
	return to_agent;
}

	
TreePtr<Node> VNLangActions::OnNegation( TreePtr<Node> operand )
{
	auto node = MakeTreeNode<NegationAgent>();
	node->negand = operand;
	return node;
}


TreePtr<Node> VNLangActions::OnConjunction( TreePtr<Node> left, TreePtr<Node> right )
{
	auto node = MakeTreeNode<ConjunctionAgent>();
	node->conjuncts = (left, right);
	return node;
}


TreePtr<Node> VNLangActions::OnDisjunction( TreePtr<Node> left, TreePtr<Node> right )
{
	auto node = MakeTreeNode<DisjunctionAgent>();
	node->disjuncts = (left, right);
	return node;
}
	

TreePtr<Node> VNLangActions::OnGrass( TreePtr<Node> through )
{
	auto node = MakeTreeNode<GreenGrassAgent>();
	node->through = through;
	return node;
}


TreePtr<Node> VNLangActions::OnPointerIs( TreePtr<Node> pointer )
{
	auto node = MakeTreeNode<PointerIsAgent>();
	node->pointer = pointer;
	return node;
}


TreePtr<Node> VNLangActions::OnStringize( TreePtr<Node> source )
{
	auto node = MakeTreeNode<StringizeAgent>();
	node->source = source;
	return node;
}


TreePtr<Node> VNLangActions::CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, any loc )
{
	int bits;
	if( lng )
		bits = TypeDb::integral_bits[clang::DeclSpec::TSW_long];
	else if( lng2 )
		bits = TypeDb::integral_bits[clang::DeclSpec::TSW_longlong];
	else
		bits = TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified];

	llvm::APSInt rv(bits, uns);
	rv = val;
	if( rv != val )
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(loc),
			"Integer literal: could not fit value " + to_string(val) + " into required type.");
	return MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificInteger>>( rv );
}


void VNLangActions::AddGnomon( shared_ptr<Gnomon> gnomon )
{
	ASSERT( gnomon );
	
	if( auto scope_gnomon = dynamic_pointer_cast<ScopeGnomon>(gnomon) )
		declaration_scope_gnomons.Push( scope_gnomon ); // front is top
	else 
		ASSERT(false)("VNLangActions doesn't know about gnomon: ")(*gnomon);
}




static NodeEnum GetNodeEnum( list<string> typ, any loc )
{
	if( !AvailableNodeData().GetNameToEnumMap().count(typ) )
	{
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(loc),
			"Built-in type " + DiagQuote(Join(typ, "::")) + " unknown.");
	}
	
	return AvailableNodeData().GetNameToEnumMap().at(typ);	
}


//////////////////////////// Virtuality ////////////////////////////// TODO don't put these here, use MakeStandardAgentFromTypeID

TreePtr<Node> CPPTree::Virtuality::GetDefaultNode(TreePtr<Node>) const
{
	return MakeTreeNode<StandardAgentWrapper<NonVirtual>>();
}

//////////////////////////// AccessSpec //////////////////////////////

TreePtr<Node> CPPTree::AccessSpec::GetDefaultNode(TreePtr<Node> type) const
{
	ASSERT(type); // if this is firing, it's probably due to explicit node parsing. Confirm we have an Instance and then pass in its type.
	auto record = TreePtr<Record>::DynamicCast(type);
	ASSERT(record); // Type is not a record (maybe could provide Public)
	return record->GetInitialAccess();
}

//////////////////////////// Constancy //////////////////////////////

TreePtr<Node> CPPTree::Constancy::GetDefaultNode(TreePtr<Node>) const
{
	return MakeTreeNode<StandardAgentWrapper<NonConst>>();
}



// Consider c-style scoping of designations (eg for macros?)

// Parsing 016-RaiseSCDeltaCount.vn reveals a mis-render - the identifier should be rendered 
// with some kind of "real identifier node" syntax (and its name hint)


// Productions using ⦅ ⦆: 
// - Try again at TypeOf->TypeIs etc
// - AFTER adding C mixture, try using () and making things look like printf etc

// Try c-style cast again, this time at a strictly lower precedence than the other prefix ops, per https://alx71hub.github.io/hcb/#statement
// Depends on whether we allow () on types. Not C, but avoids heavy reliance on designations

// NOTE ON CONSTRUCTORS
// Since VN uses "bound" C++, we will always need to specify which of possibly overloaded constructors we are
// referring to when constructing. So pure C++ is basically not possible here. Falling back to built-in node syntax herde.

// Note: comma operator can stay in: C-productions that use commas are all expressional and come in at norm_no_comma_op

// Renaming of productions and nodes 
// - Use eg https://alx71hub.github.io/hcb/#statement to rename everything in line with C++ grammar terminology. 
// - There are other interesting things here, like assign and ?: are not merged, ?: is higher and some RValue-like cases go straight to ?:, excluding assign - reproduce in parser and renderer, don't accept what C++ doesn't!
// - I think norm_/normal can become expr_
// - Labels will need their own "stuff" I think, otherwise it will be too hard to disambiguate with things like : and && hanging around
// Organisation: stuff from the C++ BNF

// {} is always an explicit Compound
// and ({}) is StatementExpression so {} should be available wherever () is

// Note: ▲⯈ stays at low prec so we can have ꩜⩨▲ uninterupted. Luckily both () and {} are lower and can be used directly, 
// each booting the parser so we can put a wide range of stuff in them. If we keep ∧ and ∨ at low precidence, the remaining 
// VN ops are all unops. This means the primary only appears once. Thus, we can safely duplicate the rules of them for any 
// "awkward" primaries (like compounds). 

// Review the virt-specifiers and const on the node methods for rendering

// MAYBE Move conj and disj back to original precidences. The lowered precs require two syntaxes, statement and expression. So we can
// get confused by eg { return x; ∧ goto y; } (=new form) versus { (return x;) ∧ (goto y;); } - or maybe we use the former and it's not confusing...
// No, don't. If we only have VN unops at high prec, we can build separate columns of them for incompatible primary
// elements without ambiguity. I.e. if P and Q are need to be kept separate, we can build P_prom, P_post, P_ptr, Q_prim, Q_post, Q_pre
// and all cases are covered. But if we want ∧ we need all 4 forms of P/Q_pre ∧ P/Q_pre and some of these it's unclear
// whether the expression is a P or a Q. It's better to put these at low prec even if we need more than one set.

// A pattern emerges in the CPPTree GetRender() functions: we are using VN-render policy to prevent the render from depending on a 
// direct analysis of child nodes - this works because VN-renders are patterns and could have special nodes in between (and
// we never attempt to analyse special nodes for their "true" type because it can be ambiguous.

// Agents inherit from nodes so Agent::GetAgentRender() can just be GetRender(). Simplify the call sites.

// Node::GetRender() could implement the explicit render.

// Check 093-DetectSuperLoop, we have a label definition ☆:; and then goto ☆ even though a coupling would seem to be needed, but we didn't get a designation.

// C fold-in: 
// things like OnPrefixOperator() should take an actual parser token not a string

// Types: see https://alx71hub.github.io/hcb/#decl-specifier-seq
// Keep the () on types for disambiguation unless you can prove away or just rely on designations TODO.

// Semantics of optional keywords: if absent, this is taken to be the default (eg, private for a base of a class, non-const for 
// a declaration etc). ☆ should be accepted to mean "any". Thus a fully wild base is ☆ ☆.

// Note: there are no explicit forms for agents. Thus, no ⯁Star⦅⦆ for example. This could have been handy for a greedy
// alternative to ★. But see zipping syntax #883 for an alternative that uses a post-pass to move ★ toward root.

// Note: 🞊  《》 ⸨⸩ are free now

// Labels an &&: use policy, as seen with resolvers, and get rid of PURE_IDENTIFIER (now only used to control && generation)

// Constructors etc:
// - Fix parse of my_type ( my_instanceidentifier ∧ ‽InstanceIdentifier ¬globals ) my_initialiser;
//   - Good example of VN stuff in a decl: ∧ has caused () but parser don't like ( when expecting identifier here
// - Fix remaining mis-parses with Instance::GetRender() enabled
// - Fill in OnInstance() properly and pass parser test
// - Re-enable render of member inits (MembInitSeq) and add parsing ability

// Resolving common ambiguities
// - Get better at dismabiguating VN stuff, primary, prefix, postfix including pre-restrictions. Use the recogniser.
// - Overdo this. Think in terms of SDE columns (statement/declaration/expression) and anything that cannot use
//   this due to conflicts needs its own "pocket syntax" i.e. prefix/postfix/primary VNs, disambiguated via recogniser
//   such that its constructs are unambiguous as with the rule for types.
// EXAMPLE: you can't use the SDE column for paraneters because it's ambiguous. You can't half use it either
// so you need a pocket for parameters. Is this to be shared with class members? Find out...

// - Add members to classes and move access_spec_nw across
// - Give operator comma lower prio than ∧ etc and re-instate

// See ReturnViaTemp: return ⯁Uninitialised⦅⦆; should just be return;

// NOTE: ; is NOT required for any VN operators working on statements or declarations. This is
// consistent with statements and declarations that end in {...} not requiring one either. We DO
// provide essentially optional semicolons for various surroundings. At present these are normal and decl_open 
// TODO name more consistently
// This is less conflicty that one might expect, and some languages, like golang, do this as 
// a matter of course.


// NOTE
// Lots of conflicts incl around declarators resolved by making pre-restriction refuse to switch between
// type and norm. This breaks DeclarationOf which was trying to be unified (the PR was supposed to disambiguate).
// fix is to split DeclarationOf and TypeDeclarationOf, then because TypeDeclarationOf converts from type
// to normal we have to put VN brackets around its argument making it a promary. To make life easier in the
// future, doing this to all the VN prefix ops that can switch between type and norm. Stuff is tricky, 1.
// we like the prefix syntax without any braces at all, but braces are likely needed for unified argument
// and 2. for completeness there should be a Stuff-As-Type symbol although this isn't manifesting atm.
 
