#ifndef MATCH_HPP
#define MATCH_HPP

#include <typeinfo>
#include <string>
#include "common/trace.hpp"

/// Support class allowing hierarchical type comparisons between nodes
struct Matcher
{
	virtual bool IsSubclass( const Matcher *source_architype ) const = 0;
	virtual bool IsLocalMatch( const Matcher *candidate ) const
	{
		// Default local matching criterion checks only the type of the candidate. If the
		// candidate's class is a (non-strict) subclass of this class, we have a match.
		return IsSubclass( candidate );
	}
    virtual ~Matcher() {}
    template< class TARGET_TYPE >
    static inline bool IsSubclassStatic( const TARGET_TYPE *target_architype, const Matcher *source_architype )
    {
        ASSERT( source_architype );
        (void)target_architype; // don't care about value of architypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(source_architype);
    }
};

#define MATCHER_FUNCTION \
    virtual bool IsSubclass( const Matcher *source_architype ) const \
    { \
        return IsSubclassStatic( this, source_architype ); \
    }

#endif

