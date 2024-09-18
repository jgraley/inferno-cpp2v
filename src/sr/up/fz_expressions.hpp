#ifndef FZ_EXPRESSIONS_HPP
#define FZ_EXPRESSIONS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"

namespace SR 
{
class XTreeDatabase;
	
namespace UP
{
	struct ExecKit
    {
		// Note: unlike kits in sym, these pointers are non-const
		// because we intend to actually change things here.
        XTreeDatabase *x_tree_db; 

        // For embedded patterns
        const SCREngine *scr_engine;
	};
}

// ------------------------- FreeZoneExpression --------------------------

class FreeZoneExpression : public Traceable
{
public:	
	static void ForDepthFirstWalk( const FreeZoneExpression *base,
	                               function<void(const FreeZoneExpression *cmd)> func_in,
		                           function<void(const FreeZoneExpression *cmd)> func_out );
	virtual void DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                        function<void(const FreeZoneExpression *cmd)> func_out) const { ASSERTFAIL(); }
	virtual unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const { ASSERTFAIL(); }
};

// ------------------------- PopulateZoneOperator --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateZoneOperator : public FreeZoneExpression
{
protected:
    PopulateZoneOperator( vector<unique_ptr<FreeZoneExpression>> &&child_expressions_ );
    PopulateZoneOperator();

public:
    void AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent );
    
    //const Zone *GetZone() const;
    int GetNumChildExpressions() const;
    
	void DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                function<void(const FreeZoneExpression *cmd)> func_out) const final;

	void PopulateFreeZone( FreeZone &free_zone, const UP::ExecKit &kit ) const;	

	string GetTrace() const final;

private:
	unique_ptr<Zone> zone;
	vector<unique_ptr<FreeZoneExpression>> child_expressions;
	std::list<RequiresSubordinateSCREngine *> embedded_agents;
};


// ------------------------- PopulateTreeZoneOperator --------------------------

// Construct with tree zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateTreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_, vector<unique_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_ );
    
    const TreeZone *GetZone() const;
	unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const final;	
    
private:
	unique_ptr<TreeZone> zone;
};

// ------------------------- PopulateFreeZoneOperator --------------------------

// Construct with free zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateFreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_, vector<unique_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_ );

    const FreeZone *GetZone() const;
   	unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const final;	

private:
	unique_ptr<FreeZone> zone;
};

}

#endif
