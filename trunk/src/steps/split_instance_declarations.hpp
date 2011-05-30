#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "sr/search_replace.hpp"

// Where variables are declared in the statement body and are 
// initialised, move the decl into the decls body and initialise
// using an assignment.
class SplitInstanceDeclarations : public SearchReplace
{
public:
    SplitInstanceDeclarations();
};

// Where variables are declared in the statement body and are not
// initialised, move the decl into the decls body.
class MoveInstanceDeclarations : public SearchReplace
{
public:
    MoveInstanceDeclarations();
};

#endif

