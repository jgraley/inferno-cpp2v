#ifndef VN_ACTIONS_HPP
#define VN_ACTIONS_HPP

#include "indenter.hpp"
#include "tree/node_names.hpp"
#include "declarators.hpp"
#include "vn/lang/vn_types.hpp"     
#include "node/syntax.hpp"     

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
	TreePtr<Node> FinishExplicitNode( TreePtr<Node> dest, Syntax::Location node_name_loc, Itemisation itemisation );
	TreePtr<Node> OnEmbeddedCommands( list<shared_ptr<Command>> commands );
	TreePtr<Node> OnRestrict( TreePtr<Node> node, Syntax::Location node_name_loc, TreePtr<Node> target, Syntax::Location target_loc );
	
	TreePtr<Node> OnTypeSpecifierSeq( multiset<string> specifiers, Syntax::Location loc );
	
	TreePtr<Node> OnInfixOperator( string tok );
	TreePtr<Node> OnPrefixOperator( string tok );
	TreePtr<Node> OnPostfixOperator( string tok );
	TreePtr<Node> OnIntegralLiteral( string text, Syntax::Location loc ); 
	TreePtr<Node> OnStringLiteral( wstring value );
	TreePtr<Node> OnConditionalOperator();	
	TreePtr<Node> OnSubscript();	
	TreePtr<Node> OnCompound( list<TreePtr<Node>> members, Syntax::Location memb_loc, list<TreePtr<Node>> statements, Syntax::Location stmt_loc );	
	TreePtr<Node> OnCodeUnit( list<TreePtr<Node>> members, Syntax::Location memb_loc );	
	TreePtr<Node> OnArrayInitialiser();	
	TreePtr<Node> OnLabel( TreePtr<Node> identifier, Syntax::Location loc );	
	TreePtr<Node> OnCase( TreePtr<Node> value, Syntax::Location loc );	
	TreePtr<Node> OnRangeCase( TreePtr<Node> value_lo, Syntax::Location lo_loc, TreePtr<Node> value_hi, Syntax::Location hi_loc );	
	TreePtr<Node> OnDefault( Syntax::Location loc );	
	TreePtr<Node> OnNop( Syntax::Location loc );	
	TreePtr<Node> OnFuncOnType( string keyword, Syntax::Location keyword_loc, TreePtr<Node> type_arg, Syntax::Location type_arg_loc );
			
	TreePtr<Node> OnFunction( TreePtr<Node> return_type, list<TreePtr<Node>> params );	
	NodeAndGnomon MakeScopeGnomonForNode( TreePtr<Node> node ) const;
	TreePtr<Node> OnDeclaratorDecl( const list<QualifierData> &quals, TreePtr<Node> type, Syntax::Location type_loc, TreePtr<Node> declarator, Syntax::Location decl_loc );
	TreePtr<Node> OnInstance( const list<QualifierData> &quals, Declarators::Result declarator_result, Syntax::Location middle_loc );	
	TreePtr<Node> OnEnumerator( Syntax::Location loc );
	TreePtr<Node> OnConstructor( Syntax::Location loc, const list<QualifierData> &quals, list<TreePtr<Node>> params );	
	void UpdateCurrentAccess( Syntax::Location loc, TreePtr<Node> access );	
	TreePtr<Node> OnMemberInitialiser( TreePtr<Node> member_id, Syntax::Location member_loc, TreePtr<Node> initialiser, Syntax::Location initialiser_loc );
	TreePtr<Node> OnAbDeclType( Syntax::Location loc, const list<QualifierData> &quals, TreePtr<Node> type, TreePtr<Node> declarator );	
	shared_ptr<Gnomon> MakeRecordScopeGnomon( TreePtr<Node> record );
	TreePtr<Node> OnBase( TreePtr<Node> access, TreePtr<Node> type, Syntax::Location loc );	
	TreePtr<Node> OnBase( TreePtr<Node> type );	// Access not specified
	Declarators::CVQuals OnCVQuals( const list<QualifierData> &quals, bool nice=false );
	
	TreePtr<Node> OnIdValuePair();
	TreePtr<Node> OnMapArgs();
	TreePtr<Node> OnSeqArgs();
	TreePtr<Node> OnCall();
	TreePtr<Node> OnLookup();

	TreePtr<Node> OnIdByName( TreePtr<Node> node, Syntax::Location id_disc_loc, wstring wname, Syntax::Location name_loc );
	TreePtr<Node> OnBuildId( TreePtr<Node> node, Syntax::Location id_disc_loc, wstring wformat, Syntax::Location name_loc, Item sources );
	TreePtr<Node> OnTransform( string kind, Syntax::Location kind_loc, TreePtr<Node> pattern, Syntax::Location pattern_loc );
	TreePtr<Node> OnNegation( TreePtr<Node> operand );
	TreePtr<Node> OnConjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnDisjunction( TreePtr<Node> left, TreePtr<Node> right );
	TreePtr<Node> OnGrass( TreePtr<Node> through );
	TreePtr<Node> OnPointerIs( TreePtr<Node> pointer );
	TreePtr<Node> OnBuildSize( TreePtr<Node> container );
	TreePtr<Node> OnStringize( TreePtr<Node> source );
	TreePtr<Node> OnNeedSoloStatement( list<TreePtr<Node>> source, Syntax::Location loc );
	
	TreePtr<Node> CreateIntegralLiteral( bool uns, bool lng, bool lng2, uint64_t val, Syntax::Location loc );
	
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

