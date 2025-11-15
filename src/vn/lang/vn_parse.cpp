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

#include <iostream>
#include <fstream>
#include <cctype>

using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

static NodeEnum GetNodeEnum( list<string> typ, any loc );


VNParse::VNParse() :
	scanner( make_unique<YY::VNLangScanner>(reflex::Input(), std::cerr) ),
	parser( make_unique<YY::VNLangParser>(*scanner, this) ),
	node_names( make_unique<NodeNames>() )
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
			"⩨ depth condition " + QuoteName(limit.cond) + " not supported." + note);		
}


TreePtr<Node> VNParse::OnDelta( TreePtr<Node> through, TreePtr<Node> overlay )
{
	auto delta = MakeTreeNode<DeltaAgent>();
	delta->through = through;
	delta->overlay = overlay;
	return delta;
}


TreePtr<Node> VNParse::OnName( wstring name, any name_loc )
{
	if( designations.count(name)==0 )
		throw YY::VNLangParser::syntax_error( 
		    any_cast<YY::VNLangParser::location_type>(name_loc), 
		    "Sub-pattern name " + 
		    QuoteName(name) +
		    " was not designated.");		
			
	return designations.at(name);
}


TreePtr<Node> VNParse::OnEmbeddedCommands( list<shared_ptr<Command>> commands )
{
	TreePtr<Node> node;
	TRACE("Decaying embedded commands: ")(commands)("\n");
	for( shared_ptr<Command> c : commands )
	{
		node = c->Decay(node, this); 
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


TreePtr<Node> VNParse::OnBuiltIn( list<string> builtin_type, any builtin_loc, Itemisation src_itemisation )
{
	NodeEnum ne = GetNodeEnum( builtin_type, builtin_loc );
	
	// The new node is the destiation
	TreePtr<Node> dest = MakeStandardAgent(ne);
	
	// The detination's itemisation "pulls" items from the source and we require a match (for now). 
	// Then for containers, the source will "push" as many elements as it has, or just one
	// for singular items.
	list<Item>::const_iterator src_it = src_itemisation.items.begin();
    vector< Itemiser::Element * > dest_items = dest->Itemise();
    string counts_msg = SSPrintf("%s expects %d %s, but %d %s given.",
                             QuoteName(Join(builtin_type, "::")).c_str(),
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
    // Sniff out the case where user put eg ⯁Node() when ⯁Node is required
    if( dest_items.size()==0 && src_itemisation.items.size()==1 && src_itemisation.items.front().nodes.size()==0 )
		empty_note = "\nNote: where a node type requires zero items, simply omit the () entirely.";                             
	if( src_it != src_itemisation.items.end() )
		throw YY::VNLangParser::syntax_error( 
			any_cast<YY::VNLangParser::location_type>(src_it->loc), 
			"In ⯁, excess items given. " + counts_msg + empty_note);

	return dest;
}


TreePtr<Node> VNParse::OnRestrict( list<string> res_type, any res_loc, TreePtr<Node> target, any target_loc )
{
	NodeEnum ne = GetNodeEnum( res_type, res_loc );
	
	Agent *agent = Agent::TryAsAgent(target);
	ASSERT( agent )("We are parsing a pattern so everything should be agents");
		
	auto pspecial = dynamic_cast<SpecialBase *>(agent);
	if( !pspecial )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(target_loc), 
		     "Restriction target " + QuoteName(agent->GetTypeName()) + " cannot be pre-restricted.");		
		
	pspecial->pre_restriction_archetype_node = node_names->MakeNode(ne);
	pspecial->pre_restriction_archetype_ptr = node_names->MakeTreePtr(ne);

	return target;
}


TreePtr<Node> VNParse::OnPrefixOperator( string tok, TreePtr<Node> operand )
{
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) \
    if( tok==TEXT ) \
    { \
		auto node = MakeTreeNode<StandardAgentWrapper<NAME>>(); \
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
		auto node = MakeTreeNode<StandardAgentWrapper<NAME>>(); \
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
		auto node = MakeTreeNode<StandardAgentWrapper<NAME>>(); \
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


TreePtr<Node> VNParse::OnStringLiteral( wstring value )
{
	ASSERT( value.front() == '\"' && value.back() == '\"' ); // expecting quoted
	value = value.substr(1, value.size()-2 );
	return MakeTreeNode<StandardAgentWrapper<SpecificString>>(ToASCII(value));
}


TreePtr<Node> VNParse::OnBoolLiteral( bool value )
{
	if( value )
		return MakeTreeNode<StandardAgentWrapper<True>>();
	else
		return MakeTreeNode<StandardAgentWrapper<False>>();
}


TreePtr<Node> VNParse::OnIdByName( list<string> typ, any type_loc, string name, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?
	
	if( typ.size() == 1 )
		typ.push_front("CPPTree"); // TODO centralise
	
	TreePtr<Node> node = IdentifierByNameAgent::TryMakeFromDestignatedType( typ.front(), typ.back(), name );
	
	if( !node )
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(type_loc),
			"⊛ requires identifier type discriminator i.e. " + 
			QuoteName(Join(typ, "::") + "Identifier") +
			" would need to exist as a node type.");
	
	return node;
}


TreePtr<Node> VNParse::OnBuildId( list<string> typ, any type_loc, string format, any name_loc, Item sources )
{
	(void)name_loc; // TODO perhaps BuildIdentifierAgent can validate this?
	
	if( typ.size() == 1 )
		typ.push_front("CPPTree");  // TODO centralise

	TreePtr<Node> bia_node = BuildIdentifierAgent::TryMakeFromDestignatedType( typ.front(), typ.back(), format );
	
	if( !bia_node )
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(type_loc),
			"⧇ requires identifier type discriminator i.e. " + 
			QuoteName(Join(typ, "::") + "Identifier") +
			" would need to exist as a node type.");
	
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
			QuoteName(kind) +
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


void VNParse::Designate( wstring name, TreePtr<Node> sub_pattern )
{
	designations.insert( make_pair(name, sub_pattern) );
}


string VNParse::QuoteName(string name)
{
	return "`" + name + "'";
}


string VNParse::QuoteName(wstring name)
{
	return QuoteName( ToASCII(name) ); 
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
	return MakeTreeNode<StandardAgentWrapper<SpecificInteger>>( rv );
}


static NodeEnum GetNodeEnum( list<string> typ, any loc )
{
	if( !NodeNames().GetNameToEnumMap().count(typ) )
	{
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(loc),
			"Built-in type " + VNParse::QuoteName(Join(typ, "::")) + " unknown.");
	}
	
	return NodeNames().GetNameToEnumMap().at(typ);	
}


// grammar for C operators
// Consider c-style scoping of designations (eg for macros?)

// Parsing 016-RaiseSCDeltaCount.vn reveals a mis-render - the identifier should be rendered 
// with some kind of "real identifier node" syntax (and its name hint)

// Use "lexical tie-in" to handle the context of designations and prevent "undesignated name" errors in designations
// - Parser designates (done)
// - Lexer checks and produces either X_NEW_NAME or X_KNOWN_NAME
// - Use the appropriate one in the grammar. Both errors should now be generated by parser

// Namespaces: CPPTree should be assumed as a default where not specified. More than one specifier is still TBD
// Common stuff for qualified types with :: including ability to throw on eg A::B::C (but could support later)

// Remove the need for subclasses in Identifier-related nodes, then simplify ⊛ 

// Diff testing!

// Don't force user to use * on command line: accept a directory for input VN files

// Tix:
// Lose StandardAgentWrapper #867
// Add ability to pre-process #862
// Be a proper Unicode language #868

// C fold-in: 
// things like OnPrefixOperator() should take an actual parser token not a string
