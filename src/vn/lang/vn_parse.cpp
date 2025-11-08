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
#include <locale>  
#include <codecvt>

using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

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
    
    saw_error = false;
    commands.clear();
    int pr = parser->parse();    
    if( saw_error ) // TODO figure out how to make the parser return a fail code
		exit(1); // An error was already reported so an assert fail here looks like a knock-on error i.e. a bug
    ASSERT(pr==EXIT_SUCCESS);    
    
    return commands;
}


void VNParse::OnError()
{
	saw_error = true;
}


void VNParse::OnVNScript( Command::List commands_ )
{
	if( saw_error )
		return;
		
	commands = commands_;
}

 
Production VNParse::OnEngine( Production stem )
{
	auto pure_engine = MakeTreeNode<PatternCommand::PureEngine>();
	pure_engine->stem = stem;
	return pure_engine;
}


Production VNParse::OnStuff( Production terminus )
{
	auto stuff = MakeTreeNode<StuffAgent>();
	stuff->terminus = terminus;
	return stuff;
}


Production VNParse::OnDelta( Production through, Production overlay )
{
	auto delta = MakeTreeNode<DeltaAgent>();
	delta->through = through;
	delta->overlay = overlay;
	return delta;
}


Production VNParse::OnName( wstring name, any name_loc )
{
	if( designations.count(name)==0 )
	{
		parser->error( any_cast<YY::VNLangParser::location_type>(name_loc), 
		               "Sub-pattern name " + 
		               wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(name) +
		               " not designated.");		
		return MakeTreeNode<Node>();
	}
			
	return designations.at(name);
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

	Production OnName( wstring name, any name_loc );

Production VNParse::OnBuiltIn( list<string> builtin_type, any builtin_loc, list<list<Production>> src_itemisation )
{
	if( !node_names->GetNameToEnumMap().count(builtin_type) )
	{
		parser->error( any_cast<YY::VNLangParser::location_type>(builtin_loc), 
		               "Built-in type " + Join(builtin_type, "::") + " unknown.");		
		return MakeTreeNode<Node>();
	}
	NodeEnum ne = node_names->GetNameToEnumMap().at(builtin_type);		
	// The new node is the destiation
	TreePtr<Node> dest = MakeStandardAgent(ne);
	
	// The detination's itemisation "pulls" items from the source and we require a match (for now). 
	// Then for containers, the source will "push" as many elements as it has, or just one
	// for singular items.
	list<list<TreePtr<Node>>>::const_iterator src_it = src_itemisation.begin();
    vector< Itemiser::Element * > dest_items = dest->Itemise();
    for( Itemiser::Element *dest_item : dest_items )
    {
		ASSERT( src_it != src_itemisation.end() )("Too few items in source"); // TODO semantic error TODO or switch to wild cards?
		const list<TreePtr<Node>> &src_item = *src_it;
        if( SequenceInterface *dest_seq = dynamic_cast<SequenceInterface *>(dest_item) ) // TODO could roll together as Container?
            for( TreePtr<Node> src : src_item )
				dest_seq->insert( src );				
        else if( CollectionInterface *dest_col = dynamic_cast<CollectionInterface *>(dest_item) )
            for( TreePtr<Node> src : src_item )
				dest_col->insert( src );	
        else if( TreePtrInterface *dest_sing = dynamic_cast<TreePtrInterface *>(dest_item) )
        {
			ASSERT( src_item.size() == 1 )("Singular item requires exactly one sub-pattern"); // TODO semantic error TODO or switch to wild cards? 
            *dest_sing = src_item.front();
		}
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
        src_it++;
    }
	ASSERT( src_it == src_itemisation.end() )("Too many items in source"); // TODO semantic error

	return dest;
}


Production VNParse::OnRestrict( list<string> res_type, any res_loc, Production target, any target_loc )
{
	if( !node_names->GetNameToEnumMap().count(res_type) )
	{
		parser->error( any_cast<YY::VNLangParser::location_type>(res_loc), 
		               "Restriction type " + Join(res_type, "::") + " unknown.");		
		return target;
	}
	NodeEnum ne = node_names->GetNameToEnumMap().at(res_type);
	
	Agent *agent = Agent::TryAsAgent(target);
	ASSERT( agent )("We are parsing a pattern so everything should be agents");
		
	auto pspecial = dynamic_cast<SpecialBase *>(agent);
	if( !pspecial )
	{
		parser->error( any_cast<YY::VNLangParser::location_type>(target_loc), 
		               "Restriction target " + agent->GetTypeName() + " cannot be pre-restricted.");		
		return target;
	}
		
	pspecial->pre_restriction_archetype_node = node_names->MakeNode(ne);
	pspecial->pre_restriction_archetype_ptr = node_names->MakeTreePtr(ne);

	return target;
}


Production VNParse::OnPrefixOperator( string tok, Production operand )
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


Production VNParse::OnPostfixOperator( string tok, Production operand )
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


Production VNParse::OnInfixOperator( string tok, Production left, Production right )
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


Production VNParse::OnSpecificInteger( int value )
{
	auto node =  MakeTreeNode<StandardAgentWrapper<SpecificInteger>>(value);
    return node;
}


void VNParse::Designate( wstring name, TreePtr<Node> sub_pattern )
{
	designations.insert( make_pair(name, sub_pattern) );
}

// Production->TreePtr<Node>
// Exceptions
// Better errors withe builtins (means custom types)
// Better errors generally
// grammer for C operators
// centralise your wstring conversions
// designate vs def, sort it out
