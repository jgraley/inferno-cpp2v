#ifndef MATCH_HPP
#define MATCH_HPP

#include "common/trace.hpp"
#include "common/mismatch.hpp"
#include "common/common.hpp"

#include <typeinfo>
#include <string>
#include <typeindex>		

/// Support class allowing hierarchical type comparisons between nodes
struct Matcher
{        
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    class LocalMismatch : public Mismatch
    {
    };

	virtual bool IsSubcategory( const Matcher &source_archetype ) const = 0;
	virtual bool IsLocalMatch( const Matcher &candidate ) const;
	virtual bool IsLocalMatchCovariant( const Matcher &candidate ) const;
    virtual ~Matcher();
    template< class TARGET_TYPE >
    static inline bool IsSubcategoryStatic( const TARGET_TYPE &target_archetype, const Matcher &source_archetype )
    {
        (void)target_archetype; // don't care about value of archetypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(&source_archetype);
    }
    
    static bool IsEquivalentCategory( const Matcher &l, const Matcher &r );
};

#define MATCHER_FUNCTION \
    virtual bool IsSubcategory( const Matcher &source_archetype ) const \
    { \
        return IsSubcategoryStatic( *this, source_archetype ); \
    }

#endif

