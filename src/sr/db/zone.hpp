#ifndef ZONE_HPP
#define ZONE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "mutator.hpp"

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
	virtual size_t GetNumTerminii() const = 0;
    virtual TreePtr<Node> GetBaseNode() const = 0;
        
    bool IsSubtree() const;
    void MarkOriginForEmbedded(RequiresSubordinateSCREngine *embedded_agent);
    FreeZone CreateSimilarScaffoldZone() const;
};    
 
}

#endif
