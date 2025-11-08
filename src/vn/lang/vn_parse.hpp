#ifndef VN_PARSE_HPP
#define VN_PARSE_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include <any> // to dep-break the generated headers

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

// Dep-break NodeNames because node_names.hpp will be big
class NodeNames;

namespace VN 
{
class VNParse;
typedef TreePtr<Node> Production;	

class Command : public Traceable
{
public:	
    ~Command();
	virtual bool OnParse(VNParse *vn);
};


class PatternCommand : public Command
{
public:	
	PatternCommand( TreePtr<Node> pattern_ );
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
	
 
class VNParse	
{
public:
	VNParse();
	~VNParse();
	
	TreePtr<Node> DoParse(string filepath);
	
	struct PureEngine : Node
	{
		NODE_FUNCTIONS_FINAL
		TreePtr<Node> stem;
	};	 
	
	void OnError();
	void OnVNScript( list<shared_ptr<Command>> commands_ );
	shared_ptr<Command> OnCommand( shared_ptr<Command> command );

	Production OnEngine( Production stem );
	Production OnStuff( Production terminus );
	Production OnDelta( Production through, Production overlay );
	Production OnRestrict( any loc, wstring type, Production target ) {(void)loc; (void)type;return target;};
	Production OnRestrict( list<string> res_type, any res_loc, Production target, any target_loc );
	
	Production OnPrefixOperator( string tok, Production operand );
	Production OnPostfixOperator( string tok, Production operand );
	Production OnInfixOperator( string tok, Production left, Production right );
	Production OnSpecificInteger( int value );

private: 
	unique_ptr<YY::VNLangScanner> scanner;
	unique_ptr<YY::VNLangParser> parser;
	unique_ptr<NodeNames> node_names;
	
	bool saw_error;
	TreePtr<Node> pattern;	
	list<shared_ptr<Command>> commands;
};
	
};

#endif

