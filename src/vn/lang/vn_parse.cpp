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

#include <iostream>
#include <fstream>


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


TreePtr<Node> VNParse::DoParse(string filepath)
{
    FILE *file = fopen(filepath.c_str(), "r");

    ASSERT(file != NULL)("Cannot open VN file: ")(filepath);

    scanner->in(file);
    scanner->filename = filepath;    
    
    saw_error = false;
    script = nullptr;
    int pr = parser->parse();    
    if( saw_error ) // TODO figure out how to make the parser return a fail code
		exit(1); // An error was already reported so an assert fail here looks like a knock-on error i.e. a bug
    ASSERT(script && pr==EXIT_SUCCESS);
    
    return script;
}


void VNParse::OnError()
{
	saw_error = true;
}


void VNParse::OnVNScript( Production script_ )
{
	if( saw_error )
		return;
		
	FTRACE("Got ")(script_)("\n");
	script = script_;
}

 
Production VNParse::OnEngine( Production stem )
{
	FTRACE("Got engine stem: ")(stem)("\n");
	auto pure_engine = MakeTreeNode<PureEngine>();
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

// TODO parse the pre-restrictions and apply to child in action
