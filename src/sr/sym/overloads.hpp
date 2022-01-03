#ifndef OVERLOADS_HPP
#define OVERLOADS_HPP

#include "expression.hpp"

namespace SYM
{ 

template<typename EVALTYPE>
class Over : public Traceable
{
public:
    // Be strict and don't support assignment, default copy etc - 
    // because most of our operators are overloaded
    Over( shared_ptr<EVALTYPE> pexpr_ = nullptr ) :
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
    
    Over<EVALTYPE> operator &=( Over<EVALTYPE> other )
    {
        pexpr = shared_ptr<EVALTYPE>(*this & other);
        return *this;
    }

    Over<EVALTYPE> operator |=( Over<EVALTYPE> other )
    {
        pexpr = shared_ptr<EVALTYPE>(*this | other);
        return *this;
    }

private:    
    shared_ptr<EVALTYPE> pexpr;        
};


template<class T, class ... Args>
Over<typename T::NominalType> MakeOver(Args && ... args)
{
    shared_ptr<T> ptr = make_shared<T>(args...);
    return Over<typename T::NominalType>(ptr);
}

};

#endif
