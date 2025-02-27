#ifndef ZONE_EXPRESSIONS_HPP
#define ZONE_EXPRESSIONS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../scr_engine.hpp"
#include <functional>

namespace SR 
{
class XTreeDatabase;

struct UpEvalExecKit
{
	XTreeDatabase *x_tree_db;
};


// ------------------------- ZoneExpression --------------------------

class ZoneExpression : public Traceable
{
public:	
	virtual unique_ptr<FreeZone> Evaluate(const UpEvalExecKit &kit) const = 0;
	virtual void ForChildren(function<void(shared_ptr<ZoneExpression> &expr)> func) = 0;
			                        
	static void ForDepthFirstWalk( shared_ptr<ZoneExpression> &base,
								   function<void(shared_ptr<ZoneExpression> &expr)> func_in,
								   function<void(shared_ptr<ZoneExpression> &expr)> func_out );

	virtual void DepthFirstWalkImpl(function<void(shared_ptr<ZoneExpression> &expr)> func_in,
			                        function<void(shared_ptr<ZoneExpression> &expr)> func_out) = 0;
};

// ------------------------- LayoutOperator --------------------------

class LayoutOperator : public ZoneExpression
{
public:
	typedef list<shared_ptr<ZoneExpression>>::iterator ChildExpressionIterator;

protected:
    LayoutOperator( list<shared_ptr<ZoneExpression>> &&child_expressions_ );
    LayoutOperator();
    
public:
    int GetNumChildExpressions() const;
    ChildExpressionIterator GetChildrenBegin();
    ChildExpressionIterator GetChildrenEnd();
	list<shared_ptr<ZoneExpression>> &GetChildExpressions();
	const list<shared_ptr<ZoneExpression>> &GetChildExpressions() const;
	list<shared_ptr<ZoneExpression>> &&MoveChildExpressions();
	
    string GetChildExpressionsTrace() const;

	void ForChildren(function<void(shared_ptr<ZoneExpression> &expr)> func) override;

	void DepthFirstWalkImpl(function<void(shared_ptr<ZoneExpression> &expr)> func_in,
			                function<void(shared_ptr<ZoneExpression> &expr)> func_out) override;

private:
	list<shared_ptr<ZoneExpression>> child_expressions;

};

// ------------------------- MergeZoneOperator --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class MergeZoneOperator : public LayoutOperator
{
protected:
    MergeZoneOperator( list<shared_ptr<ZoneExpression>> &&child_expressions_ );
    MergeZoneOperator();

public:
    void AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker );
    virtual void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) = 0;
    virtual list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const = 0;
    virtual void ClearEmbeddedMarkers() = 0;
    
    virtual Zone &GetZone() = 0;
    virtual const Zone &GetZone() const = 0;
    
 	void EvaluateChildrenAndPopulate( const UpEvalExecKit &kit, FreeZone &free_zone ) const;		
};


// ------------------------- DupMergeTreeZoneOperator --------------------------

// Construct with tree zone and child expressions for terminii. Markers can 
// then be added. On evaluate: duplicate into a free zone, apply markers,
// populate it immediately (rule #726), and return the resulting FreeZone. 
// Due to rule #726, we cannot provide a merge method (or we could add support 
// for markers in interior possibly not at base).
class DupMergeTreeZoneOperator : public MergeZoneOperator
{
public:
    DupMergeTreeZoneOperator( TreeZone zone_, list<shared_ptr<ZoneExpression>> &&child_expressions );
    DupMergeTreeZoneOperator( TreeZone zone_ );
    
    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;
    void ClearEmbeddedMarkers() final;

    TreeZone &GetZone() override;
    const TreeZone &GetZone() const override;
	
	unique_ptr<FreeZone> Evaluate(const UpEvalExecKit &kit) const final;	
    
    shared_ptr<ZoneExpression> DuplicateToFree() const;
    
	string GetTrace() const final;
    
private:
	TreeZone zone;
	list<RequiresSubordinateSCREngine *> embedded_markers;
};

// ------------------------- MergeFreeZoneOperator --------------------------

// Construct with free zone and child expressions for terminii. Markers can 
// then be added. On evaluate: populate the zone, and return the resulting 
// FreeZone. Rule #726 means there can never be duplicate, clone, move etc, 
// because we mark for embedded immediately (but this means we can merge 
// without needing to represent markers in interior possibly not at base).
class MergeFreeZoneOperator : public MergeZoneOperator
{
public:
    MergeFreeZoneOperator( FreeZone zone_, list<shared_ptr<ZoneExpression>> &&child_expressions );
    MergeFreeZoneOperator( FreeZone zone_ );

    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;
    void ClearEmbeddedMarkers() final;

	ChildExpressionIterator SpliceOver( ChildExpressionIterator it_child, 
                                        list<shared_ptr<ZoneExpression>> &&child_expressions );

    FreeZone &GetZone() final;
    const FreeZone &GetZone() const final;
    
   	unique_ptr<FreeZone> Evaluate(const UpEvalExecKit &kit) const final;	

	string GetTrace() const final;

private:
	FreeZone zone;
};

// ------------------------- ReplaceOperator --------------------------

class ReplaceOperator : public LayoutOperator
{
public:	
	ReplaceOperator( TreeZone target_tree_zone_, 
	                 shared_ptr<Zone> source_zone_,
	                 list<shared_ptr<ZoneExpression>> &&child_expressions );
	unique_ptr<FreeZone> Evaluate(const UpEvalExecKit &kit) const override;
	void Execute(const UpEvalExecKit &kit) const;

private:
	TreeZone target_tree_zone;
	shared_ptr<Zone> source_zone;
};

}

#endif
