#ifndef RESULT_HPP
#define RESULT_HPP

#include "common/common.hpp"
#include "../link.hpp"

namespace SYM
{ 

// ------------------------- Result --------------------------

class Result
{
public:
    enum Category
    {
        UNDEFINED,
        DEFINED
    };    

    virtual bool IsDefinedAndUnique() const = 0;
};

// ------------------------- BooleanResult --------------------------

class BooleanResult : public Result
{
public:
    BooleanResult( Category cat, bool value = false );

    bool IsDefinedAndUnique() const override;    
    bool IsDefinedAndTrue() const;    
    bool IsDefinedAndFalse() const;    
    bool GetAsBool() const;    

    // Present a "certainty" ordering to simplify eval with dominance
    // effect in And and Or operators. True is bigger than undefined, 
    // which is bigger than false.
    bool operator<( const BooleanResult &other ) const;
    static bool CertaintyCompare( const shared_ptr<BooleanResult> &a, 
                                  const shared_ptr<BooleanResult> &b );

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

// ------------------------- SymbolResult --------------------------

class SymbolResult : public Result
{
public:
    SymbolResult( Category cat, SR::XLink xlink=SR::XLink() );
    
    bool IsDefinedAndUnique() const override;    
    SR::XLink GetAsXLink() const;    

private:    
    Category cat; 
    SR::XLink xlink;
};

};

#endif // include guard
