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
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    class LocalMismatch : public Mismatch
    {
    };

	virtual bool IsSubclass( const Matcher *source_architype ) const = 0;
	virtual bool IsLocalMatch( const Matcher *candidate ) const
	{
		// Default local matching criterion checks only the type of the candidate. If the
		// candidate's class is a (non-strict) subclass of this class, we have a match.
		return IsSubclass( candidate );
	}
	virtual void CheckLocalMatch( const Matcher *candidate ) const
	{
        if( !IsLocalMatch( candidate ) )
            throw LocalMismatch();
	}
    virtual ~Matcher() {}
    template< class TARGET_TYPE >
    static inline bool IsSubclassStatic( const TARGET_TYPE *target_architype, const Matcher *source_architype )
    {
        ASSERT( source_architype );
        (void)target_architype; // don't care about value of architypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(source_architype);
    }
    static CompareResult Compare( const Matcher *l, const Matcher *r )
    {
        type_index l_index( typeid(*l) );
        type_index r_index( typeid(*r) );
        if( l_index != r_index )
            return (l_index > r_index) ? 1 : -1;
        return l->CovariantCompare(r);
    }
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const 
    {
        return EQUAL; // usually there are no contents
    }
};

#define MATCHER_FUNCTION \
    virtual bool IsSubclass( const Matcher *source_architype ) const \
    { \
        return IsSubclassStatic( this, source_architype ); \
    }

#endif

