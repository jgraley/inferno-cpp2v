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

struct SCConstruct {};

/// Base for SC nodes that correspond to named ements of SystemC
struct SCNamedConstruct : public SCConstruct
{
    virtual string GetName()=0; ///< Produce the name of the corresponding SC element for detection and rendering
};

struct SCNamedIdentifier : SCNamedConstruct {};
struct SCNamedRecord : SCNamedConstruct {};
struct SCNamedFunction : SCNamedConstruct {};


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

struct Wait : CPPTree::Statement,
              SCNamedFunction
{
    NODE_FUNCTIONS_FINAL
    virtual string GetName() { return "wait"; }
    TreePtr<CPPTree::Initialiser> event; ///< event to wait for or Uninitialised for static sens 
};

};

#endif
