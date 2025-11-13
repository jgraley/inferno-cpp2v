#include "vn_parse.hpp"

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
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include "vn/agents/all.hpp"
#include "tree/node_names.hpp"
#include "vn_commands.hpp"

#include <iostream>
#include <fstream>

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


TreePtr<Node> VNParse::OnStar()
{
	return MakeTreeNode<StarAgent>();
}


TreePtr<Node> VNParse::OnStuff( TreePtr<Node> terminus )
{
	auto stuff = MakeTreeNode<StuffAgent>();
	stuff->terminus = terminus;
	return stuff;
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
	ASSERTFAIL("Invalid value for node enum"); 
}

	TreePtr<Node> OnName( wstring name, any name_loc );

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
				"Insufficient items given. " + counts_msg );
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
				    SSPrintf("Singular item requires exactly one sub-pattern but %d were given.",
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
			"Excess items given. " + counts_msg + empty_note);

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


TreePtr<Node> VNParse::OnSpecificInteger( int value )
{
	auto node =  MakeTreeNode<StandardAgentWrapper<SpecificInteger>>(value);
    return node;
}


TreePtr<Node> VNParse::OnIdByName( list<string> typ, any type_loc, wstring name, any name_loc )
{
	(void)name_loc; // TODO perhaps IdentifierByNameAgent can validate this?
	
	if( typ.size() == 1 )
		typ.push_front("CPPTree");
	
	// We need to strip off the quotes
	name = name.substr(1, name.size() - 2);

	TreePtr<Node> node = IdentifierByNameAgent::TryMakeFromName( typ.front(), typ.back(), ToASCII(name) );
	
	if( !node )
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(type_loc),
			"⊛ requires itentifier type discriminator i.e. " + 
			QuoteName(Join(typ, "::") + "Identifier") +
			" would need to exist as a node type.");
	
	return node;
}


TreePtr<Node> VNParse::OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc )
{
	if( kind == "TypeOf" )
	{
		return MakeTreeNode<TransformOfAgent>( &TypeOf::instance );
	}
	else
	{
		throw YY::VNLangParser::syntax_error(
		    any_cast<YY::VNLangParser::location_type>(kind_loc),
			QuoteName(kind) +
			" unsupported in ⤨.");
	}
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

// .vn to .c.vn?

// Item/Itemisation trouble:
// () is ambiguous: no items, or one empty one? Parses as the former. Resolve by looking at the node type.
// - OR... no parents means no items, and empty parens mean one empty item. Give a "note" 
//         when empty parens given for node type that wants no items
// Source locations not filled in for empty items: use the location of the ⚬ that was "swallowed"
// - we probably want explict syntax i.e. Item::Empty(@n)
// Test out the erorr cases!

// Tix:
// Lose StandardAgentWrapper #867
// Add ability to pre-process #862
// Be a proper Unicode language #868
