#ifndef SCTREE_HPP
#define SCTREE_HPP

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include <string>
#include <deque>
#include "node/node.hpp"
#include "clang/Parse/DeclSpec.h"
#include "tree/type_db.hpp"
#include "tree/cpptree.hpp"

namespace SCTree {

/// Base for SC nodes that correspond to named ements of SystemC
struct SCNamedConstruct
{
    virtual string GetName()=0; ///< Produce the name of the corresponding SC element for detection and rendering
};

struct SCNamedIdentifier : SCNamedConstruct
{
};

struct SCNamedRecord : SCNamedConstruct
{
};


struct Event : CPPTree::Type,
               SCNamedIdentifier
{
    NODE_FUNCTIONS_FINAL
    virtual string GetName() { return "sc_event"; }
};

struct Module : CPPTree::InheritanceRecord,
                SCNamedRecord
{
    NODE_FUNCTIONS_FINAL
    virtual string GetName() { return "sc_module"; }
};

struct Interface : CPPTree::InheritanceRecord,
                   SCNamedRecord
{
    NODE_FUNCTIONS_FINAL
    virtual string GetName() { return "sc_interface"; }
};

};

#endif
