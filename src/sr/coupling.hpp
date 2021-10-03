#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "link.hpp"

namespace SR
{ 

enum KeyingPlace
{
    PLACE_UNKNOWN,
    PLACE_1,
    PLACE_2,
    PLACE_3,
    PLACE_4,
    PLACE_5   
};


class CouplingKey : public Traceable
{
public:
    CouplingKey();
    CouplingKey( const CouplingKey &other );
    CouplingKey( const XLink &xlink_ );
    explicit CouplingKey( XLink xlink_,
                          KeyingPlace place_,
                          PatternLink plink_,
                          const class AndRuleEngine *are_,
                          const class SCREngine *scre_ );
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