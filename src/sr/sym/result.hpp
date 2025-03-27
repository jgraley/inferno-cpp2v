#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"
#include "../db/sc_relation.hpp"
#include "../db/cat_relation.hpp"
#include "../db/df_relation.hpp"

#if 1
#define unique_ptr unique_ptr
#define make_unique make_unique
#else
#define unique_ptr shared_ptr
#define make_unique make_shared
#endif

namespace SR
{
    class XTreeDatabase;
};

namespace SYM
{ 

// ------------------------- BooleanResult --------------------------

class BooleanResult : public Traceable
{
public:
    explicit BooleanResult( bool value = false );

    bool IsDefinedAndUnique() const;    
    bool IsDefinedAndTrue() const;    
    bool IsDefinedAndFalse() const;    
    bool GetAsBool() const;    
    bool operator==( const BooleanResult &other ) const;    
    
    // Present a "certainty" ordering to simplify eval with dominance
    // effect in And and Or operators. True is bigger than undefined, 
    // which is bigger than false.
    bool operator<( const BooleanResult &other ) const;

    virtual string Render() const;
    string GetTrace() const final;
    
private:
    bool value;
};

// ------------------------- SymbolicResult --------------------------

class SymbolicResult : public Traceable
{
public:
    virtual bool IsDefinedAndUnique() const = 0;
    virtual SR::XLink GetOnlyXLink() const = 0;   
    virtual bool TryExtensionalise( set<SR::XLink> &links ) const = 0;     
    virtual bool operator==( const SymbolicResult &other ) const = 0;    

    virtual string Render() const = 0;
    string GetTrace() const final;
};

// ------------------------- UniqueResult --------------------------

class UniqueResult : public SymbolicResult
{
public:
    explicit UniqueResult( SR::XLink xlink );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;    
    
    string Render() const override;

private:    
    SR::XLink xlink;
};

// ------------------------- EmptyResult --------------------------

class EmptyResult : public SymbolicResult
{
public:
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;    
    
    string Render() const override;
};

// ------------------------- SubsetResult --------------------------

class SubsetResult : public SymbolicResult
{
public:
    explicit SubsetResult( set<SR::XLink> xlinks = set<SR::XLink>(), bool complement_flag = false );
    
    // Use this to force other or unknown symbol results to extensionalise
    explicit SubsetResult( unique_ptr<SymbolicResult> other );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;

    unique_ptr<SubsetResult> GetComplement() const;
    static unique_ptr<SubsetResult> GetUnion( list<unique_ptr<SubsetResult>> ops );
    static unique_ptr<SubsetResult> GetIntersection( list<unique_ptr<SubsetResult>> ops );

    string Render() const override;

private:    
    static unique_ptr<SubsetResult> DeMorgan( function<unique_ptr<SubsetResult>( list<unique_ptr<SubsetResult>> )> lambda,
                                                 list<unique_ptr<SubsetResult>> ops );
    static unique_ptr<SubsetResult> UnionCore( list<unique_ptr<SubsetResult>> ops );
    static unique_ptr<SubsetResult> IntersectionCore( list<unique_ptr<SubsetResult>> ops );

    set<SR::XLink> xlinks;
    bool complement_flag;
};

// ------------------------- DepthFirstRangeResult --------------------------

class DepthFirstRangeResult : public SymbolicResult
{
public:
    // lower or upper can be null to exclude that limit
    typedef SR::DepthFirstRelation::KeyType KeyType;
    DepthFirstRangeResult( const SR::XTreeDatabase *x_tree_db, KeyType lower, bool lower_incl, KeyType upper, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;

    //unique_ptr<SubsetResult> GetComplement() const;
    //static unique_ptr<SubsetResult> GetUnion( list<unique_ptr<SubsetResult>> ops );
    //static unique_ptr<SubsetResult> GetIntersection( list<unique_ptr<SubsetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    const SR::XLink lower, upper;
    const bool lower_incl, upper_incl;
};

// ------------------------- SimpleCompareRangeResult --------------------------

class SimpleCompareRangeResult : public SymbolicResult
{
public:
    // lower or upper can be null to exclude that limit
    typedef SR::SimpleCompareRelation::KeyType KeyType;
    SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db, KeyType lower, bool lower_incl, KeyType upper, bool upper_incl );  
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;

    //unique_ptr<SubsetResult> GetComplement() const;
    //static unique_ptr<SubsetResult> GetUnion( list<unique_ptr<SubsetResult>> ops );
    //static unique_ptr<SubsetResult> GetIntersection( list<unique_ptr<SubsetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    const KeyType lower, upper;
    const bool lower_incl, upper_incl;
};


// ------------------------- CategoryRangeResult --------------------------

class CategoryRangeResult : public SymbolicResult
{
public:
    // Shared_ptr is OK since we don't mutate at eval time, just dereference
    typedef SR::CategoryRelation::KeyType KeyType;
    typedef pair<shared_ptr<KeyType>, shared_ptr<KeyType>> CatBounds;   
    typedef list<CatBounds> CatBoundsList;

    // lower or upper can be null to exclude that limit
    CategoryRangeResult( const SR::XTreeDatabase *x_tree_db, CatBoundsList &&bounds_list, bool lower_incl, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryExtensionalise( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolicResult &other ) const override;

    //unique_ptr<SubsetResult> GetComplement() const;
    //static unique_ptr<SubsetResult> GetUnion( list<unique_ptr<SubsetResult>> ops );
    //static unique_ptr<SubsetResult> GetIntersection( list<unique_ptr<SubsetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    const CatBoundsList bounds_list;
    const bool lower_incl, upper_incl;
};


};

#endif // include guard
