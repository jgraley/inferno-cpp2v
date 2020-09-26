#ifndef MISMATCH_HPP
#define MISMATCH_HPP

#include "standard.hpp"

#include <exception>
	
// This is just an interface for mismatch exceptions. They should
// be subclassed inside each class that throws them. To throw outside 
// of a call to Engine::Compare() is an error.
class Mismatch : public exception
{
public:    
    virtual const char* what() const noexcept;
    string What() const noexcept;
    
private:
    mutable string w;
};    

#endif
