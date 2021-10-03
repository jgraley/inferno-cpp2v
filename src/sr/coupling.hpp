#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "link.hpp"

#define COUPLINGKEY_DEFAULT_CONSTRUCTOR
#define COUPLINGKEY_XLINK_CONSTRUCTOR

namespace SR
{ 

enum KeyingPlace
{
    PLACE_UNKNOWN,
    PLACE_1,
    PLACE_2,
    PLACE_3,
    PLACE_4,
    PLACE_5,
    PLACE_6,
    PLACE_7,
    PLACE_8,
    PLACE_9       
};


class CouplingKey : public Traceable
{
public:
#ifdef COUPLINGKEY_DEFAULT_CONSTRUCTOR
    CouplingKey();
#endif    
#ifdef COUPLINGKEY_XLINK_CONSTRUCTOR
    explicit CouplingKey( XLink xlink_ );
#endif    
    explicit CouplingKey( XLink xlink_,
                          KeyingPlace place_,
                          PatternLink plink_ = PatternLink(),
                          const class AndRuleEngine *are_ = nullptr,
                          const class SCREngine *scre_ = nullptr );
    CouplingKey( const CouplingKey &other );
    ~CouplingKey();
    XLink &operator =( const XLink &xlink_ );
    operator XLink() const;
    operator bool() const;
    TreePtr<Node> GetKeyX() const;
    string GetTrace() const; // used for debug
    
private:    
    // This is the real key
    XLink xlink;

    // These are just for investigations and checks 
    KeyingPlace place;
    PatternLink plink;
    const class AndRuleEngine *are;
    const class SCREngine *scre;
};


typedef map< Agent *, CouplingKey > CouplingKeysMap;

}

#endif