#ifndef TO_SC_METHOD_HPP
#define TO_SC_METHOD_HPP

#include "sr/search_replace.hpp"

namespace Steps {

class VarsToModule : public SearchReplace
{
public:
    VarsToModule();
};

class DeclsToModule : public SearchReplace
{
public:
    DeclsToModule();
};

class ThreadToMethod : public SearchReplace
{
public:
    ThreadToMethod();
};

}; // end namespace

#endif

