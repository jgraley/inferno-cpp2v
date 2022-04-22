#ifndef ORDERABLE_HPP
#define ORDERABLE_HPP

using namespace std;

class Orderable
{
public:
    typedef int Result;
    static const Result EQUAL = 0;

    enum OrderProperty
    {
        // Objects with different values must compare non-equal
        STRICT,     
        
        // Objects with different values may compare equal if this
        // avoids non-repeatable behaviour eg memory allocation dependencies 
        REPEATABLE
    };
    
    static Orderable::Result OrderCompare( const Orderable *l, 
                                           const Orderable *r, 
                                           OrderProperty order_property = STRICT );
    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const;
    virtual Orderable::Result OrderCompareChildren( const Orderable *candidate, 
                                                    OrderProperty order_property ) const;
    static bool OrderCompareEqual( const Orderable *l, 
                                   const Orderable *r, 
                                   OrderProperty order_property = STRICT );
};

#endif