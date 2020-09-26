#include "standard.hpp"
#include "mismatch.hpp"

#include <exception>
	

const char* Mismatch::what() const noexcept 
{ 
    w = What();
    return w.c_str(); 
}    


string Mismatch::What() const noexcept
{
    return Traceable::CPPFilt( typeid(*this).name() );
}
