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
#include "vn_parse.hpp"
#include "vn_script.hpp"
#include "agents/embedded_scr_agent.hpp"
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"

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


bool Command::OnParse(VNParse *vn)
{
	return true; // By default, keep me
}


TreePtr<Node> Command::DecayToPattern( TreePtr<Node> node, VNParse *vn )
{
	return nullptr; // by default, I can't decay
}


void Command::Execute(const ScriptKit &kit) const
{
	ASSERTFAIL("Commands should either be discarded by OnParse() returning false, or implement Execute()");
}

//////////////////////////// EngineCommand ///////////////////////////////

EngineCommand::EngineCommand( TreePtr<Node> stem_, any loc_ ) :
	Command(loc_),
	stem( stem_ )
{
	ASSERT(stem);
}


TreePtr<Node> EngineCommand::DecayToPattern( TreePtr<Node> node, VNParse *vn )
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
	ASSERT(pattern);
}


TreePtr<Node> PatternCommand::DecayToPattern( TreePtr<Node> node, VNParse *vn )
{
	if( node )
		throw YY::VNLangParser::syntax_error(
		     any_cast<YY::VNLangParser::location_type>(loc), 
		     "Only the first command in an embedded command sequnce may be a pattern");
	return pattern; 
}


string PatternCommand::GetTrace() const
{
	return "PatternCommand: " + Trace(pattern);
}

//////////////////////////// Designation ///////////////////////////////

Designation::Designation( std::wstring name_, TreePtr<Node> pattern_, any loc_ ) :
	Command(loc_),
	name( name_ ),
	pattern( pattern_ )
{
}


bool Designation::OnParse(VNParse *vn)
{
	vn->Designate(name, pattern);
	
	return false; // discard
}


string Designation::GetTrace() const
{
	return "Designation: " + Trace(name) + "⪮" + Trace(pattern);
}
