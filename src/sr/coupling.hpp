#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "link.hpp"

namespace SR
{ 

enum KeyProducer
{
    KEY_PRODUCER_U,
    KEY_PRODUCER_1,
    KEY_PRODUCER_2,
    KEY_PRODUCER_3,
    KEY_PRODUCER_4,
    KEY_PRODUCER_5,
    KEY_PRODUCER_6,
    KEY_PRODUCER_7
};


enum KeyConsumer
{
    KEY_CONSUMER_U,
    KEY_CONSUMER_1,
    KEY_CONSUMER_2,
    KEY_CONSUMER_3,
    KEY_CONSUMER_4,
    KEY_CONSUMER_5,
    KEY_CONSUMER_6,
    KEY_CONSUMER_7
};


class CouplingKey : public Traceable
{
public:
    CouplingKey();   
    explicit CouplingKey( XLink xlink_,
                          KeyProducer producer_,
                          PatternLink plink_ = PatternLink(),
                          const class AndRuleEngine *are_ = nullptr,
                          const class SCREngine *scre_ = nullptr );
    CouplingKey( const CouplingKey &other );
    ~CouplingKey();
    XLink &operator =( const XLink &xlink_ );
    operator bool() const;
    XLink GetKeyXLink( KeyConsumer consumer = KEY_CONSUMER_U ) const;
    TreePtr<Node> GetKeyXNode( KeyConsumer consumer = KEY_CONSUMER_U ) const;
    string GetTrace() const; // used for debug
    bool IsFinal() const;
    
private:    
    void Dump( KeyConsumer consumer ) const;

    // This is the real key
    XLink xlink;

    // These are just for investigations and checks 
    KeyProducer producer;
    string producer_pre;
    PatternLink plink;
    const class AndRuleEngine *are;
    const class SCREngine *scre;
};


typedef map< Agent *, CouplingKey > CouplingKeysMap;

}

#endif