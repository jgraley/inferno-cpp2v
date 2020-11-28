#ifndef MISMATCH_HPP
#define MISMATCH_HPP

#include "standard.hpp"

#include <exception>
	
// This is just an interface for mismatch exceptions. They should
// be subclassed inside each class that throws them. 
// - Inside the SearchReplace algos, this is simply an indication 
//   that the current compare oprtation mismatched.
// - In tree helper utilities, it is an error indicating the 
//   operation could not be completed.
class Mismatch : public exception
{
public:    
    virtual const char* what() const noexcept;
    virtual string What() const noexcept;
    
private:
    mutable string w;
};    

#endif
