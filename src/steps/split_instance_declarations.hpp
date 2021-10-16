#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "sr/vn_transformation.hpp"

namespace Steps {

using namespace SR;

/** Where variables are declared in the statement body and are 
    initialised, move the decl into the decls body and initialise 
    using an assignment. */
class SplitInstanceDeclarations : public VNTransformation
{
public:
    SplitInstanceDeclarations();
};

/** Where variables are declared in the statement body and are not
    initialised, move the decl into the decls body. */
class MoveInstanceDeclarations : public VNTransformation
{
public:
    MoveInstanceDeclarations();
};

/** Where variables are declared in the decls collection and are 
    initialised, move the init into the statement body and initialise
    using an assignment. */
class SplitInstanceDeclarations2 : public VNTransformation
{
public:
    SplitInstanceDeclarations2();
};

}; // end namespace

#endif

