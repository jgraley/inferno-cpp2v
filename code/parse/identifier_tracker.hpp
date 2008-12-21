
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
  /// Translation Unit clang::Scope - useful to Objective-C actions that need
  /// to lookup file scope declarations in the "ordinary" C decl namespace.
  /// For example, user-defined classes, built-in "id" type, etc.
  clang::Scope *TUScope;
  clang::IdentifierTable &Idents;

public:
  IdentifierTracker(clang::IdentifierTable &IT) : Idents(IT) {}
  
  /// ActOnDeclarator - If this is a typedef declarator, we modify the
  /// clang::IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
  /// popped.
  virtual clang::Action::DeclTy *ActOnDeclarator(clang::Scope *S, clang::Declarator &D, clang::Action::DeclTy *LastInGroup, shared_ptr<Node> rcp);
  
  void AddNakedIdentifier(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Node> rcp);  
  
  /// ActOnPopScope - When a scope is popped, if any typedefs are now 
  /// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
  virtual void ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S);
  virtual void ActOnTranslationUnitScope(clang::SourceLocation Loc, clang::Scope *S);
  
  // Extract the shared_ptr for the identifier. Where the identifier is differently declared
  // in nested scopes, we get the one that applies currently (which is the innermost one)  
  shared_ptr<Node> IdentifierTracker::GetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II );                                         
  
  // Version that just results NULL if identifier has not been added yet
  shared_ptr<Node> IdentifierTracker::TryGetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II );                                         
};

#endif
