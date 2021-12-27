#ifndef LAZY_HPP
#define LAZY_HPP

#include "expression.hpp"

namespace SYM
{ 

template<typename EVALTYPE>
class Lazy : public Traceable
{
public:
    // Be strict and don't support assignment, default copy etc - 
    // because most of our operators are overloaded
    Lazy( shared_ptr<EVALTYPE> pexpr_ ) :
        pexpr( pexpr_ )
    {
    }

    operator shared_ptr<EVALTYPE>()
    {
        return pexpr;
    }
    
    string GetTrace() const
    {
        return pexpr->GetTrace(); // Skip the shared_ptr
    }

    shared_ptr<EVALTYPE> operator->()
    {
        return pexpr;
    }
    
    Lazy<EVALTYPE> operator &=( Lazy<EVALTYPE> other )
    {
        pexpr = shared_ptr<EVALTYPE>(*this & other);
        return *this;
    }

    Lazy<EVALTYPE> operator |=( Lazy<EVALTYPE> other )
    {
        pexpr = shared_ptr<EVALTYPE>(*this | other);
        return *this;
    }

private:    
    shared_ptr<EVALTYPE> pexpr;        
};


template<class T, class ... Args>
Lazy<typename T::NominalType> MakeLazy(Args && ... args)
{
    shared_ptr<T> ptr = make_shared<T>(args...);
    return Lazy<typename T::NominalType>(ptr);
}

};

#endif
