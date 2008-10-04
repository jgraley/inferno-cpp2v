//===--- InfernoMinimalAction.cpp - Implement the InfernoMinimalAction class ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the InfernoMinimalAction interface.
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Scope.h"

#include "inferno_minimal_action.hpp"

using namespace clang;

/// TypeNameInfo - A link exists here for each scope that an identifier is
/// defined.
struct TypeNameInfo {
  TypeNameInfo *Prev;
  bool isTypeName;
  RCPtr<RCTarget> rcptr;
  TypeNameInfo(bool istypename, TypeNameInfo *prev, RCPtr<RCTarget> rcp) {
    isTypeName = istypename;
    Prev = prev;
    rcptr = rcp;
  }
};

void InfernoMinimalAction:: ActOnTranslationUnitScope(SourceLocation Loc, Scope *S) {
  TUScope = S;
  IdentifierInfo *II;
  TypeNameInfo *TI;
  
  // recognize the ObjC built-in type identifiers.
  /* JSG I don't like doing this here TODO if supporting objective-C, do 
     this somewhere else (and make sure the actions get invoked)
  II = &Idents.get("id");
  TI = new TypeNameInfo(1, II->getFETokenInfo<TypeNameInfo>());
  II->setFETokenInfo(TI);
  II = &Idents.get("SEL");
  TI = new TypeNameInfo(1, II->getFETokenInfo<TypeNameInfo>());
  II->setFETokenInfo(TI);
  II = &Idents.get("Class");
  TI = new TypeNameInfo(1, II->getFETokenInfo<TypeNameInfo>());
  II->setFETokenInfo(TI);
  II = &Idents.get("Protocol");
  TI = new TypeNameInfo(1, II->getFETokenInfo<TypeNameInfo>());
  II->setFETokenInfo(TI);
  */
}

/// isTypeName - This looks at the IdentifierInfo::FETokenInfo field to
/// determine whether the name is a type name (objc class name or typedef) or
/// not in this scope.
Action::TypeTy *
InfernoMinimalAction::isTypeName(const IdentifierInfo &II, Scope *S) {
  if (TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>())
    if (TI->isTypeName)
      return TI;
  return 0;
}

/// ActOnDeclarator - If this is a typedef declarator, we modify the
/// IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
/// popped.
Action::DeclTy *
InfernoMinimalAction::ActOnDeclarator(Scope *S, clang::Declarator &D, DeclTy *LastInGroup, RCPtr<RCTarget> rcp) {
  IdentifierInfo *II = D.getIdentifier();
  
  // If there is no identifier associated with this declarator, bail out.
  if (II == 0) return 0;
  
  TypeNameInfo *weCurrentlyHaveTypeInfo = II->getFETokenInfo<TypeNameInfo>();
  bool isTypeName = D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef;

  TypeNameInfo *TI = new TypeNameInfo(isTypeName, weCurrentlyHaveTypeInfo, rcp);

  II->setFETokenInfo(TI);
  
  // Remember that this needs to be removed when the scope is popped.
  S->AddDecl(II);

  return 0;
}

Action::DeclTy *
InfernoMinimalAction::ActOnStartClassInterface(SourceLocation AtInterfaceLoc,
                                        IdentifierInfo *ClassName,
                                        SourceLocation ClassLoc,
                                        IdentifierInfo *SuperName,
                                        SourceLocation SuperLoc,
                                        DeclTy * const *ProtoRefs,
                                        unsigned NumProtocols,
                                        SourceLocation EndProtoLoc,
                                        AttributeList *AttrList,
                                        RCPtr<RCTarget> rcp) {
  TypeNameInfo *TI =
    new TypeNameInfo(1, ClassName->getFETokenInfo<TypeNameInfo>(), rcp);

  ClassName->setFETokenInfo(TI);
  return 0;
}

/// ActOnForwardClassDeclaration - 
/// Scope will always be top level file scope. 
Action::DeclTy *
InfernoMinimalAction::ActOnForwardClassDeclaration(SourceLocation AtClassLoc,
                                IdentifierInfo **IdentList, unsigned NumElts,
                                RCPtr<RCTarget> rcp) {
  for (unsigned i = 0; i != NumElts; ++i) {
    TypeNameInfo *TI =
      new TypeNameInfo(1, IdentList[i]->getFETokenInfo<TypeNameInfo>(), rcp);

    IdentList[i]->setFETokenInfo(TI);
  
    // Remember that this needs to be removed when the scope is popped.
    TUScope->AddDecl(IdentList[i]);
  }
  return 0;
}

/// ActOnPopScope - When a scope is popped, if any typedefs are now
/// out-of-scope, they are removed from the IdentifierInfo::FETokenInfo field.
void InfernoMinimalAction::ActOnPopScope(SourceLocation Loc, Scope *S) {
  for (Scope::decl_iterator I = S->decl_begin(), E = S->decl_end();
       I != E; ++I) {
    IdentifierInfo &II = *static_cast<IdentifierInfo*>(*I);
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    assert(TI && "This decl didn't get pushed??");
    
    if (TI) {
      TypeNameInfo *Next = TI->Prev;
      delete TI;
      
      II.setFETokenInfo(Next);
    }
  }
}

RCPtr<RCTarget> InfernoMinimalAction::GetCurrentIdentifierRCPtr( const IdentifierInfo &II )
{
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    assert(TI && "This decl didn't get pushed??"); // could remove this
    if( TI )
        return TI->rcptr;
    else
        return RCPtr<RCTarget>();
}
