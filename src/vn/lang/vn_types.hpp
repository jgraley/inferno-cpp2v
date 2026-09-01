#ifndef VN_TYPES_HPP
#define VN_TYPES_HPP

// Watch the deps here: parser header file includes this
#include "node/tree_ptr.hpp"

#include <any> // to dep-break the generated headers

struct ANDBlock;

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

class Gnomon : public Traceable
{
public:
	virtual ~Gnomon();
};


class ScopeGnomon : public Gnomon
{
public:	
	virtual string GetMessageText() const = 0;	
	virtual TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const;
};


class RegularScopeGnomon : public ScopeGnomon
{
public:	
	RegularScopeGnomon( TreePtr<Node> node_ );
	string GetMessageText() const final;
	TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const override;

	TreePtr<Node> node;
};


class ParameterisationScopeGnomon : public ScopeGnomon
{
public:	
	string GetMessageText() const final;
	TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const override;
};


class RecordScopeGnomon : public ScopeGnomon
{
public:	
	RecordScopeGnomon( TreePtr<Node> initial_access, TreePtr<Node> record_type_ );
	string GetMessageText() const final;
	TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const override;

	TreePtr<Node> current_access; // changes as we parse the scope
	TreePtr<Node> record_type;
};


class UnknownScopeGnomon : public ScopeGnomon
{
public:	
	UnknownScopeGnomon(string reason_);
	string GetMessageText() const final;
	TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const override;

	const string reason;
};


class PrerestrictScopeGnomon : public ScopeGnomon
{
public:	
	PrerestrictScopeGnomon( TreePtr<Node> node_ );
	string GetMessageText() const final;
	TreePtr<Node> GetDeclarationNode(any loc, bool static_keyword_specified) const override;

	TreePtr<Node> node;
};


enum class QualCat
{
	UNDEFINED, 
	NODE, // See the node itself
	STATIC,
	TYPEDEF
};

struct QualifierData : Traceable
{
	QualifierData(  ) : cat(QualCat::UNDEFINED), node(nullptr) {}

	QualifierData( any loc_, QualCat cat_, TreePtr<Node> node_=nullptr ) :
		loc(loc_),
		cat(cat_),
		node(node_)
	{
	}
	
	string GetTrace() const final
	{
		switch( cat )
		{
			case QualCat::STATIC:
				return "STATIC";
			case QualCat::TYPEDEF:
				return "TYPEDEF";
			case QualCat::NODE:
				return Trace(node);
			case QualCat::UNDEFINED:
				ASSERTFAIL();
		}
		ASSERTFAIL();
	}
	string GetDiagnostic() const
	{
		switch( cat )
		{
			case QualCat::STATIC:
				return "static";
			case QualCat::TYPEDEF:
				return "typedef";
			case QualCat::NODE:
				return Traceable::TypeIdName( *node );
			case QualCat::UNDEFINED:
				ASSERTFAIL();
		}
		ASSERTFAIL();
	}
	
	any loc;	
	QualCat cat;
	TreePtr<Node> node;
};

struct BlockAndGnomon
{
	const ANDBlock *block;
	shared_ptr<Gnomon> gnomon;
};

struct NodeAndGnomon
{
	TreePtr<Node> node;
	shared_ptr<Gnomon> gnomon;
};

	
};

#endif

