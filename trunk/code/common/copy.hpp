#ifndef COPY_HPP
#define COPY_HPP

#include "common.hpp"

class Copier
{
public:
    static shared_ptr<Copier> Copy( shared_ptr<Copier> architype )
    {
        return Copy( architype.get() );
    }
    
    static shared_ptr<Copier> Copy( const Copier *architype )
    {
        return architype->CopyVirtual();
    }
    
    template< class TYPE >
    inline static shared_ptr<Copier> CopyConcrete( const TYPE *architype )
    {
        (void)architype; // don't care about value of architypes; just want the type
        return shared_ptr<TYPE>(new TYPE);
    }    

    virtual shared_ptr<Copier> CopyVirtual() const = 0;
};

#define COPY_FUNCTION \
    virtual shared_ptr<Copier> CopyVirtual() const  \
    { \
        return Copier::CopyConcrete( this ); \
    } 

#endif
