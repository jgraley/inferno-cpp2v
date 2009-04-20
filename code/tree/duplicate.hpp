#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "common/common.hpp"

class Duplicator
{
public:
    static shared_ptr<Duplicator> Duplicate( shared_ptr<Duplicator> source )
    {
        return Duplicate( source.get() );
    }
    
    static shared_ptr<Duplicator> Duplicate( const Duplicator *source )
    {
        return source->DuplicateVirtual();
    }
    
    template< class TYPE >
    inline static shared_ptr<Duplicator> DuplicateConcrete( const TYPE *source )
    {
        shared_ptr<TYPE> p(new TYPE);
        *p = *source;
        return p;
    }    

    virtual shared_ptr<Duplicator> DuplicateVirtual() const = 0;
};

#define DUPLICATE_FUNCTION \
    virtual shared_ptr<Duplicator> DuplicateVirtual() const  \
    { \
        return Duplicator::DuplicateConcrete( this ); \
    } 

#endif
