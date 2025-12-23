#ifndef VN_SHIM_HPP
#define VN_SHIM_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"

// WHAT WE NEED
// A "name info" struct containing a variety of handy info relating to a name we saw:
// - unicode/ASCII flag
// - A subtree if the name was designated
// - An archetype if the type is known
// - An enum of TYPE, OTHER etc which has been deduced from the above according to some priority scheme
// It is this enum that the scanner will use to decide what token to issue
// Alternatively, WE could choose the token and put that in the struct instead.

namespace YY
{
	class VNLangScanner;
	class VNLangParser;
};

class AvailableNodeData;

namespace VN 
{
class VNLangActions;	
class Gnomon : public Traceable
{
public:
	virtual ~Gnomon()
	{ 
	}
};


class ResolverGnomon : public Gnomon
{
public:	
	ResolverGnomon( const AvailableNodeData::Block *andata_block ) : 
		namespace_block(dynamic_cast<const AvailableNodeData::NamespaceBlock *>(andata_block))
	{
		ASSERT( namespace_block );
	}

	string GetTrace() const
	{
		return Trace(namespace_block);
	}
	
private:
	friend class VNLangRecogniser;
	const AvailableNodeData::NamespaceBlock * const namespace_block;
};
		
		
class DesignationGnomon : public Gnomon
{
public:	
	DesignationGnomon( std::wstring name_, TreePtr<Node> pattern_ ) : 
		name( name_ ),
		pattern( pattern_ )	
	{
		ASSERT( !name.empty() );
		// Pattern can be NULL for eg singular wildcard
	}

	string GetTrace() const
	{
		return ToASCII(name) + "⪮" + Trace(pattern);
	}
	
private:
	friend class VNLangRecogniser;
	std::wstring name;
	TreePtr<Node> pattern;
};
		
		
class TypeDesignationGnomon : public DesignationGnomon
{
public:	
	using DesignationGnomon::DesignationGnomon;
};
		
		
class NonTypeDesignationGnomon : public DesignationGnomon
{
public:	
	using DesignationGnomon::DesignationGnomon;
};
		
		
class ScopeGnomon : public Gnomon
{
public:	
	virtual string GetMessageText() const = 0;	
};


class NodeNameScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "node name scope";
	}
};
		
		
class IdentifierDiscriminatorScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "identifier discriminator scope";
	}
};		

class TransformNameScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final
	{
		return "transformation name scope";
	}
};
		
		
class VNLangRecogniser
{
public:	
	void AddGnomon( shared_ptr<Gnomon> gnomon );

	void Designate( wstring name, TreePtr<Node> sub_pattern );
	
	YY::VNLangParser::symbol_type OnUnquoted(string text, YY::VNLangParser::location_type loc) const;
	YY::VNLangParser::symbol_type OnUnquoted(wstring text, YY::VNLangParser::location_type loc) const;
	YY::VNLangParser::symbol_type ProcessToken(wstring text, bool ascii, YY::VNLangParser::location_type loc) const;
	YY::VNLangParser::symbol_type ProcessTokenInNodeNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const;
	YY::VNLangParser::symbol_type ProcessTokenInIdentifierDiscriminatorScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const;
	YY::VNLangParser::symbol_type ProcessTokenTransformNameScope(wstring text, bool ascii, YY::VNLangParser::location_type loc, YY::TokenMetadata metadata) const;

private:	
	void PurgeExpiredGnomons();
	string GetContextText() const;

	// store with weak_ptr => these will expire when the parser exists the scope
	list<weak_ptr<const ScopeGnomon>> scope_gnomons;

	// store with weak_ptr => these will expire when the parser exists the resolver production
	list<weak_ptr<const ResolverGnomon>> resolver_gnomons;
	
	// Store with shared_ptr => these will stick around until we ditch them
	map<wstring, shared_ptr<const DesignationGnomon>> designation_gnomons;
};
	
};

#endif

