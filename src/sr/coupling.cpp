#include "coupling.hpp"

using namespace SR;

CouplingKey::CouplingKey()
{
}


CouplingKey::CouplingKey( const XLink &o )
{
    xlink = o;
}


CouplingKey::~CouplingKey()
{
}


XLink &CouplingKey::operator =( const XLink &o )
{
    xlink = o;
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
    return xlink.GetTrace();
}

