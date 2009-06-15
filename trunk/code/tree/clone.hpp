#ifndef CLONE_HPP
#define CLONE_HPP

#include "common/common.hpp"

class Cloner
{
public:
    static shared_ptr<Cloner> Clone( shared_ptr<Cloner> source )
    {
        ASSERT( source );
        return Clone( source.get() );
    }
    
    static shared_ptr<Cloner> Clone( const Cloner *source )
    {
        ASSERT( source );
        return source->CloneVirtual();
    }
    
    template< class TYPE >
    inline static shared_ptr<Cloner> CloneStatic( const TYPE *source )
    {
        shared_ptr<TYPE> p(new TYPE);
        *p = *source;
        return p;
    }    

    virtual shared_ptr<Cloner> CloneVirtual() const = 0;
};

#define CLONE_FUNCTION \
	private: friend class Cloner; \
    virtual shared_ptr<Cloner> CloneVirtual() const  \
    { \
        return Cloner::CloneStatic( this ); \
    } \
    public:

#endif
