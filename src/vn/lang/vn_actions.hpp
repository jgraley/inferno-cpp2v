#ifndef VN_PARSE_HPP
#define VN_PARSE_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include "vn_commands.hpp"
#include "tree/node_names.hpp"
#include <any> // to dep-break the generated headers

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

namespace VN 
{
		
struct Limit
{
	string cond;
	any cond_loc;
	string num_text;
	any num_loc;
};
		
struct Item
{
	list<TreePtr<Node>> nodes;
	any loc;
};

struct Itemisation
{
	list<Item> items;
	any loc;
};


class Command;
class VNLangRecogniser;

class Gnomon : public Traceable
{
public:
	virtual ~Gnomon()
	{ 
	}
};


class ScopeGnomon : public Gnomon
{
public:	
	virtual string GetMessageText() const = 0;	
};


class ParameterisationScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "parameters scope";
	}
};

class RecordScopeGnomon : public ScopeGnomon
{
public:	
	RecordScopeGnomon( TreePtr<CPPTree::AccessSpec> initial_access ) :
		current_access( initial_access ) {}
	string GetMessageText() const final
	{
		return "fields scope";
	}
	TreePtr<CPPTree::AccessSpec> current_access;
};

class EnumeratorScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "enumerators scope";
	}
};

class CodeUnitScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "global scope";
	}
};

class CompoundScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "local scope";
	}
};

class UnknownScopeGnomon : public ScopeGnomon
{
public:	
	UnknownScopeGnomon(string reason_) :
		reason( reason_ ) {}
	string GetMessageText() const final
	{
		return "local scope";
	}
	const string reason;
};

class PrerestrictScopeGnomon : public ScopeGnomon
{
public:	
	PrerestrictScopeGnomon( const AvailableNodeData::Block *block_ ) :
		block(block_) {}
	string GetMessageText() const final
	{
		return "prerestrict scope";
	}
	const AvailableNodeData::Block * const block;
};


enum class QualCat
{
	NODE, // See the node itself
	STATIC
};

struct QualifierData : Traceable
{
	TreePtr<Node> node;
	QualCat cat;
	
	string GetTrace() const final
	{
		switch( cat )
		{
			case QualCat::STATIC:
				return "STATIC";
			case QualCat::NODE:
				return Trace(node);
		}
		ASSERTFAIL();
	}
};

struct BlockAndGnomon
{
	const AvailableNodeData::Block *block;
	shared_ptr<Gnomon> gnomon;
};

struct NodeAndGnomon
{
	TreePtr<Node> node;
	shared_ptr<Gnomon> gnomon;
};

class VNLangActions	
{
public:
	VNLangActions();
	~VNLangActions();
		
	void OnError();
	void OnVNScript( list<shared_ptr<Command>> commands_ );
	shared_ptr<Command> OnCommand( shared_ptr<Command> command );

	TreePtr<Node> OnStar( TreePtr<Node> restriction );
	TreePtr<Node> OnStuff( TreePtr<Node> terminus, TreePtr<Node> recurse_restriction, Limit limit );
	TreePtr<Node> OnDelta( TreePtr<Node> through, TreePtr<Node> overlay );
	TreePtr<Node> OnExplicitNode( const AvailableNodeData::Block *block, any node_name_loc, Itemisation itemisation );
	TreePtr<Node> OnEmbeddedCommands( list<shared_ptr<Command>> commands );
	TreePtr<Node> OnRestrict( const AvailableNodeData::Block *block, any node_name_loc, TreePtr<Node> target, any target_loc );
	
	TreePtr<Node> OnTypeSpecifierSeq( multiset<string> specifiers, any loc );
	
	TreePtr<Node> OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnPrefixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnPostfixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnIntegralLiteral( string text, any loc ); 
	TreePtr<Node> OnStringLiteral( wstring value );
	TreePtr<Node> OnBoolLiteral( bool value );
	TreePtr<Node> OnCast( TreePtr<Node> type, any type_loc, TreePtr<Node> target, any target_loc );	
	TreePtr<Node> OnConditionalOperator( TreePtr<Node> condition, TreePtr<Node> expr_then, TreePtr<Node> expr_else );	
	TreePtr<Node> OnSubscript( TreePtr<Node> destination, TreePtr<Node> index );	
	TreePtr<Node> OnCompound( list<TreePtr<Node>> members, any memb_loc, list<TreePtr<Node>> statements, any stmt_loc );	
	TreePtr<Node> OnCodeUnit( list<TreePtr<Node>> members, any memb_loc );	
	TreePtr<Node> OnArrayInitialiser( list<TreePtr<Node>> elements );	
	TreePtr<Node> OnLabel( TreePtr<Node> identifier, any loc );	
	TreePtr<Node> OnCase( TreePtr<Node> value, any loc );	
	TreePtr<Node> OnRangeCase( TreePtr<Node> value_lo, any lo_loc, TreePtr<Node> value_hi, any hi_loc );	
	TreePtr<Node> OnDefault( any loc );	
	TreePtr<Node> OnNop( any loc );	
	TreePtr<Node> OnFuncOnType( string keyword, any keyword_loc, TreePtr<Node> type_arg, any type_arg_loc );
	
	TreePtr<Node> OnNormalTerminalKeyword( string keyword, any keyword_loc );
	TreePtr<Node> OnSimpleStmt( string keyword, any keyword_loc);
	TreePtr<Node> OnSpaceSepStmt( string keyword, any keyword_loc, TreePtr<Node> operand, any operand_loc );
	TreePtr<Node> OnArgsBodyStmt( string keyword, any keyword_loc, list<TreePtr<Node>> args, any args_loc, TreePtr<Node> body, any body_loc );
	TreePtr<Node> OnArgsBodyChainStmt( string keyword, any keyword_loc, 
	                                   list<TreePtr<Node>> args, any args_loc, 
	                                   TreePtr<Node> body, any body_loc,
	                                   string chain_keyword, any chain_keyword_loc, 
	                                   TreePtr<Node> chain_body, any chain_body_loc );
	TreePtr<Node> OnWhile( TreePtr<Node> arg, any arg_loc, TreePtr<Node> body, any body_loc );
	TreePtr<Node> OnDo( TreePtr<Node> body, any body_loc, TreePtr<Node> arg, any arg_loc );
	
	TreePtr<Node> OnFunction( TreePtr<Node> return_type, list<TreePtr<Node>> params );	
	TreePtr<Node> OnConstructorType( list<TreePtr<Node>> params );	
	BlockAndGnomon MakeScopeGnomonForNode( const AvailableNodeData::Block *block ) const;
	TreePtr<Node> OnInstance( any loc, const list<QualifierData> &quals_pre, TreePtr<Node> type, TreePtr<Node> declarator );	
	TreePtr<Node> OnConstructorInstance( any loc, const list<QualifierData> &quals_pre, TreePtr<Node> id, list<TreePtr<Node>> params );	
	void ApplyAccessSpec( TreePtr<Node> instance, any loc, TreePtr<Node> access );	
	void ApplyInitialiser( TreePtr<Node> instance, any instance_loc, TreePtr<Node> init );	
	TreePtr<Node> OnAbDeclType( TreePtr<Node> type, TreePtr<Node> declarator, any declarator_loc );	
	TreePtr<Node> StartRecord( any loc, string keyword );
	shared_ptr<Gnomon> MakeRecordScopeGnomon( TreePtr<Node> rec );
	TreePtr<Node> FinishRecord( any loc, TreePtr<Node> node, TreePtr<Node> id, list<TreePtr<Node>> bases, list<TreePtr<Node>> members );	
	TreePtr<Node> OnBase( TreePtr<Node> access, TreePtr<Node> type );	
	TreePtr<Node> OnBase( TreePtr<Node> type );	// Access not specified
	TreePtr<Node> OnQualifierNodeKeyword( string keyword );
	
	TreePtr<Node> OnIdValuePair( TreePtr<Node> id, any id_loc, TreePtr<Node> value );
	TreePtr<Node> OnMapArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnSeqArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnMapArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnSeqArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnLookup( TreePtr<Node> object, TreePtr<Node> member, any member_loc );

	TreePtr<Node> OnIdByName( const AvailableNodeData::Block *block, any id_disc_loc, wstring wname, any name_loc );
	TreePtr<Node> OnBuildId( const AvailableNodeData::Block *block, any id_disc_loc, wstring wformat, any name_loc, Item sources );
	TreePtr<Node> OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc );
	TreePtr<Node> OnNegation( TreePtr<Node> operand );
	TreePtr<Node> OnConjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnDisjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnGrass( TreePtr<Node> through );
	TreePtr<Node> OnPointerIs( TreePtr<Node> pointer );
	TreePtr<Node> OnBuildSize( TreePtr<Node> container );
	TreePtr<Node> OnStringize( TreePtr<Node> source );
	
	TreePtr<Node> CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, any loc );
	
	void AddGnomon( shared_ptr<Gnomon> gnomon );
	
private: 
	unique_ptr<AvailableNodeData> node_names;	
	
	// store with weak_ptr => these will expire when the parser exists the scope
	WeakStack<ScopeGnomon> declaration_scope_gnomons;
	
public: // TODO provide a getter	
	Command::List top_level_commands;	
};
	
};

#endif

