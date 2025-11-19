#ifndef VN_SCRIPT_HPP
#define VN_SCRIPT_HPP

#include "vn_commands.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include "vn/vn_step.hpp"
#include <any> // to dep-break the generated headers

// Dep-break NodeNames because node_names.hpp will be big
class NodeNames;

namespace VN 
{
class VNParse;
class VNStep;

class VNSoftStep : public VNStep
{
public:	
	VNSoftStep( string step_name_ );
	string GetName() const final;

private:
	string step_name;
};




class VNScriptRunner
{
public:		
	VNScriptRunner( vector< shared_ptr<VN::VNStep> > *sequence_ );
	void AddStep(const VN::ScriptKit &kit, TreePtr<Node> stem);
	void ProcessVNPath( string spath );

private:
	void ProcessVNFile( string spath );
	void RunScript( string spath, Command::List script );	

	vector< shared_ptr<VN::VNStep> > *sequence;

};



	 	 	
};

#endif

