#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"

namespace SYM
{ 

// ------------------------- BooleanResultInterface --------------------------

class BooleanResultInterface : public Traceable
{
public:
    virtual bool IsDefinedAndTrue() const = 0;    
    virtual bool IsDefinedAndFalse() const = 0;    
    virtual bool GetAsBool() const = 0;    
    
    virtual bool IsDefinedAndUnique() const = 0;
    virtual bool operator==( const BooleanResultInterface &other ) const = 0;    
    virtual bool operator<( const BooleanResultInterface &other ) const = 0;
};

// ------------------------- BooleanResult --------------------------

class BooleanResult : public BooleanResultInterface
{
public:
    explicit BooleanResult( bool value = false );

    bool IsDefinedAndUnique() const;    
    bool IsDefinedAndTrue() const override;    
    bool IsDefinedAndFalse() const override;    
    bool GetAsBool() const override;    
    bool operator==( const BooleanResultInterface &other ) const;    
    
    // Present a "certainty" ordering to simplify eval with dominance
    // effect in And and Or operators. True is bigger than undefined, 
    // which is bigger than false.
    bool operator<( const BooleanResultInterface &other ) const override;

    string GetTrace() const override;
    
private:
    bool value;
};

// ------------------------- SymbolResultInterface --------------------------

class SymbolResultInterface : public Traceable
{
public:
    enum Category
    {
        UNDEFINED,
        DEFINED
    };    

    virtual SR::XLink GetAsXLink() const = 0;   
    virtual bool TryGetAsSetOfXLinks( set<SR::XLink> &links ) const = 0;
     
    virtual bool IsDefinedAndUnique() const = 0;
    virtual bool operator==( const SymbolResultInterface &other ) const = 0;    
};

// ------------------------- SymbolResult --------------------------

class SymbolResult : public SymbolResultInterface
{
public:
    explicit SymbolResult( Category cat, SR::XLink xlink=SR::XLink() );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetAsXLink() const override;    
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
    static shared_ptr<SymbolSetResult> Create( shared_ptr<SymbolResultInterface> other );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetAsXLink() const override;    
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

};

#endif // include guard
