#ifndef ZONE_HPP
#define ZONE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "terminus.hpp"

#include <unordered_set>

namespace SR 
{
	class SCREngine;
	class RequiresSubordinateSCREngine;
	class FreeZone;
	class XTreeDatabase;
	    
// ------------------------- Zone --------------------------
class Zone : public Traceable
{
public:
	virtual ~Zone() = default;

    virtual bool IsEmpty() const = 0;
	virtual int GetNumTerminii() const = 0;
    virtual TreePtr<Node> GetBaseNode() const = 0;
        
    void MarkBaseForEmbedded(const SCREngine *scr_engine, RequiresSubordinateSCREngine *embedded_agent);
};    
 
}

#endif
