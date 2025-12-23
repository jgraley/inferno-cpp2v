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

// Dep-break AvailableNodeData because node_names.hpp will be big
class AvailableNodeData;

namespace VN 
{
class VNLangActions;
class VNStep;
struct ScriptKit
{
	class VNScript *vn_script;
	vector< shared_ptr<VN::VNStep> > *step_sequence;
	string script_filepath;
};

class Command : public Traceable
{
public:	
	typedef std::list<shared_ptr<Command>> List; 
	Command( any loc_ );
    ~Command();

	// Called on a sequence of commands when usage dictates it should become a sub-pattern 
	virtual TreePtr<Node> DecayToPattern( TreePtr<Node> node, VNLangActions *vn ); // return NULL if cannot decay 
	
	// Called on a sequence of commands when usage dictates it should be part of a script
	virtual void Execute( const ScriptKit &kit ) const;

protected:
	const any loc;
};


class EngineCommand : public Command
{
public:	
	EngineCommand( TreePtr<Node> pattern_, any loc );
	TreePtr<Node> DecayToPattern( TreePtr<Node> node, VNLangActions *vn ) final; 
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
	TreePtr<Node> DecayToPattern( TreePtr<Node> node, VNLangActions *vn ) final; 
		
	string GetTrace() const final;
	
private:
	const TreePtr<Node> pattern;
};

	
class AttributeCommand : public Command
{
public:	
	AttributeCommand( any loc );
	TreePtr<Node> DecayToPattern( TreePtr<Node> node, VNLangActions *vn ) final; 
	void Execute( const ScriptKit &kit ) const final;

	string GetTrace() const final;	
};
	
};

#endif
