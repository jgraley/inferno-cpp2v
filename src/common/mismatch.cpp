#include "standard.hpp"
#include "mismatch.hpp"

#include <exception>
	

const char* Exception::what() const noexcept 
{ 
    w = What();
    return w.c_str(); 
}    


string Exception::What() const noexcept
{
    return Traceable::TypeIdName( *this );
}
