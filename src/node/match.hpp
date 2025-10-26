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

    // Returns true if `candidate` is a non-strict subcategory of the category upon which the method was called
    virtual bool IsSubcategory( const Matcher &candidate ) const = 0;
    
    virtual bool IsLocalMatch( const Matcher &candidate ) const;
    virtual bool IsLocalMatchCovariant( const Matcher &candidate ) const;
    virtual ~Matcher();
    template< class MY_TYPE >
    static inline bool IsSubcategoryStatic( const MY_TYPE &, const Matcher &candidate )
    {
        return !!dynamic_cast<const MY_TYPE *>(&candidate);
    }
    
    static bool IsEquivalentCategory( const Matcher &l, const Matcher &r );
};

#define MATCHER_FUNCTION \
    virtual bool IsSubcategory( const Matcher &candidate ) const \
    { \
        return Matcher::IsSubcategoryStatic( *this, candidate ); \
    }
#endif

