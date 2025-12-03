#include "vn_parse.hpp"

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

//using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

static NodeEnum GetNodeEnum( list<string> typ, any loc );


VNParse::VNParse() :
	recogniser( make_unique<VNLangRecogniser>() ),
	scanner( make_unique<YY::VNLangScanner>(recogniser.get(), reflex::Input(), std::cerr) ),
	parser( make_unique<YY::VNLangParser>(*scanner, this, recogniser.get()) ),
	node_names( make_unique<AvailableNodeData>() )
{
}

VNParse::~VNParse()
{
	// Keep me: compiler can see complete VNLangScanner and VNLangParser here
}


Command::List VNParse::DoParse(string filepath)
{
    FILE *file = fopen(filepath.c_str(), "r");

    ASSERT(file != NULL)("Cannot open VN file: ")(filepath);

    scanner->in(file);
    scanner->filename = filepath;    
    
    top_level_commands.clear();
    int pr = parser->parse();    
    if( pr != EXIT_SUCCESS ) 
		exit(pr); // An error was already reported so an assert fail here looks like a knock-on error i.e. a bug
    
    return top_level_commands;
}


void VNParse::OnVNScript( Command::List top_level_commands_ )
{
	top_level_commands = top_level_commands_;
}


TreePtr<Node> VNParse::OnStar( TreePtr<Node> restriction )
{
	auto node = MakeTreeNode<StarAgent>();
	node->restriction = restriction;
	return node;
}


TreePtr<Node> VNParse::OnBuildSize( TreePtr<Node> container )
{
	auto node = MakeTreeNode<BuildContainerSizeAgent>();
	node->container = container;
	return node;
}


TreePtr<Node> VNParse::OnStuff( TreePtr<Node> terminus, TreePtr<Node> recurse_restriction, Limit limit )
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


TreePtr<Node> VNParse::OnDelta( TreePtr<Node> through, TreePtr<Node> overlay )
{
	auto delta = MakeTreeNode<DeltaAgent>();
	delta->through = through;
	delta->overlay = overlay;
	return delta;
}


TreePtr<Node> VNParse::OnEmbeddedCommands( list<shared_ptr<Command>> commands )
{
	TreePtr<Node> node;
	TRACE("Decaying embedded commands: ")(commands)("\n");
	for( shared_ptr<Command> c : commands )
	{
		node = c->DecayToPattern(node, this); 
		// TODO could generalise to things that can Execute from within SCREngine
		ASSERT( node )("Command ")(c)(" could not decay with sub-pattern ")(node);
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


TreePtr<Node> VNParse::OnBuiltIn( const AvailableNodeData::Block *block, any node_name_loc, Itemisation src_itemisation )
{
	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
	// Parser could do this if we separated the tokens 
	if( !leaf_block || !leaf_block->node_enum )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(node_name_loc), 
				    SSPrintf("In ⯁, unexpected %s when expecting node name.", block->What().c_str()) ); 
	
	// The new node is the destination
	NodeEnum ne = leaf_block->node_enum.value();
	TreePtr<Node> dest = MakeStandardAgent(ne);
	
	// The detination's itemisation "pulls" items from the source and we require a match (for now). 
	// Then for containers, the source will "push" as many elements as it has, or just one
	// for singular items.
	list<Item>::const_iterator src_it = src_itemisation.items.begin();
    vector< Itemiser::Element * > dest_items = dest->Itemise();
    string counts_msg = SSPrintf("%s expects %d %s, but %d %s given.",
                             DiagQuote(Traceable::TypeIdName( *dest )).c_str(),
                             dest_items.size(),
                             dest_items.size()==1?"item":"items",
                             src_itemisation.items.size(),
                             src_itemisation.items.size()==1?"was":"were");
    YY::VNLangParser::location_type prev_loc = any_cast<YY::VNLangParser::location_type>(src_itemisation.loc);
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
			if( src_item.nodes.size() != 1 )
			    throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(src_item.loc),
				    SSPrintf("In ⯁, singular item requires exactly one sub-pattern but %d were given.",
				    src_item.nodes.size() ) ); 
            *dest_sing = src_item.nodes.front();
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
        src_it++;
        prev_loc = any_cast<YY::VNLangParser::location_type>(src_item.loc);
    }
    string empty_note;
    // Sniff out the case where we got one empty item and needed zero items: this is now allowed!
    if( dest_items.size()==0 && src_itemisation.items.size()==1 && src_itemisation.items.front().nodes.size()==0 )
		return dest;
		                             
	if( src_it != src_itemisation.items.end() )
		throw YY::VNLangParser::syntax_error( 
			any_cast<YY::VNLangParser::location_type>(src_it->loc), 
			"In ⯁, excess items given. " + counts_msg + empty_note);

	return dest;
}


TreePtr<Node> VNParse::OnRestrict( const AvailableNodeData::Block *block, any node_name_loc, TreePtr<Node> target, any target_loc )
{
	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
	// Parser could do this if we separated the tokens 
	if( !leaf_block || !leaf_block->node_enum )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(node_name_loc), 
				    SSPrintf("In ‽, unexpected %s when expecting node name.", block->What().c_str()) ); 	
	
	NodeEnum ne = leaf_block->node_enum.value();
	Agent *agent = Agent::TryAsAgent(target);
	ASSERT( agent )("We are parsing a pattern so everything should be agents");
		
	auto pspecial = dynamic_cast<SpecialBase *>(agent);
	if( !pspecial )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(target_loc), 
		     "‽ cannot be used with " + DiagQuote(agent->GetTypeName()));		
		
	pspecial->pre_restriction_archetype_node = node_names->MakeNode(ne);
	pspecial->pre_restriction_archetype_ptr = node_names->MakeTreePtr(ne);

	return target;
}


TreePtr<Node> VNParse::OnPrefixOperator( string tok, TreePtr<Node> operand )
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


TreePtr<Node> VNParse::OnPostfixOperator( string tok, TreePtr<Node> operand )
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


TreePtr<Node> VNParse::OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right )
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


TreePtr<Node> VNParse::OnIntegralLiteral( string text, any loc )
{
	// Normalise to upper case
    transform(text.begin(), text.end(), text.begin(), ::toupper);
    
    bool uns = text.find("U") != std::string::npos;
    bool lng2 = text.find("LL") != std::string::npos;
    bool lng = (text.find("L") != std::string::npos) && !lng2;
    
    return CreateIntegralLiteral(uns, lng2, lng, stoull(text), loc);
}


TreePtr<Node> VNParse::OnStringLiteral( wstring wvalue )
{
	string value = Unquote(ToASCII(wvalue));
	return MakeTreeNode<StandardAgentWrapper<CPPTree::SpecificString>>(value);
}


TreePtr<Node> VNParse::OnBoolLiteral( bool value )
{
	if( value )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::True>>();
	else
		return MakeTreeNode<StandardAgentWrapper<CPPTree::False>>();
}


TreePtr<Node> VNParse::OnCast( TreePtr<Node> type, any type_loc, TreePtr<Node> operand, any operand_loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Cast>>();
	node->operand = operand;	
	node->type = type;
	return node;
}


TreePtr<Node> VNParse::OnConditionalOperator( TreePtr<Node> condition, TreePtr<Node> expr_then, TreePtr<Node> expr_else )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::ConditionalOperator>>();
	node->condition = condition;	
	node->expr_then = expr_then;
	node->expr_else = expr_else;
	return node;
}
	

TreePtr<Node> VNParse::OnSubscript( TreePtr<Node> destination, TreePtr<Node> index )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Subscript>>();
	node->destination = destination;	
	node->index = index;
	return node;
}
	

TreePtr<Node> VNParse::OnCompound( list<TreePtr<Node>> statements )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Compound>>();
	for( TreePtr<Node> statement : statements )
		node->statements.insert( statement );
	return node;
}


TreePtr<Node> VNParse::OnArrayLiteral( TreePtr<Node> root_of_comma_expression )
{
	// Break up a comma-separated expression, assuming left-associative
	TreePtr<Node> current = root_of_comma_expression;
	list<TreePtr<Node>> values;
	while( auto comma = TreePtr<CPPTree::Comma>::DynamicCast(current) )
	{
		values.push_front( comma->operands.back() );
		current = comma->operands.front();
	}
	values.push_front( current );
	
	// Fill out the array literal
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::ArrayLiteral>>();
	for( TreePtr<Node> value : values )
		node->operands.insert( value );
	return node;
}


TreePtr<Node> VNParse::OnNormalTerminalKeyword( string keyword, any keyword_loc )
{
	if( keyword=="this" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::This>>();
	
	ASSERTFAIL();
}


TreePtr<Node> VNParse::OnSpecificId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?

	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
	// Parser could do this if we separated the tokens 
	if( !leaf_block || !leaf_block->identifier_discriminator_enum )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(id_disc_loc), 
				    SSPrintf("In 🞊, unexpected %s when expecting identifier-discriminator.", block->What().c_str()) ); 		
	IdentifierEnum ie = leaf_block->identifier_discriminator_enum.value();

	string name = Unquote(ToASCII(wname));
		
	switch( ie )
	{
#define NODE(NS, NAME) \
	case IdentifierEnum::NS##_##NAME: \
		return MakeTreeNode<StandardAgentWrapper<NS::Specific##NAME##Identifier>>(name); 
#include "tree/identifier_names.inc"	
#undef NODE
	default:
		ASSERTFAIL(); // switch should have covered everything in the inc file
	}	
}


TreePtr<Node> VNParse::OnIdByName( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?

	if( !block )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(id_disc_loc), 
				    SSPrintf("In ⊛, unexpected non-block when expecting identifier-discriminator.") ); 		


	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
	// Parser could do this if we separated the tokens 
	if( !leaf_block || !leaf_block->identifier_discriminator_enum )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(id_disc_loc), 
				    SSPrintf("In ⊛, unexpected %s when expecting identifier-discriminator.", block->What().c_str()) ); 		
	IdentifierEnum ie = leaf_block->identifier_discriminator_enum.value();

	string name = Unquote(ToASCII(wname));

	switch( ie )
	{
#define NODE(NS, NAME) \
	case IdentifierEnum::NS##_##NAME: \
		return IdentifierByNameAgent::TryMakeFromDestignatedType( #NS, #NAME, name );
#include "tree/identifier_names.inc"	
#undef NODE
	default:
		ASSERTFAIL(); // switch should have covered everything in the inc file
	}	
}


TreePtr<Node> VNParse::OnBuildId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wformat, any name_loc, Item sources )
{
	(void)name_loc; // TODO perhaps BuildIdentifierAgent can validate this?

	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
	// Parser could do this if we separated the tokens 
	if( !leaf_block || !leaf_block->identifier_discriminator_enum )
		throw YY::VNLangParser::syntax_error( 
				    any_cast<YY::VNLangParser::location_type>(id_disc_loc), 
				    SSPrintf("In ⧇, unexpected %s when expecting identifier-discriminator.", block->What().c_str()) ); 		
	IdentifierEnum ie = leaf_block->identifier_discriminator_enum.value();

	// Format is "" if omitted otherwise a quoted string with the quotes still on
	string format = wformat.empty() ? "" : Unquote(ToASCII(wformat));
		
	TreePtr<Node> bia_node;
	switch( ie )
	{
#define NODE(NS, NAME) \
	case IdentifierEnum::NS##_##NAME: \
		bia_node = BuildIdentifierAgent::TryMakeFromDestignatedType( #NS, #NAME, format ); \
		break;
#include "tree/identifier_names.inc"	
#undef NODE
	default:
		ASSERTFAIL(); // switch should have covered everything in the inc file
	}
	
	for( TreePtr<Node> src : sources.nodes )
		dynamic_cast<BuildIdentifierAgent *>(bia_node.get())->sources.insert( src );						
	return bia_node;	
}


TreePtr<Node> VNParse::OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc )
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
	else
	{
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(kind_loc),
			DiagQuote(kind) +
			" unsupported in ⤨.");
	}
	to_agent->pattern = pattern;
	return to_agent;
}

	
TreePtr<Node> VNParse::OnNegation( TreePtr<Node> operand )
{
	auto node = MakeTreeNode<NegationAgent>();
	node->negand = operand;
	return node;
}


TreePtr<Node> VNParse::OnConjunction( TreePtr<Node> left, TreePtr<Node> right )
{
	auto node = MakeTreeNode<ConjunctionAgent>();
	node->conjuncts = (left, right);
	return node;
}


TreePtr<Node> VNParse::OnDisjunction( TreePtr<Node> left, TreePtr<Node> right )
{
	auto node = MakeTreeNode<DisjunctionAgent>();
	node->disjuncts = (left, right);
	return node;
}
	

TreePtr<Node> VNParse::OnGrass( TreePtr<Node> through )
{
	auto node = MakeTreeNode<GreenGrassAgent>();
	node->through = through;
	return node;
}


TreePtr<Node> VNParse::OnPointerIs( TreePtr<Node> pointer )
{
	auto node = MakeTreeNode<PointerIsAgent>();
	node->pointer = pointer;
	return node;
}


TreePtr<Node> VNParse::OnStringize( TreePtr<Node> source )
{
	auto node = MakeTreeNode<StringizeAgent>();
	node->source = source;
	return node;
}


string VNParse::DiagQuote(string name)
{
	return "`" + name + "'";
}


string VNParse::DiagQuote(wstring name)
{
	return DiagQuote( ToASCII(name) ); 
}


TreePtr<Node> VNParse::CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, any loc )
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


static NodeEnum GetNodeEnum( list<string> typ, any loc )
{
	if( !AvailableNodeData().GetNameToEnumMap().count(typ) )
	{
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(loc),
			"Built-in type " + VNParse::DiagQuote(Join(typ, "::")) + " unknown.");
	}
	
	return AvailableNodeData().GetNameToEnumMap().at(typ);	
}


VNLangRecogniser &VNParse::GetShim()
{
	return *recogniser;
}

// Consider c-style scoping of designations (eg for macros?)

// Parsing 016-RaiseSCDeltaCount.vn reveals a mis-render - the identifier should be rendered 
// with some kind of "real identifier node" syntax (and its name hint)

// Namespaces: CPPTree should be assumed as a default where not specified. More than one specifier is still TBD
// Common stuff for qualified types with :: including ability to throw on eg A::B::C (but could support later)

// Renamings: 
// VNScriptRunner -> VNScript
// VNParse -> VNActions and move DoParse() into VNScript.
// Ensure available_node_info, recogniser, and this file have the right filenames

// Productions using 【 】: Use a gnomon to enable recogniser to "see" whatever is in the brackets eg TypeOf

// Productions using 【 】: AFTER adding C mixture, try using () and making things look like printf etc

// When designating a ⧇ or speciifc identifier node, why not use the given name as the name of the designation?

// NOT putting some kind of quotes around identifier names or format strings feels like a conflict even if it isn't

// Implement %printer for semantic values

// itemisation () should parse as zero items, and then be relaxed into one empty item in the action, i.e.
// the other way around fropm how it is now. That's because, if they are used in "resembles" productions,
// we won't want empty items - we'll want items to be one of: singular, collection, sequence

// Tix:
// Lose StandardAgentWrapper #867
// Add ability to pre-process #862
// Global designations #872
// Type mismatches #874


// C fold-in: 
// things like OnPrefixOperator() should take an actual parser token not a string
