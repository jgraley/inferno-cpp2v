#ifndef VN_SHIM_HPP
#define VN_SHIM_HPP

#include "tree/node_names.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include "vn/lang/vn_types.hpp"     
#include "vn/lang/vn_lang.ypp.hpp"
#include "vn/lang/vn_lang.lpp.hpp"
#include "vn/lang/vn_lang.location.hpp"
#include "vn_actions.hpp"

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

class AvailableNodeData;

namespace VN 
{
class VNLangActions;	
		
class DesignationGnomon : public Gnomon
{
public:	
	DesignationGnomon( std::wstring name_, TreePtr<Node> node_, Syntax::Token token_ );
	string GetTrace() const;
	
private:
	friend class VNLangRecogniser;
	std::wstring name;
	TreePtr<Node> node;
	Syntax::Token token;
};


class TransformNameScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final;
};	
		
		
class VNLangRecogniser
{
public:	
	void AddGnomon( shared_ptr<Gnomon> gnomon );

	void Designate( wstring name, TreePtr<Node> sub_pattern );
	
	YY::VNLangParser::symbol_type OnUnquotedLexeme(string text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnUnquotedLexeme(wstring text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnExplicitLexeme(wstring text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnPrerestrictLexeme(wstring text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnIdByNameLexeme(wstring text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnIdBuilderLexeme(wstring text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type OnTransformLexeme(wstring text, Syntax::Location loc) const;
	
private:
	TreePtr<Node> CreateNodeFromName(string text, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type Recognise(wstring text, bool ascii, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type RecogniseInNodeNameScope(wstring text, bool ascii, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type CreateBlockToken(const ANDBlock *block, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type CreateNodeToken(const AvailableNodeData::NodeBlock *block, Syntax::Location loc) const;
    YY::VNLangParser::symbol_type RecogniseKeyword(wstring text, bool ascii, Syntax::Location loc) const;
	YY::VNLangParser::symbol_type RecogniseDesignation(wstring text, Syntax::Location loc) const;

	class Unrecognised : Exception {};
		
	// Store with shared_ptr => these will stick around until we ditch them
	map<wstring, shared_ptr<const DesignationGnomon>> designation_gnomons;
};
	
};

#endif

