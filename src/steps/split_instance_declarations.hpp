#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "vn/vn_step.hpp"

namespace Steps {

using namespace SR;

/** Where variables are declared in the statement body and are 
    initialised, move the decl into the decls body and initialise 
    using an assignment. */
class SplitInstanceDeclarations : public VNStep
{
public:
    SplitInstanceDeclarations();
};

/** Where variables are declared in the statement body and are not
    initialised, move the decl into the decls body. */
class MoveInstanceDeclarations : public VNStep
{
public:
    MoveInstanceDeclarations();
};

/** Where variables are declared in the decls collection and are 
    initialised, move the init into the statement body and initialise
    using an assignment. */
class SplitInstanceDeclarations2 : public VNStep
{
public:
    SplitInstanceDeclarations2();
};

}; // end namespace

#endif

