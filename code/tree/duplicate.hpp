#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "common/common.hpp"

class Duplicator
{
public:
    static shared_ptr<Duplicator> Duplicate( shared_ptr<Duplicator> architype )
    {
        return Duplicate( architype.get() );
    }
    
    static shared_ptr<Duplicator> Duplicate( const Duplicator *architype )
    {
        return architype->DuplicateVirtual();
    }
    
    template< class TYPE >
    inline static shared_ptr<Duplicator> DuplicateConcrete( const TYPE *architype )
    {
        (void)architype; // don't care about value of architypes; just want the type
        return shared_ptr<TYPE>(new TYPE);
    }    

    virtual shared_ptr<Duplicator> DuplicateVirtual() const = 0;
};

#define DUPLICATE_FUNCTION \
    virtual shared_ptr<Duplicator> DuplicateVirtual() const  \
    { \
        return Duplicator::DuplicateConcrete( this ); \
    } 

#endif
