#ifndef DYNAMIC_NEW_HPP
#define DYNAMIC_NEW_HPP

#include "common.hpp"

class DynamicNew
{
public:
    static shared_ptr<DynamicNew> MakeNew( shared_ptr<DynamicNew> architype )
    {
        return MakeNew( architype.get() );
    }
    
    static shared_ptr<DynamicNew> MakeNew( const DynamicNew *architype )
    {
        return architype->MakeNewVirtual();
    }
    
    template< class TYPE >
    inline static shared_ptr<DynamicNew> MakeNewConcrete( const TYPE *architype )
    {
        (void)architype; // don't care about value of architypes; just want the type
        return shared_ptr<TYPE>(new TYPE);
    }    

    virtual shared_ptr<DynamicNew> MakeNewVirtual() const = 0;
};

#define DYNAMIC_NEW_FUNCTION \
    virtual shared_ptr<DynamicNew> MakeNewVirtual() const  \
    { \
        return DynamicNew::MakeNewConcrete( this ); \
    } 

#endif
