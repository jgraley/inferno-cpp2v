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

// ------------------------- FreeZoneExpression --------------------------

class FreeZoneExpression : public Traceable
{
public:	
	virtual unique_ptr<FreeZone> Evaluate() const = 0;
	virtual void ForChildren(function<void(shared_ptr<FreeZoneExpression> &expr)> func) = 0;
			                        
	static void ForDepthFirstWalk( shared_ptr<FreeZoneExpression> &base,
								   function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
								   function<void(shared_ptr<FreeZoneExpression> &expr)> func_out );

	virtual void DepthFirstWalkImpl(function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
			                        function<void(shared_ptr<FreeZoneExpression> &expr)> func_out) = 0;
};

// ------------------------- PopulateZoneOperator --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateZoneOperator : public FreeZoneExpression
{
protected:
    PopulateZoneOperator( const SCREngine *scr_engine_, list<shared_ptr<FreeZoneExpression>> &&child_expressions_ );
    PopulateZoneOperator( const SCREngine *scr_engine_ );

public:
    void AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker );
    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers );
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const;
    
    virtual Zone &GetZone() = 0;
    virtual const Zone &GetZone() const = 0;
    
    int GetNumChildExpressions() const;
	list<shared_ptr<FreeZoneExpression>> &GetChildExpressions();
	const list<shared_ptr<FreeZoneExpression>> &GetChildExpressions() const;
    string GetChildExpressionsTrace() const;

	void ForChildren(function<void(shared_ptr<FreeZoneExpression> &expr)> func) override;

	void EvaluateWithFreeZone( FreeZone &free_zone ) const;	
	
	void DepthFirstWalkImpl(function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
			                function<void(shared_ptr<FreeZoneExpression> &expr)> func_out) override;

protected:
	const SCREngine * const scr_engine;

private:
	list<shared_ptr<FreeZoneExpression>> child_expressions;
	list<RequiresSubordinateSCREngine *> embedded_markers;
};


// ------------------------- PopulateTreeZoneOperator --------------------------

// Construct with tree zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateTreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateTreeZoneOperator( const SCREngine *scr_engine, TreeZone zone_, list<shared_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateTreeZoneOperator( const SCREngine *scr_engine, TreeZone zone_ );
    
    TreeZone &GetZone() override;
    const TreeZone &GetZone() const override;
	
	unique_ptr<FreeZone> Evaluate() const final;	
    
    shared_ptr<FreeZoneExpression> DuplicateToFree() const;
    
	string GetTrace() const final;
    
private:
	TreeZone zone;
};

// ------------------------- PopulateFreeZoneOperator --------------------------

// Construct with free zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateFreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateFreeZoneOperator( const SCREngine *scr_engine, FreeZone zone_, list<shared_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateFreeZoneOperator( const SCREngine *scr_engine, FreeZone zone_ );

    FreeZone &GetZone() override;
    const FreeZone &GetZone() const override;
    
   	unique_ptr<FreeZone> Evaluate() const final;	

	string GetTrace() const final;

private:
	FreeZone zone;
};

}

#endif
