#ifndef VN_COMMANDS_HPP
#define VN_COMMANDS_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include <any> // to dep-break the generated headers

// Dep-break NodeNames because node_names.hpp will be big
class NodeNames;

namespace VN 
{
class VNParse;
class VNStep;
struct ScriptKit
{
	vector< shared_ptr<VN::VNStep> > *step_sequence;
};

class Command : public Traceable
{
public:	
	typedef std::list<shared_ptr<Command>> List; 

    ~Command();
	virtual bool OnParse( VNParse *vn ); // true to keep for execution, false to discard
	virtual void Execute( const ScriptKit &kit ) const;
};


class ScriptEngine
{
public:		
	void DoExecute( const ScriptKit &kit, Command::List script );	
};


class PatternCommand : public Command
{
public:	
	struct PureEngine : Node
	{
		NODE_FUNCTIONS_FINAL
		TreePtr<Node> stem;
	};
	
	PatternCommand( TreePtr<Node> pattern_ );
	void Execute( const ScriptKit &kit ) const final;
	
	TreePtr<Node> GetPattern() const;
	
	string GetTrace() const final;
	
private:
	const TreePtr<Node> pattern;
};


class Designation : public Command
{
public:	
	Designation( std::string name_, TreePtr<Node> pattern_ );
	bool OnParse(VNParse *vn) final;

	string GetTrace() const final;

private:
	const std::string name;
	const TreePtr<Node> pattern;
};	 	
	 	
};

#endif

