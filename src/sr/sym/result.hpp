#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"

namespace SYM
{ 

// ------------------------- ResultInterface --------------------------

class ResultInterface
{
public:
    enum Category
    {
        UNDEFINED,
        DEFINED
    };    

    virtual bool IsDefinedAndUnique() const = 0;
    virtual bool operator==( const ResultInterface &other ) const = 0;    
};

// ------------------------- BooleanResultInterface --------------------------

class BooleanResultInterface : public ResultInterface
{
public:
    virtual bool IsDefinedAndTrue() const = 0;    
    virtual bool IsDefinedAndFalse() const = 0;    
    virtual bool GetAsBool() const = 0;    
    
    virtual bool operator<( const BooleanResultInterface &other ) const = 0;
};

// ------------------------- BooleanResult --------------------------

class BooleanResult : public BooleanResultInterface
{
public:
    BooleanResult( Category cat, bool value = false );

    bool IsDefinedAndUnique() const override;    
    bool IsDefinedAndTrue() const override;    
    bool IsDefinedAndFalse() const override;    
    bool GetAsBool() const override;    
    bool operator==( const ResultInterface &other ) const override;    
    
    // Present a "certainty" ordering to simplify eval with dominance
    // effect in And and Or operators. True is bigger than undefined, 
    // which is bigger than false.
    bool operator<( const BooleanResultInterface &other ) const override;

private:
    // Certainty combines category and boolean value into a single
    // value which posesses a certainty ordering property
    enum class Certainty
    {
        FALSE,
        UNDEFINED,
        TRUE
    };    

    Certainty certainty;
};

// ------------------------- SymbolResultInterface --------------------------

class SymbolResultInterface : public ResultInterface
{
public:
    virtual SR::XLink GetAsXLink() const = 0;   
    virtual set<SR::XLink> GetAsSetOfXLinks() const = 0;
     
};

// ------------------------- SingleSymbolResult --------------------------

class SingleSymbolResult : public SymbolResultInterface
{
public:
    SingleSymbolResult( Category cat, SR::XLink xlink=SR::XLink() );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetAsXLink() const override;    
    set<SR::XLink> GetAsSetOfXLinks() const override;    
    bool operator==( const ResultInterface &other ) const override;    

private:    
    SR::XLink xlink;
};

// ------------------------- MultiSymbolResult --------------------------

class MultiSymbolResult : public SymbolResultInterface
{
public:
    MultiSymbolResult( set<SR::XLink> xlinks = set<SR::XLink>() );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetAsXLink() const override;    
    set<SR::XLink> GetAsSetOfXLinks() const override;    
    bool operator==( const ResultInterface &other ) const override;    

private:    
    set<SR::XLink> xlinks;
};

};

#endif // include guard