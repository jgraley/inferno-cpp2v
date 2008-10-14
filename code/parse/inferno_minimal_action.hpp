
#ifndef INFERNO_MINIMAL_ACTION
#define INFERNO_MINIMAL_ACTION

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

/// InfernoMinimalAction - Minimal actions are used by light-weight clients of the
/// parser that do not need name resolution or significant semantic analysis to
/// be performed.  The actions implemented here are in the form of unresolved
/// identifiers.  By using a simpler interface than the SemanticAction class,
/// the parser doesn't have to build complex data structures and thus runs more
/// quickly.
class InfernoMinimalAction : public clang::Action {
  /// Translation Unit clang::Scope - useful to Objective-C actions that need
  /// to lookup file scope declarations in the "ordinary" C decl namespace.
  /// For example, user-defined classes, built-in "id" type, etc.
  clang::Scope *TUScope;
  clang::IdentifierTable &Idents;
public:
  InfernoMinimalAction(clang::IdentifierTable &IT) : Idents(IT) {}
  
  /// isTypeName - This looks at the clang::IdentifierInfo::FETokenInfo field to
  /// determine whether the name is a typedef or not in this scope.
  virtual TypeTy *isTypeName(const clang::IdentifierInfo &II, clang::Scope *S);
  
  /// ActOnDeclarator - If this is a typedef declarator, we modify the
  /// clang::IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
  /// popped.
  virtual DeclTy *ActOnDeclarator(clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup, shared_ptr<Identifier> rcp);
  
  void AddNakedIdentifier(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Identifier> rcp, bool istype);  
  
  /// ActOnPopScope - When a scope is popped, if any typedefs are now 
  /// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
  virtual void ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S);
  virtual void ActOnTranslationUnitScope(clang::SourceLocation Loc, clang::Scope *S);
  
  virtual DeclTy *ActOnForwardClassDeclaration(clang::SourceLocation AtClassLoc,
                                               clang::IdentifierInfo **IdentList,
                                               unsigned NumElts,
                                               shared_ptr<Identifier> rcp);
  
  virtual DeclTy *ActOnStartClassInterface(clang::SourceLocation interLoc,
                                           clang::IdentifierInfo *ClassName,
                                           clang::SourceLocation ClassLoc,
                                           clang::IdentifierInfo *SuperName,
                                           clang::SourceLocation SuperLoc,
                                           DeclTy * const *ProtoRefs, 
                                           unsigned NumProtoRefs,
                                           clang::SourceLocation EndProtoLoc,
                                           AttributeList *AttrList, 
                                           shared_ptr<Identifier> rcp);
  
  // Extract the shared_ptr for the identifier. Where the identifier is differently declared
  // in nested scopes, we get the one that applies currently (which is the innermost one)  
  shared_ptr<Identifier> InfernoMinimalAction::GetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II );                                         
};

#endif
