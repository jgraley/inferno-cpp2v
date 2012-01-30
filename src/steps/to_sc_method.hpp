#ifndef TO_SC_METHOD_HPP
#define TO_SC_METHOD_HPP

#include "sr/search_replace.hpp"

namespace Steps {

class AutosToModule : public SearchReplace
{
public:
    AutosToModule();
};

class TempsAndStaticsToModule : public SearchReplace
{
public:
    TempsAndStaticsToModule();
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

