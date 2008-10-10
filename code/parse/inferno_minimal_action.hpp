
#ifndef INFERNO_MINIMAL_ACTION
#define INFERNO_MINIMAL_ACTION

#include "clang/Parse/Action.h"
#include "common/refcount.hpp"

class Identifier; // this will be the identifier from the inferno tree. 
                  // Try not to include the tree here because we enter the clang namespace
                  // and there would be conflicts
typedef Identifier InfernoIdentifier;

namespace clang {

  // Semantic.
  class DeclSpec;
  class ObjCDeclSpec;
  class Declarator;
  class AttributeList;
  struct FieldDeclarator;
  // Parse.
  class Scope;
  class Action;
  class Selector;
  // Lex.
  class Token;

/// InfernoMinimalAction - Minimal actions are used by light-weight clients of the
/// parser that do not need name resolution or significant semantic analysis to
/// be performed.  The actions implemented here are in the form of unresolved
/// identifiers.  By using a simpler interface than the SemanticAction class,
/// the parser doesn't have to build complex data structures and thus runs more
/// quickly.
class InfernoMinimalAction : public Action {
  /// Translation Unit Scope - useful to Objective-C actions that need
  /// to lookup file scope declarations in the "ordinary" C decl namespace.
  /// For example, user-defined classes, built-in "id" type, etc.
  Scope *TUScope;
  IdentifierTable &Idents;
public:
  InfernoMinimalAction(IdentifierTable &IT) : Idents(IT) {}
  
  /// isTypeName - This looks at the IdentifierInfo::FETokenInfo field to
  /// determine whether the name is a typedef or not in this scope.
  virtual TypeTy *isTypeName(const IdentifierInfo &II, Scope *S);
  
  /// ActOnDeclarator - If this is a typedef declarator, we modify the
  /// IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
  /// popped.
  virtual DeclTy *ActOnDeclarator(Scope *S, Declarator &D, DeclTy *LastInGroup, RCPtr<InfernoIdentifier> rcp);
  
  /// ActOnPopScope - When a scope is popped, if any typedefs are now 
  /// out-of-scope, they are removed from the IdentifierInfo::FETokenInfo field.
  virtual void ActOnPopScope(SourceLocation Loc, Scope *S);
  virtual void ActOnTranslationUnitScope(SourceLocation Loc, Scope *S);
  
  virtual DeclTy *ActOnForwardClassDeclaration(SourceLocation AtClassLoc,
                                               IdentifierInfo **IdentList,
                                               unsigned NumElts,
                                               RCPtr<InfernoIdentifier> rcp);
  
  virtual DeclTy *ActOnStartClassInterface(SourceLocation interLoc,
                                           IdentifierInfo *ClassName,
                                           SourceLocation ClassLoc,
                                           IdentifierInfo *SuperName,
                                           SourceLocation SuperLoc,
                                           DeclTy * const *ProtoRefs, 
                                           unsigned NumProtoRefs,
                                           SourceLocation EndProtoLoc,
                                           AttributeList *AttrList, 
                                           RCPtr<InfernoIdentifier> rcp);
  
  // Extract the RCPtr for the identifier. Where the identifier is differently declared
  // in nested scopes, we get the one that applies currently (which is the innermost one)  
  RCPtr<InfernoIdentifier> InfernoMinimalAction::GetCurrentIdentifierRCPtr( const IdentifierInfo &II );                                         
};

}  // end namespace clang

#endif
