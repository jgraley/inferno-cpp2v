#ifndef CLONE_HPP
#define CLONE_HPP

#include "common/common.hpp"

class TreeUtilsInterface;
template< class TYPE >
class Scaffold;

// NOTE: Duplicate uses shared_ptr so that it can be overloaded to return
// the supplied pointer (as done by SpecificIdentifier). Clone is guaranteed
// to return a new object.
/// Support class for allowing copies of nodes to be made
class Cloner
{
public: 
    virtual shared_ptr<Cloner> Clone() const = 0; 
    virtual shared_ptr<Cloner> Duplicate( shared_ptr<Cloner> p );
    
    template< class TYPE >
    inline static shared_ptr<Cloner> CloneStatic( const TYPE *source )
    {
        shared_ptr<Cloner> clone = make_shared<TYPE>(*source);        
        return clone;
    }    
};

// Outside of class to allow partial specialisation; PS prevents
// compiler spinning trying to make eg Scaffold<Scaffold<Scaffold<Scaffold<...
// by absorbing the Scaffold and returting utils for the templated type.
// TODO into own source file etc
template< class TYPE >
static const TreeUtilsInterface *MakeTreeUtilsStatic( const TYPE * );
template< class TYPE >
static const TreeUtilsInterface *MakeTreeUtilsStatic( const Scaffold<TYPE> * );


#define CLONE_FUNCTION \
    virtual shared_ptr<Cloner> Clone() const  \
    { \
        return Cloner::CloneStatic(this); \
    } \
    virtual const TreeUtilsInterface *MakeTreeUtils() const  \
    { \
        return ::MakeTreeUtilsStatic(this); \
    }
#endif
