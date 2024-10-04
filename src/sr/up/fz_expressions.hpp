#ifndef FZ_EXPRESSIONS_HPP
#define FZ_EXPRESSIONS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include <functional>

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
public:
	typedef list<shared_ptr<FreeZoneExpression>>::iterator ChildExpressionIterator;
	
protected:
    PopulateZoneOperator( list<shared_ptr<FreeZoneExpression>> &&child_expressions_ );
    PopulateZoneOperator();

public:
    void AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker );
    virtual void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) = 0;
    virtual list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const = 0;
    
    virtual Zone &GetZone() = 0;
    virtual const Zone &GetZone() const = 0;
    
    int GetNumChildExpressions() const;
    ChildExpressionIterator GetChildrenBegin();
    ChildExpressionIterator GetChildrenEnd();
	list<shared_ptr<FreeZoneExpression>> &GetChildExpressions();
	const list<shared_ptr<FreeZoneExpression>> &GetChildExpressions() const;
	list<shared_ptr<FreeZoneExpression>> &&MoveChildExpressions();
	
    string GetChildExpressionsTrace() const;

	void ForChildren(function<void(shared_ptr<FreeZoneExpression> &expr)> func) override;

	void EvaluateWithFreeZone( FreeZone &free_zone ) const;	
	
	void DepthFirstWalkImpl(function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
			                function<void(shared_ptr<FreeZoneExpression> &expr)> func_out) override;

private:
	list<shared_ptr<FreeZoneExpression>> child_expressions;
};


// ------------------------- PopulateTreeZoneOperator --------------------------

// Construct with tree zone and child expressions for terminii. Markers can 
// then be added. On evaluate: duplicate into a free zone, apply markers,
// populate it immediately (rule #726), and return the resulting FreeZone. 
// Due to rule #726, we cannot provide a merge method (or we could add support 
// for markers in interior possibly not at base).
class PopulateTreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateTreeZoneOperator( TreeZone zone_, list<shared_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateTreeZoneOperator( TreeZone zone_ );
    
    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;

    TreeZone &GetZone() override;
    const TreeZone &GetZone() const override;
	
	unique_ptr<FreeZone> Evaluate() const final;	
    
    shared_ptr<FreeZoneExpression> DuplicateToFree() const;
    
	string GetTrace() const final;
    
private:
	TreeZone zone;
	list<RequiresSubordinateSCREngine *> embedded_markers;
};

// ------------------------- PopulateFreeZoneOperator --------------------------

// Construct with free zone and child expressions for terminii. Markers can 
// then be added. On evaluate: populate the zone, and return the resulting 
// FreeZone. Rule #726 means there can never be duplicate, clone, move etc, 
// because we mark for embedded immediately (but this means we can merge 
// without needing to represent markers in interior possibly not at base).
class PopulateFreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateFreeZoneOperator( FreeZone zone_, list<shared_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateFreeZoneOperator( FreeZone zone_ );

    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;

	ChildExpressionIterator SpliceOver( ChildExpressionIterator it_child, 
                                        list<shared_ptr<FreeZoneExpression>> &&child_expressions );

    FreeZone &GetZone() final;
    const FreeZone &GetZone() const final;
    
   	unique_ptr<FreeZone> Evaluate() const final;	

	string GetTrace() const final;

private:
	FreeZone zone;
};

}

#endif
