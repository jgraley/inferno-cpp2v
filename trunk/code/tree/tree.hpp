#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include <string>
#include <vector>

struct Node : public RCTarget,
              public clang::SourceLocation
{                 
};

struct Identifier : public Node,
                    public std::string
{
};

// ProgramElement is the base class for Statement and Declarator. There's 
// nothing in Clang or C++ BNF for this, but we need it to avoid 
// a multiple inheritance diamond because certain decls (eg int a;)
// can appear at top level and in fn bodies. 
struct ProgramElement : public Node
{
};

struct Declarator : public ProgramElement
{   
    RCPtr<Identifier> identifier;
};

template<typename ELEMENT>
struct Sequence : public Node,
                  public std::vector< RCPtr<ELEMENT> >
{
};                   

struct Type : public Node
{
};

struct VariableDeclarator : public Declarator
{
    RCPtr<Type> type;
    enum
    {
        DEFAULT,
        STATIC,
        AUTO
    } storage_class;
};

struct FunctionDeclarator : public Declarator
{
    RCPtr<Type> return_type;
    RCPtr< Sequence<ProgramElement> > body;
};

struct Program : public Sequence<ProgramElement>
{
};

struct Int : public Type
{
};

struct Char : public Type
{
};

#endif
