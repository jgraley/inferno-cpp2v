#ifndef VN_ACTIONS_HPP
#define VN_ACTIONS_HPP

#include "indenter.hpp"
#include "tree/node_names.hpp"
#include "declarators.hpp"
#include "vn/lang/vn_types.hpp"     
#include <any> // to dep-break the generated headers

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

namespace VN 
{
class Command; // TODO don't fwd in a header
class VNLangRecogniser;

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
	TreePtr<Node> FinishExplicitNode( TreePtr<Node> dest, any node_name_loc, Itemisation itemisation );
	TreePtr<Node> OnEmbeddedCommands( list<shared_ptr<Command>> commands );
	TreePtr<Node> OnRestrict( const ANDBlock *block, any node_name_loc, TreePtr<Node> target, any target_loc );
	
	TreePtr<Node> OnTypeSpecifierSeq( multiset<string> specifiers, any loc );
	
	TreePtr<Node> OnInfixOperator( string tok, TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnPrefixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnPostfixOperator( string tok, TreePtr<Node> operand );
	TreePtr<Node> OnIntegralLiteral( string text, any loc ); 
	TreePtr<Node> OnStringLiteral( wstring value );
	TreePtr<Node> OnBoolLiteral( string value );
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
	TreePtr<Node> OnArgsBodyWithElseStmt( string keyword, any keyword_loc, 
	                                   list<TreePtr<Node>> args, any args_loc, 
	                                   TreePtr<Node> body, any body_loc,
	                                   any else_loc, 
	                                   TreePtr<Node> body_else, any else_body_loc );
	TreePtr<Node> OnWhile( TreePtr<Node> arg, any arg_loc, TreePtr<Node> body, any body_loc );
	TreePtr<Node> OnDo( TreePtr<Node> body, any body_loc, TreePtr<Node> arg, any arg_loc );
	
	TreePtr<Node> OnFunction( TreePtr<Node> return_type, list<TreePtr<Node>> params );	
	TreePtr<Node> OnConstructorType( list<TreePtr<Node>> params );	
	TreePtr<Node> NodeFromANDataBlock( const ANDBlock *block ) const;
	NodeAndGnomon MakeScopeGnomonForNode( TreePtr<Node> node ) const;
	TreePtr<Node> OnDeclaratorDecl( const list<QualifierData> &quals, TreePtr<Node> type, any type_loc, TreePtr<Node> declarator, any decl_loc );
	TreePtr<Node> OnTypedef( const list<QualifierData> &quals, Declarators::Result declarator_result, any middle_loc );	
	TreePtr<Node> OnInstance( const list<QualifierData> &quals, Declarators::Result declarator_result, any middle_loc );	
	TreePtr<Node> OnEnumerator( any loc, TreePtr<Node> id );
	TreePtr<Node> OnConstructorDecl( any loc, const list<QualifierData> &quals, TreePtr<Node> id, list<TreePtr<Node>> params );	
	TreePtr<Node> ApplyAccessSpec( TreePtr<Node> declaration, any loc, TreePtr<Node> access );	
	TreePtr<Node> ApplyInitialiser( TreePtr<Node> declaration, any instance_loc, TreePtr<Node> init, any init_loc );	
	TreePtr<Node> OnMemberInitialiser( TreePtr<Node> member_id, any member_loc, TreePtr<Node> initialiser, any initialiser_loc );
	TreePtr<Node> ApplyMemberInits( TreePtr<Node> instance, any instance_loc, list<TreePtr<Node>> memb_inits, any memb_inits_loc );	
	TreePtr<Node> OnAbDeclType( any loc, const list<QualifierData> &quals, TreePtr<Node> type, TreePtr<Node> declarator );	
	TreePtr<Node> StartRecord( any loc, string keyword );
	TreePtr<Node> ApplyIdentifier( TreePtr<Node> record, any loc, TreePtr<Node> id );
	shared_ptr<Gnomon> MakeRecordScopeGnomon( TreePtr<Node> record, TreePtr<Node> type );
	TreePtr<Node> FinishRecord( any loc, TreePtr<Node> node, list<TreePtr<Node>> bases, list<TreePtr<Node>> members );	
	TreePtr<Node> OnBase( TreePtr<Node> access, TreePtr<Node> type, any loc );	
	TreePtr<Node> OnBase( TreePtr<Node> type );	// Access not specified
	TreePtr<Node> OnQualifierNodeKeyword( string keyword );
	Declarators::CVQuals OnCVQuals( const list<QualifierData> &quals, bool nice=false );
	
	TreePtr<Node> OnIdValuePair( TreePtr<Node> id, any id_loc, TreePtr<Node> value );
	TreePtr<Node> OnMapArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnSeqArgsCall( TreePtr<Node> callee, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnMapArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnSeqArgsConsInit( TreePtr<Node> constructor_id, list<TreePtr<Node>> arguments );
	TreePtr<Node> OnLookup( TreePtr<Node> object, TreePtr<Node> member, any member_loc );

	TreePtr<Node> OnIdByName( const ANDBlock *block, any id_disc_loc, wstring wname, any name_loc );
	TreePtr<Node> OnBuildId( const ANDBlock *block, any id_disc_loc, wstring wformat, any name_loc, Item sources );
	TreePtr<Node> OnTransform( string kind, any kind_loc, TreePtr<Node> pattern, any pattern_loc );
	TreePtr<Node> OnNegation( TreePtr<Node> operand );
	TreePtr<Node> OnConjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnDisjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnGrass( TreePtr<Node> through );
	TreePtr<Node> OnPointerIs( TreePtr<Node> pointer );
	TreePtr<Node> OnBuildSize( TreePtr<Node> container );
	TreePtr<Node> OnStringize( TreePtr<Node> source );
	TreePtr<Node> OnNeedSoloStatement( list<TreePtr<Node>> source, any loc );
	
	TreePtr<Node> CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, any loc );
	
	void AddGnomon( shared_ptr<Gnomon> gnomon );
	
private: 
	unique_ptr<AvailableNodeData> node_names;	
	
	// store with weak_ptr => these will expire when the parser exists the scope
	WeakStack<ScopeGnomon> declaration_scope_gnomons;
	
public: // TODO provide a getter	
	list<shared_ptr<Command>> top_level_commands;	
};
	
};

#endif

