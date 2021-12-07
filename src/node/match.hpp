#ifndef MATCH_HPP
#define MATCH_HPP


#include "common/trace.hpp"
#include "common/mismatch.hpp"
#include <typeinfo>
#include <string>
#include <typeindex>		

/// Support class allowing hierarchical type comparisons between nodes
struct Matcher
{
    enum Ordering
    {
        UNIQUE,     // a strong_ordering in C++20 terms
        REPEATABLE
    };
        
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    class LocalMismatch : public Mismatch
    {
    };

	virtual bool IsSubclass( const Matcher *source_archetype ) const = 0;
	virtual bool IsLocalMatch( const Matcher *candidate ) const
	{
		// Default local matching criterion checks only the type of the candidate. If the
		// candidate's class is a (non-strict) subclass of this class, we have a match.
		return IsSubclass( candidate );
	}
    virtual ~Matcher() {}
    template< class TARGET_TYPE >
    static inline bool IsSubclassStatic( const TARGET_TYPE *target_archetype, const Matcher *source_archetype )
    {
        ASSERT( source_archetype );
        (void)target_archetype; // don't care about value of archetypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(source_archetype);
    }
    static CompareResult Compare( const Matcher *l, const Matcher *r, Ordering ordering = UNIQUE )
    {
        type_index l_index( typeid(*l) );
        type_index r_index( typeid(*r) );
        if( l_index != r_index )
            return (l_index > r_index) ? 1 : -1;
        return l->CovariantCompare(r, ordering);
    }
    virtual CompareResult CovariantCompare( const Matcher *candidate, Ordering ordering ) const 
    {
        return EQUAL; // usually there are no contents to compare
    }
};

#define MATCHER_FUNCTION \
    virtual bool IsSubclass( const Matcher *source_archetype ) const \
    { \
        return IsSubclassStatic( this, source_archetype ); \
    }

#endif

