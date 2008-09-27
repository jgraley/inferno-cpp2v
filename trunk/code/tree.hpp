#ifndef TREE_HPP
#define TREE_HPP

#include "refcount.hpp"
#include "tree_pointers.hpp"
#include "clang/Basic/SourceLocation.h"
#include <string>

struct BaseNode : public RCTarget,
                  public clang::SourceLocation
{                 
    static const char *kind;
    virtual const char *GetKindOfBaseNode()=0;
};

struct Identifier : public BaseNode
{
    virtual const char *GetKindOfBaseNode() {return kind;}
    static const char *kind;
    std::string name;
};

struct Declarator : public BaseNode
{
    virtual const char *GetKindOfBaseNode() {return kind;}
    static const char *kind;
    virtual const char *GetKindOfDeclarator()=0;
    
    OwnerPtr<Identifier> identifier;
};

struct Type : public BaseNode
{
    virtual const char *GetKindOfBaseNode() {return kind;}
    static const char *kind;    
    virtual const char *GetKindOfType()=0;
};

struct VariableDeclarator : public Declarator
{
    virtual const char *GetKindOfDeclarator() {return kind;}
    static const char *kind;
    
    ShortcutPtr<Type> type;
    enum
    {
        STATIC,
        AUTO
    } storage_class;
};

struct FunctionDeclarator : public Declarator
{
    virtual const char *GetKindOfDeclarator() {return kind;}
    static const char *kind;
    ShortcutPtr<Type> return_type;
    OwnerPtr<Declarator> code;
};


struct Int : public Type
{
    virtual const char *GetKindOfType() {return kind;}
    static const char *kind;    
};

#endif
