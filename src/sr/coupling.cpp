#include "coupling.hpp"
#include "and_rule_engine.hpp"
#include "scr_engine.hpp"

using namespace SR;

CouplingKey::CouplingKey() :
    xlink(XLink()),
    producer(KEY_PRODUCER_U),
    plink(PatternLink()),
    are(nullptr),
    scre(nullptr)
{
}


CouplingKey::CouplingKey( XLink xlink_,
                          KeyProducer producer_,
                          PatternLink plink_,
                          const class AndRuleEngine *are_,
                          const class SCREngine *scre_ ) :
    xlink(xlink_),
    producer(producer_),
    producer_pre( Tracer::GetPrefix() ),
    plink(plink_),
    are(are_),
    scre(scre_)
{
}


CouplingKey::CouplingKey( LocatedLink link_,
                          KeyProducer producer_,
                          const class AndRuleEngine *are_,
                          const class SCREngine *scre_ ) :
    xlink(link_),
    producer(producer_),
    producer_pre( Tracer::GetPrefix() ),
    plink(link_),
    are(are_),
    scre(scre_)
{
}


CouplingKey::CouplingKey( const CouplingKey &other ) :
    xlink(other.xlink),
    producer(other.producer),
    producer_pre( Tracer::GetPrefix() ),
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


CouplingKey::operator bool() const
{
    if( xlink )
        ASSERT( xlink.GetChildX() );
    return (bool)xlink;
}


XLink CouplingKey::GetKeyXLink( KeyConsumer consumer ) const
{
    Dump( consumer );
    return xlink;
}


TreePtr<Node> CouplingKey::GetKeyXNode( KeyConsumer consumer ) const
{
    Dump( consumer );
    return xlink.GetChildX();
}


string CouplingKey::GetTrace() const
{
    string s;
    if( producer == KEY_PRODUCER_U )
        s += "KEY_PRODUCER_U ";                                               
    else
        s += SSPrintf("KEY_PRODUCER_%d ", producer);
    s += producer_pre + " ";
    /*s += plink.GetTrace();
    s += " := ";
    s += xlink.GetTrace();
    s += " ";                                                    
    if( are )
        s += are->GetTrace();
    else if( scre )
        s += scre->GetTrace();*/
    return s;
}


bool CouplingKey::IsFinal() const
{
    return xlink ? xlink.GetChildX()->IsFinal() : false;
}


void CouplingKey::Dump( KeyConsumer consumer ) const
{
    string s;
    if( producer == KEY_PRODUCER_U )
        s += "PRODUCER_U";                                               
    else
        s += SSPrintf("PRODUCER_%d", producer);
    s += "->";
    if( consumer == KEY_CONSUMER_U )
        s += "CONSUMER_U";                                               
    else
        s += SSPrintf("CONSUMER_%d", consumer);
        
    string pp = producer_pre;
    string cp = Tracer::GetPrefix() + " ";
    RemoveCommonPrefix( pp, cp );
    s += " " + pp + "->"+ cp;
    
    printf("%s\n", s.c_str());
}
