#ifndef ORDERABLE_HPP
#define ORDERABLE_HPP

using namespace std;

typedef int CompareResult;
static const CompareResult EQUAL = 0;

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

#endif