#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "link.hpp"

namespace SR
{ 

enum KeyingPlace
{
    PLACE_0,
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
    CouplingKey( const XLink &o ); // implicit allowed
    ~CouplingKey();
    XLink &operator =( const XLink &o );
    operator XLink() const;
    operator bool() const;
    TreePtr<Node> GetChildX() const;
    string GetTrace() const; // used for debug
    
private:    
    XLink xlink;
};


struct CouplingKeyMapBlock
{
    // This is the real key
    CouplingKey key;
    // These are just for investigations and checks 
    KeyingPlace place;
    PatternLink plink;
    const class AndRuleEngine *are;
    const class SCREngine *scre;
};

typedef map< Agent *, CouplingKeyMapBlock > CouplingKeysMap;

}

#endif