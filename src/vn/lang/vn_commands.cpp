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
#include "vn/agents/embedded_scr_agent.hpp"

#include <iostream>
#include <fstream>



using namespace CPPTree; 
using namespace VN;

//////////////////////////// Command ///////////////////////////////

Command::~Command()
{
}


bool Command::OnParse(VNParse *vn)
{
	return true; // By default, keep me
}


TreePtr<Node> Command::Decay( TreePtr<Node> node, VNParse *vn )
{
	return nullptr; // by default, I can't decay
}


void Command::Execute(const ScriptKit &kit) const
{
	ASSERTFAIL("Commands should either be discarded by OnParse() returning false, or implement Execute()");
}

//////////////////////////// ScriptEngine ///////////////////////////////

void ScriptEngine::DoExecute( const ScriptKit &kit, Command::List script )
{
	for( shared_ptr<Command> c : script )
	{
		c->Execute(kit);
	}
}

//////////////////////////// EngineCommand ///////////////////////////////

EngineCommand::EngineCommand( TreePtr<Node> stem_ ) :
	stem( stem_ )
{
	ASSERT(stem);
}


TreePtr<Node> EngineCommand::Decay( TreePtr<Node> node, VNParse *vn )
{
	ASSERT( node )("Through pattern not specified before embedded engine");
	auto embedded = MakeTreeNode<EmbeddedSCRAgent>(stem, stem);			
	embedded->through = node;
	return embedded;	
}


void EngineCommand::Execute(const ScriptKit &kit) const
{
	auto step = make_shared<VNStep>();
	step->Configure(VNStep::COMPARE_REPLACE, stem);
	kit.step_sequence->push_back( step );
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

PatternCommand::PatternCommand( TreePtr<Node> pattern_ ) :
	pattern( pattern_ )
{
	ASSERT(pattern);
}


TreePtr<Node> PatternCommand::Decay( TreePtr<Node> node, VNParse *vn )
{
	ASSERT( !node )("Unexpected extra pattern " + pattern.GetTrace() + " in command production");
	return pattern; 
}


TreePtr<Node> PatternCommand::GetPattern() const
{
	return pattern;
}


string PatternCommand::GetTrace() const
{
	return "PatternCommand: " + Trace(pattern);
}

//////////////////////////// Designation ///////////////////////////////

Designation::Designation( std::wstring name_, TreePtr<Node> pattern_ ) :
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
	return "Designation: " + Trace(name) + "≝" + Trace(pattern);
}
