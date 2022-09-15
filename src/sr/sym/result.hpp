#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"

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

// ------------------------- SymbolResultInterface --------------------------

class SymbolResultInterface : public Traceable
{
public:
    virtual bool IsDefinedAndUnique() const = 0;
    virtual SR::XLink GetOnlyXLink() const = 0;   
    virtual bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const = 0;     
    virtual bool operator==( const SymbolResultInterface &other ) const = 0;    

    virtual string Render() const = 0;
    string GetTrace() const final;
};

// ------------------------- SymbolResult --------------------------

class SymbolResult : public SymbolResultInterface
{
public:
    enum Category
    {
        NOT_A_SYMBOL
    };    

    explicit SymbolResult( SR::XLink xlink );
    explicit SymbolResult( Category cat );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;    
    
    string Render() const override;

private:    
    SR::XLink xlink;
};

// ------------------------- SetResult --------------------------

class SetResult : public SymbolResultInterface
{
public:
    explicit SetResult( set<SR::XLink> xlinks = set<SR::XLink>(), bool complement_flag = false );
    
    // Use this to force other or unknown symbol results to extensionalise
    explicit SetResult( unique_ptr<SymbolResultInterface> other );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    unique_ptr<SetResult> GetComplement() const;
    static unique_ptr<SetResult> GetUnion( list<unique_ptr<SetResult>> ops );
    static unique_ptr<SetResult> GetIntersection( list<unique_ptr<SetResult>> ops );

    string Render() const override;

private:    
    static unique_ptr<SetResult> DeMorgan( function<unique_ptr<SetResult>( list<unique_ptr<SetResult>> )> lambda,
                                                 list<unique_ptr<SetResult>> ops );
    static unique_ptr<SetResult> UnionCore( list<unique_ptr<SetResult>> ops );
    static unique_ptr<SetResult> IntersectionCore( list<unique_ptr<SetResult>> ops );

    set<SR::XLink> xlinks;
    bool complement_flag;
};

// ------------------------- DepthFirstRangeResult --------------------------

class DepthFirstRangeResult : public SymbolResultInterface
{
public:
    // lower or upper can be null to exclude that limit
    DepthFirstRangeResult( const SR::XTreeDatabase *x_tree_db, SR::XLink lower, bool lower_incl, SR::XLink upper, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //unique_ptr<SetResult> GetComplement() const;
    //static unique_ptr<SetResult> GetUnion( list<unique_ptr<SetResult>> ops );
    //static unique_ptr<SetResult> GetIntersection( list<unique_ptr<SetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    SR::XLink lower, upper;
    bool lower_incl, upper_incl;
};

// ------------------------- SimpleCompareRangeResult --------------------------

class SimpleCompareRangeResult : public SymbolResultInterface
{
public:
    // lower or upper can be null to exclude that limit
    SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db, SR::XLink lower, bool lower_incl, SR::XLink upper, bool upper_incl );
    SimpleCompareRangeResult( const SR::XTreeDatabase *x_tree_db, TreePtr<Node> lower, bool lower_incl, TreePtr<Node> upper, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //unique_ptr<SetResult> GetComplement() const;
    //static unique_ptr<SetResult> GetUnion( list<unique_ptr<SetResult>> ops );
    //static unique_ptr<SetResult> GetIntersection( list<unique_ptr<SetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    SR::XLink lower;
    bool lower_incl;
    SR::XLink upper;
    bool upper_incl;
};


// ------------------------- CategoryRangeResult --------------------------

class CategoryRangeResult : public SymbolResultInterface
{
public:
    // Shared_ptr is OK since we don't mutate at eval time, just dereference
    typedef pair<shared_ptr<SR::XLink>, shared_ptr<SR::XLink>> XLinkBounds;
    typedef list<XLinkBounds> XLinkBoundsList;

    // lower or upper can be null to exclude that limit
    CategoryRangeResult( const SR::XTreeDatabase *x_tree_db, XLinkBoundsList &&bounds_list, bool lower_incl, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //unique_ptr<SetResult> GetComplement() const;
    //static unique_ptr<SetResult> GetUnion( list<unique_ptr<SetResult>> ops );
    //static unique_ptr<SetResult> GetIntersection( list<unique_ptr<SetResult>> ops );

    string Render() const override;

private:    
    const SR::XTreeDatabase *x_tree_db;
    const XLinkBoundsList bounds_list;
    const bool lower_incl, upper_incl;
};


};

#endif // include guard
