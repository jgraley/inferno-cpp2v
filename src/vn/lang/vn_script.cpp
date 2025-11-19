#include "vn_script.hpp"

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
#include "agents/embedded_scr_agent.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace CPPTree; 
using namespace VN;


//////////////////////////// VNSoftStep ///////////////////////////////

VNSoftStep::VNSoftStep( string name )
{
	step_name = name;
}

string VNSoftStep::GetName() const
{
	return step_name;
}

//////////////////////////// VNScriptRunner ///////////////////////////////

VNScriptRunner::VNScriptRunner( vector< shared_ptr<VN::VNStep> > *sequence_ ) :
	sequence(sequence_)
{
}


void VNScriptRunner::AddStep(const VN::ScriptKit &kit, TreePtr<Node> stem)
{
	filesystem::path path( kit.script_filepath );
	string basename = path.replace_extension().filename().string();
#ifdef ELIMINATE_STEP_NUMBER
	int s;
	char c;
	int n = sscanf( basename.c_str(), "%003d-%c", &s, &c );
	if( n==2 )
		basename = basename.substr(4);
#endif

	auto step = make_shared<VNSoftStep>(basename);
	step->Configure(VNStep::COMPARE_REPLACE, stem);
	kit.step_sequence->push_back( step );
}


void VNScriptRunner::ProcessVNPath( string spath )
{
	filesystem::path path(spath);
	if( filesystem::is_directory(path) )
	{
		set<string> ss;
		for( const filesystem::directory_entry &entry : filesystem::directory_iterator(path) )
			ss.insert(entry.path()); // These won't be sorted (this isn't "ls")
			
		// Process the steps in lexigographical order so they can be prefixed with 000-, 001- etc
		for( string s : ss )
			ProcessVNPath( s );
	}
	else
	{
		ProcessVNFile(spath);
	}
}


void VNScriptRunner::ProcessVNFile(string spath )
{
	VNParse vn_parser;
	VN::Command::List script = vn_parser.DoParse(spath);
	if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Read %s ok\n", spath.c_str()); 
	RunScript( spath, script );
}


void VNScriptRunner::RunScript( string spath, Command::List script )
{
	VN::ScriptKit kit{ this, sequence, spath };
	for( shared_ptr<Command> c : script )
	{
		c->Execute(kit);
	}
}
