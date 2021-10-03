#include "coupling.hpp"
#include "and_rule_engine.hpp"
#include "scr_engine.hpp"

using namespace SR;

#ifdef COUPLINGKEY_DEFAULT_CONSTRUCTOR
CouplingKey::CouplingKey() :
    xlink(XLink()),
    place(PLACE_UNKNOWN),
    plink(PatternLink()),
    are(nullptr),
    scre(nullptr)
{
}
#endif


#ifdef COUPLINGKEY_XLINK_CONSTRUCTOR
CouplingKey::CouplingKey( XLink xlink_ ) :
    xlink(xlink_),
    place(PLACE_UNKNOWN),
    plink(PatternLink()),
    are(nullptr),
    scre(nullptr)
{
}
#endif


CouplingKey::CouplingKey( XLink xlink_,
                          KeyingPlace place_,
                          PatternLink plink_,
                          const class AndRuleEngine *are_,
                          const class SCREngine *scre_ ) :
    xlink(xlink_),
    place(place_),
    plink(plink_),
    are(are_),
    scre(scre_)
{
}


CouplingKey::CouplingKey( const CouplingKey &other ) :
    xlink(other.xlink),
    place(other.place),
    plink(other.plink),
    are(other.are),
    scre(other.scre)
{
};


CouplingKey::~CouplingKey()
{
}


XLink &CouplingKey::operator =( const XLink &xlink_ )
{
    xlink = xlink_;
    return xlink;
}


CouplingKey::operator XLink() const
{
    return xlink;
}


CouplingKey::operator bool() const
{
    return (bool)xlink;
}


TreePtr<Node> CouplingKey::GetKeyX() const
{
    return xlink.GetChildX();
}


string CouplingKey::GetTrace() const
{
    string s = plink.GetTrace();
    s += " := ";
    s += xlink.GetTrace();
    if( place == PLACE_UNKNOWN )
        s += "PLACE_UNKNOWN";
    else
        s += SSPrintf(" PLACE_%d ", place);
    if( are )
        s += are->GetTrace();
    else if( scre )
        s += scre->GetTrace();
    return s;
}

