#ifndef MISMATCH_HPP
#define MISMATCH_HPP

#include <exception>
	
// This is just an interface for mismatch exceptions. They should
// be subclassed inside each class that throws them. To throw outside 
// of a call to Engine::Compare() is an error.
class Mismatch : public std::exception
{
    virtual std::string Explanation() { return std::string(typeid(*this).name()); }
};    

#endif