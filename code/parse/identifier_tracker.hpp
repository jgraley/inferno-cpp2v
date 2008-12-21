#ifndef IDENTIFIER_TRACKER
#define IDENTIFIER_TRACKER

#include "clang/Parse/Action.h"
#include "common/refcount.hpp"
#include "tree/tree.hpp"

// Semantic.
class DeclSpec;
class ObjCDeclSpec;
class clang::Declarator;
class AttributeList;
struct FieldDeclarator;
// Parse.
class Selector;
// Lex.
class Token;

/// IdentifierTracker - Minimal actions are used by light-weight clients of the
/// parser that do not need name resolution or significant semantic analysis to
/// be performed.  The actions implemented here are in the form of unresolved
/// identifiers.  By using a simpler interface than the SemanticAction class,
/// the parser doesn't have to build complex data structures and thus runs more
/// quickly.
class IdentifierTracker
{
public:
  /// Add - If this is a typedef declarator, we modify the
  /// clang::IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
  /// popped.
  void Add(clang::Scope *S, clang::Declarator &D,      shared_ptr<Node> rcp);
  void Add(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Node> rcp);  
  
  /// ActOnPopScope - When a scope is popped, if any typedefs are now 
  /// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
  virtual void PopScope(clang::Scope *S);
  
  // Extract the shared_ptr for the identifier. Where the identifier is differently declared
  // in nested scopes, we get the one that applies currently (which is the innermost one)  
  shared_ptr<Node> Get( const clang::IdentifierInfo &II );                                         
  
  // Version that just results NULL if identifier has not been added yet
  shared_ptr<Node> TryGet( const clang::IdentifierInfo &II );                                         
};

#endif
