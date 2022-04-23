#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"

namespace SR
{
    class TheKnowledge;
}

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

    string GetTrace() const override;
    
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
    
    string GetTrace() const override;

private:    
    SR::XLink xlink;
};

// ------------------------- SymbolSetResult --------------------------

class SymbolSetResult : public SymbolResultInterface
{
public:
    explicit SymbolSetResult( set<SR::XLink> xlinks = set<SR::XLink>(), bool complement_flag = false );
    
    // Use this to force other or unknown symbol results to extensionalise
    explicit SymbolSetResult( shared_ptr<SymbolResultInterface> other );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    shared_ptr<SymbolSetResult> GetComplement() const;
    static shared_ptr<SymbolSetResult> GetUnion( list<shared_ptr<SymbolSetResult>> ops );
    static shared_ptr<SymbolSetResult> GetIntersection( list<shared_ptr<SymbolSetResult>> ops );

    string GetTrace() const override;

private:    
    static shared_ptr<SymbolSetResult> DeMorgan( function<shared_ptr<SymbolSetResult>( list<shared_ptr<SymbolSetResult>> )> lambda,
                                                 list<shared_ptr<SymbolSetResult>> ops );
    static shared_ptr<SymbolSetResult> UnionCore( list<shared_ptr<SymbolSetResult>> ops );
    static shared_ptr<SymbolSetResult> IntersectionCore( list<shared_ptr<SymbolSetResult>> ops );

    set<SR::XLink> xlinks;
    bool complement_flag;
};

// ------------------------- SymbolRangeResult --------------------------

class SymbolRangeResult : public SymbolResultInterface
{
public:
    // lower or upper can be null to exclude that limit
    SymbolRangeResult( const SR::TheKnowledge *knowledge, SR::XLink lower, bool lower_incl, SR::XLink upper, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //shared_ptr<SymbolSetResult> GetComplement() const;
    //static shared_ptr<SymbolSetResult> GetUnion( list<shared_ptr<SymbolSetResult>> ops );
    //static shared_ptr<SymbolSetResult> GetIntersection( list<shared_ptr<SymbolSetResult>> ops );

    string GetTrace() const override;

private:    
    const SR::TheKnowledge *knowledge;
    SR::XLink lower, upper;
    bool lower_incl, upper_incl;
};


};

#endif // include guard
