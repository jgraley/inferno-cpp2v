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

class NodeNames;

namespace VN 
{
	
class VNParse;	
class Gnomon : public Traceable
{
public:
	Gnomon( list<string> resolution_ ) : 
		resolution(resolution_)
	{ 
		FTRACE("Gnomon at %p constructed with resolution: ", this)(resolution)("\n");
	}
	~Gnomon()
	{ 
		FTRACE("Gnomon at %p destructed, resolution was: ", this)(resolution)("\n");
	}

	string GetTrace() const
	{
		return Trace(resolution);
	}
	
	friend class VNShim;
private:
	list<string> resolution;
};

		
class VNShim
{
public:	
	shared_ptr<Gnomon> PushScopeRes( list<string> resolution );

	void Designate( wstring name, TreePtr<Node> sub_pattern );

	
	YY::VNLangParser::symbol_type OnUnquoted(string text, YY::VNLangParser::location_type loc) const;
	YY::VNLangParser::symbol_type OnUnquoted(wstring text, YY::VNLangParser::location_type loc) const;
	YY::VNLangParser::symbol_type ProcessToken(wstring text, bool ascii, YY::VNLangParser::location_type loc) const;
	
	TreePtr<Node> TryGetNamedSubtree(wstring name) const;	
	TreePtr<Node> TryGetArchetype(list<string> typ) const;

private:	
	void PurgeExpiredGnomons();

	map<wstring, TreePtr<Node>> designations;	
	list<weak_ptr<Gnomon>> current_gnomons;
};
	
};

#endif

