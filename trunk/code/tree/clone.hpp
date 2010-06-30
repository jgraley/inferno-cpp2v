#ifndef CLONE_HPP
#define CLONE_HPP

#include "common/common.hpp"


// NOTE: Duplicate uses shared_ptr so that it can be overloaded to return
// the supplied pointer (as done by SpecificIdentifier). Clone is guaranteed
// to return a new() object.
// TODO maybe use TreePtr
class Cloner
{
public:
    virtual shared_ptr<Cloner> Clone() const = 0;
    virtual shared_ptr<Cloner> Duplicate( shared_ptr<Cloner> p )
    {
    	return Clone(); // default duplication is to clone, but can be over-ridden
    }
    template< class TYPE >
    inline static shared_ptr<Cloner> CloneStatic( const TYPE *source )
    {
        return shared_ptr<Cloner>( new TYPE(*source) );
    }    
};

#define CLONE_FUNCTION \
    virtual shared_ptr<Cloner> Clone() const  \
    { \
        return Cloner::CloneStatic(this); \
    }
#endif
