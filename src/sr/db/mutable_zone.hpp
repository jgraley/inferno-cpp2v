#ifndef MUTABLE_ZONE_HPP
#define MUTABLE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "mutator.hpp"
#include "df_relation.hpp"
#include "duplicate.hpp"
#include "db_common.hpp"

#include <unordered_set>

namespace SR 
{
class XTreeDatabase;    
class FreeZone;
 
class MutableZone : public Traceable
{
public:
    explicit MutableZone( Mutator &&base_, 
                              vector<Mutator> &&terminii_,
                              DBCommon::TreeOrdinal ordinal_ );

    bool IsEmpty() const;

	TreeZone GetXTreeZone() const;

	static void Swap( MutableZone &tree_zone_l, vector<TreeZone *> fixups_l, 
	                  MutableZone &tree_zone_r, vector<TreeZone *> fixups_r );
       
    string GetTrace() const override;
    
private:
    Mutator base;
    vector<Mutator> terminii;
   	DBCommon::TreeOrdinal ordinal;
};
 
}

#endif
