#include "vn_commands.hpp"

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "tree/node_names.hpp"
#include "vn_step.hpp"
#include "vn_actions.hpp"
#include "vn_script.hpp"
#include "agents/embedded_scr_agent.hpp"
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include "vn_recogniser.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#define ELIMINATE_STEP_NUMBER

using namespace CPPTree; 
using namespace VN;

//////////////////////////// Command ///////////////////////////////

Command::Command( any loc_ ) :
	loc( loc_ )
{
}


Command::~Command()
{
}


TreePtr<Node> Command::OnParseEmbedded( TreePtr<Node> node, VNLangActions *vn )
{
	if( node )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(loc), 
		     "Unexpected pattern before " + DiagQuote(Traceable::TypeIdName( *this )));
	return node; 
}


void Command::Execute(const ScriptKit &kit) const
{
}

//////////////////////////// EngineCommand ///////////////////////////////

EngineCommand::EngineCommand( TreePtr<Node> stem_, any loc_ ) :
	Command(loc_),
	stem( stem_ )
{
	ASSERT(stem);
}


TreePtr<Node> EngineCommand::OnParseEmbedded( TreePtr<Node> node, VNLangActions *vn )
{
	if( !node )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(loc), 
		     "A through-pattern must be provided as the command before the first ꩜ command");
	auto embedded = MakeTreeNode<EmbeddedSCRAgent>(stem, stem);			
	embedded->through = node;
	return embedded;	
}


void EngineCommand::Execute(const ScriptKit &kit) const
{
	kit.vn_script->AddStep( kit, stem );
}


TreePtr<Node> EngineCommand::GetPattern() const // TODO drop?
{
	return stem;
}


string EngineCommand::GetTrace() const
{
	return "EngineCommand stem: " + stem.GetTrace();
}

//////////////////////////// PatternCommand ///////////////////////////////

PatternCommand::PatternCommand( TreePtr<Node> pattern_, any loc_ ) :
	Command(loc_),
	pattern( pattern_ )
{
	// pattern can be NULL (i.e. singular wildcard)
}


TreePtr<Node> PatternCommand::OnParseEmbedded( TreePtr<Node> node, VNLangActions *vn )
{
	if( node )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(loc), 
		     "Unexpected pair of patterns in command sequence");
	return pattern; 
}


string PatternCommand::GetTrace() const
{
	return "PatternCommand: " + Trace(pattern);
}


//////////////////////////// AttributeCommand ///////////////////////////////

AttributeCommand::AttributeCommand( any loc_ ) :
	Command(loc_)
{
}


TreePtr<Node> AttributeCommand::OnParseEmbedded( TreePtr<Node> node, VNLangActions *vn )
{
	throw YY::VNLangParser::syntax_error(
	     any_cast<YY::VNLangParser::location_type>(loc), 
	     "Attribute command may only be used at top level");	
}


void AttributeCommand::Execute(const ScriptKit &kit) const
{
	// The only thing we can do with attributes is to set the lowering-for-render flag
	// which marks the step as still to be run when using -q...+
	kit.vn_script->SetLoweringForRenderStep();
}


string AttributeCommand::GetTrace() const
{
	return "AttributeCommand";
}

