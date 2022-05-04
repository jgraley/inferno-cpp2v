#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"
#include "../equivalence.hpp"
#include "../the_knowledge.hpp"

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

// ------------------------- SetResult --------------------------

class SetResult : public SymbolResultInterface
{
public:
    explicit SetResult( set<SR::XLink> xlinks = set<SR::XLink>(), bool complement_flag = false );
    
    // Use this to force other or unknown symbol results to extensionalise
    explicit SetResult( shared_ptr<SymbolResultInterface> other );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    shared_ptr<SetResult> GetComplement() const;
    static shared_ptr<SetResult> GetUnion( list<shared_ptr<SetResult>> ops );
    static shared_ptr<SetResult> GetIntersection( list<shared_ptr<SetResult>> ops );

    string GetTrace() const override;

private:    
    static shared_ptr<SetResult> DeMorgan( function<shared_ptr<SetResult>( list<shared_ptr<SetResult>> )> lambda,
                                                 list<shared_ptr<SetResult>> ops );
    static shared_ptr<SetResult> UnionCore( list<shared_ptr<SetResult>> ops );
    static shared_ptr<SetResult> IntersectionCore( list<shared_ptr<SetResult>> ops );

    set<SR::XLink> xlinks;
    bool complement_flag;
};

// ------------------------- RangeResult --------------------------

class RangeResult : public SymbolResultInterface
{
public:
    // lower or upper can be null to exclude that limit
    RangeResult( const SR::TheKnowledge *knowledge, SR::XLink lower, bool lower_incl, SR::XLink upper, bool upper_incl );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //shared_ptr<SetResult> GetComplement() const;
    //static shared_ptr<SetResult> GetUnion( list<shared_ptr<SetResult>> ops );
    //static shared_ptr<SetResult> GetIntersection( list<shared_ptr<SetResult>> ops );

    string GetTrace() const override;

private:    
    const SR::TheKnowledge *knowledge;
    SR::XLink lower, upper;
    bool lower_incl, upper_incl;
};

// ------------------------- EquivalenceClassResult --------------------------

class EquivalenceClassResult : public SymbolResultInterface
{
public:
    // lower or upper can be null to exclude that limit
    EquivalenceClassResult( const SR::TheKnowledge *knowledge, SR::XLink class_example );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetOnlyXLink() const override;    
    bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const override;
    bool operator==( const SymbolResultInterface &other ) const override;

    //shared_ptr<SetResult> GetComplement() const;
    //static shared_ptr<SetResult> GetUnion( list<shared_ptr<SetResult>> ops );
    //static shared_ptr<SetResult> GetIntersection( list<shared_ptr<SetResult>> ops );

    string GetTrace() const override;

private:    
    const SR::TheKnowledge *knowledge;
    SR::XLink class_example;
    SR::EquivalenceRelation equivalence_relation;
};


};

#endif // include guard
