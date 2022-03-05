#ifndef TELEPORT_AGENT_HPP
#define TELEPORT_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/overloads.hpp"
#include "../cache.hpp"

namespace SR
{ 

class TeleportAgent : public PreRestrictedAgent
{
public:    
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                  
    virtual LocatedLink RunTeleportQuery( XLink keyer_xlink ) const { ASSERTFAIL(); }
    
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks );

    virtual void Reset();    

private:
    mutable CacheByLocation cache;    
};

};

#endif
