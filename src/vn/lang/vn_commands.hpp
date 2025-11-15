#ifndef VN_COMMANDS_HPP
#define VN_COMMANDS_HPP

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
struct ScriptKit
{
	vector< shared_ptr<VN::VNStep> > *step_sequence;
	string script_filepath;
};

class Command : public Traceable
{
public:	
	typedef std::list<shared_ptr<Command>> List; 
	Command( any loc_ );
    ~Command();
    
    // Called as soon as node is seen, so that parsing context can be updated
	virtual bool OnParse( VNParse *vn ); // true to keep for decay/execution, false to discard
	
	// Called on a sequence of commands when usage dictates it should become a sub-pattern 
	virtual TreePtr<Node> Decay( TreePtr<Node> node, VNParse *vn ); // return NULL if cannot decay 
	
	// Called on a sequence of commands when usage dictates it should be a script
	virtual void Execute( const ScriptKit &kit ) const;

protected:
	const any loc;
};


class ScriptEngine
{
public:		
	void DoExecute( const ScriptKit &kit, Command::List script );	
};


class VNSoftStep : public VNStep
{
public:	
	VNSoftStep( string step_name_ );
	string GetName() const final;

private:
	string step_name;
};


class EngineCommand : public Command
{
public:	
	EngineCommand( TreePtr<Node> pattern_, any loc );
	TreePtr<Node> Decay( TreePtr<Node> node, VNParse *vn ) final; 
	void Execute( const ScriptKit &kit ) const final;
	
	TreePtr<Node> GetPattern() const;
	
	string GetTrace() const final;
	
private:
	const TreePtr<Node> stem;
};


class PatternCommand : public Command
{
public:	
	PatternCommand( TreePtr<Node> pattern_, any loc );
	TreePtr<Node> Decay( TreePtr<Node> node, VNParse *vn ) final; 
		
	string GetTrace() const final;
	
private:
	const TreePtr<Node> pattern;
};


class Designation : public Command
{
public:	
	Designation( std::wstring name_, TreePtr<Node> pattern_, any loc );
	bool OnParse(VNParse *vn) final;

	string GetTrace() const final;

private:
	const std::wstring name;
	const TreePtr<Node> pattern;
};	 	
	 	 	
};

#endif

