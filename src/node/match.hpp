#ifndef MATCH_HPP
#define MATCH_HPP

#include "common/trace.hpp"
#include "common/mismatch.hpp"
#include "common/common.hpp"

#include <typeinfo>
#include <string>
#include <typeindex>		


class Orderable
{
public:
    enum OrderProperty
    {
        // Objects with different values must compare non-equal
        STRICT,     
        
        // Objects with different values may compare equal if this
        // avoids non-repeatable behaviour eg memory allocation dependencies 
        REPEATABLE
    };
    
    static CompareResult Compare( const Orderable *l, 
                                  const Orderable *r, 
                                  OrderProperty order_property = STRICT );
    virtual CompareResult CovariantCompare( const Orderable *candidate, 
                                            OrderProperty order_property ) const;
};

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

	virtual bool IsSubclass( const Matcher *source_archetype ) const = 0;
	virtual bool IsLocalMatch( const Matcher *candidate ) const;
    virtual ~Matcher();
    template< class TARGET_TYPE >
    static inline bool IsSubclassStatic( const TARGET_TYPE *target_archetype, const Matcher *source_archetype )
    {
        ASSERT( source_archetype );
        (void)target_archetype; // don't care about value of archetypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(source_archetype);
    }
};

#define MATCHER_FUNCTION \
    virtual bool IsSubclass( const Matcher *source_archetype ) const \
    { \
        return IsSubclassStatic( this, source_archetype ); \
    }

#endif

