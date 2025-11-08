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

void Command::Execute(const ScriptKit &kit) const
{
	ASSERTFAIL("Commands should either be discarded by OnParse() returning false, or implement Execute()")
}

//////////////////////////// ScriptEngine ///////////////////////////////

void ScriptEngine::DoExecute( const ScriptKit &kit, Command::List script )
{
	for( shared_ptr<Command> c : script )
	{
		c->Execute(kit);
	}
}

//////////////////////////// PatternCommand ///////////////////////////////

PatternCommand::PatternCommand( TreePtr<Node> pattern_ ) :
	pattern( pattern_ )
{
	ASSERT(pattern);
}


void PatternCommand::Execute(const ScriptKit &kit) const
{
	auto step = make_shared<VNStep>();
	auto pure_engine = TreePtr<PureEngine>::DynamicCast(pattern);
	ASSERT(pure_engine)( "Only a single pure engine supported rn, saw: ")(pattern)(" lol TODO\n");
	step->Configure(VNStep::COMPARE_REPLACE, pure_engine->stem);
	kit.step_sequence->push_back( step );
}


TreePtr<Node> PatternCommand::GetPattern() const
{
	return pattern;
}


string PatternCommand::GetTrace() const
{
	return "PatternCommand:" + Trace(pattern);
}

//////////////////////////// Designation ///////////////////////////////

Designation::Designation( std::wstring name_, TreePtr<Node> pattern_ ) :
	name( name_ ),
	pattern( pattern_ )
{
}


bool Designation::OnParse(VNParse *vn)
{
	// Teeing into parse class during parse so name is available
	//vn->OnDesignation(name, pattern);
	FTRACE(name)(" designates ")(pattern)(" TODO\n");
	
	return false; // discard
}


string Designation::GetTrace() const
{
	return "Designation:" + Trace(name) + "≝" + Trace(pattern);
}
