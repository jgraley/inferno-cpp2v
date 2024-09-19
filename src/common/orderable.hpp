#ifndef ORDERABLE_HPP
#define ORDERABLE_HPP

#include <string>

using namespace std;


// Note: this is just the part of the orderings code that's relevent to tree node 
// designers. See also STLCompare3Way and the various Relations
class Orderable
{
public:
    typedef int Diff;

    enum OrderProperty
    {
        // Objects with different values must compare non-equal
        STRICT,     
        
        // Objects with different values may compare equal if this
        // avoids non-repeatable behaviour eg memory allocation dependencies 
        REPEATABLE
    };
    
    enum class BoundingRole
    {
        MINIMUS = -1, 
        NONE = 0,
        MAXIMUS = 1
        // Note: inclusiveness of MINIMUS and MAXIMUS is impl-defined, see rule #528
    };
    
    static Orderable::Diff OrderCompare3Way( const Orderable &l, 
                                             const Orderable &r, 
                                             OrderProperty order_property = STRICT );
    virtual Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                                   OrderProperty order_property ) const;
    virtual Orderable::Diff OrderCompare3WayChildren( const Orderable &right, 
                                                      OrderProperty order_property ) const;
    static string RoleToString( BoundingRole role );
};

#endif
