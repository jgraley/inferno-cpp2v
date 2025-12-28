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


TreePtr<Node> VNLangActions::OnBuiltIn( const AvailableNodeData::Block *block, any node_name_loc, Itemisation src_itemisation )
{
	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
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
		                             
	if( src_it != src_itemisation.items.end() )
		throw YY::VNLangParser::syntax_error( 
			any_cast<YY::VNLangParser::location_type>(src_it->loc), 
			"In ⯁, excess items given. " + counts_msg);

	return dest;
}


TreePtr<Node> VNLangActions::OnRestrict( const AvailableNodeData::Block *block, any node_name_loc, TreePtr<Node> target, any target_loc )
{
	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
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
	

TreePtr<Node> VNLangActions::OnCompound( list<TreePtr<Node>> statements )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Compound>>();
	for( TreePtr<Node> statement : statements )
		node->statements.insert( statement );
	return node;
}


TreePtr<Node> VNLangActions::OnArrayLiteral( Item elements )
{
	// Fill out the array literal
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::ArrayLiteral>>();
	for( TreePtr<Node> value : elements.nodes )
		node->elements.insert( value );
	return node;
}


TreePtr<Node> VNLangActions::OnLabel( TreePtr<Node> identifier, any loc )
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Label>>();
	node->identifier = identifier;
	return node;
}


TreePtr<Node> VNLangActions::OnNormalTerminalKeyword( string keyword, any keyword_loc )
{
	if( keyword=="this" )
		return MakeTreeNode<StandardAgentWrapper<CPPTree::This>>();
	
	ASSERTFAIL();
}


TreePtr<Node> VNLangActions::OnSpaceSepStmtKeyword( string keyword, any keyword_loc, TreePtr<Node> operand, any operand_loc )
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
		if( auto der = TreePtr<CPPTree::Dereference>::DynamicCast(operand) )
			ret->destination = der->operands.front();
		else
			ret->destination = operand;
			
		return ret;
	}
	
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



TreePtr<Node> VNLangActions::OnSpecificId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?

	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
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


TreePtr<Node> VNLangActions::OnIdByName( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?
	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
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


TreePtr<Node> VNLangActions::OnBuildId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wformat, any name_loc, Item sources )
{
	(void)name_loc; // TODO perhaps BuildIdentifierAgent can validate this?

	auto leaf_block = dynamic_cast<const AvailableNodeData::LeafBlock *>(block);
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
	else
	{
		ASSERT(false)("Unknown name ")(kind)(" should have been caught by recogniser");
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


// Consider c-style scoping of designations (eg for macros?)

// Parsing 016-RaiseSCDeltaCount.vn reveals a mis-render - the identifier should be rendered 
// with some kind of "real identifier node" syntax (and its name hint)


// Productions using 【 】: 
// - I think it looks better to go eg ⯁CPPTree::TypeDeclaration【sc_module】 and similar for that whole family of productions
// - Default away the CPPTree:: (it's sugar, but the bulkiness of these is affecting my decisions!)
// - AFTER adding C mixture, try using () and making things look like printf etc

// Try c-style cast again, this time at a strictly lower precedence than the other prefix ops, per https://alx71hub.github.io/hcb/#statement

// Calls/Constructors plan:
// - Try to use ⫶ to switch into initialiser production just for the next thing i.e. ⫶ goes in at the last minute

// Note: comma operator can stay in: C-productions that use commas are all expressional and come in at norm_no_comma_op

// It looks like TRANSFORM rule is acting on types only, which might be good for TypeOf but not others

// Renaming of productions and nodes 
// - Use eg https://alx71hub.github.io/hcb/#statement to rename everything in line with C++ grammar terminology. 
// - There are other interesting things here, like assign and ?: are not merged, ?: is higher and some RValue-like cases go straight to ?:, excluding assign - reproduce in parser and renderer, don't accept what C++ doesn't!
// - I think norm_/normal can become expr_
// - Labels will need their own "stuff" I think, otherwise it will be too hard to disambiguate with things like : and && hanging around

// Tix:
// Lose StandardAgentWrapper #867
// Add ability to pre-process #862
// Global designations #872
// Type mismatches #874


// C fold-in: 
// things like OnPrefixOperator() should take an actual parser token not a string
