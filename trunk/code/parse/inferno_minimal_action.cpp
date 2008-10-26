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

#include "common/common.hpp"

#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Scope.h"

#include "inferno_minimal_action.hpp"

/// TypeNameInfo - A link exists here for each scope that an identifier is
/// defined.
struct TypeNameInfo {
  TypeNameInfo *Prev;
  bool isTypeName;
  shared_ptr<Node> rcptr;
  TypeNameInfo(bool istypename, TypeNameInfo *prev, shared_ptr<Node> rcp) {
    isTypeName = istypename;
    Prev = prev;
    rcptr = rcp;
  }
};

void InfernoMinimalAction::ActOnTranslationUnitScope(clang::SourceLocation Loc, clang::Scope *S) {
  TUScope = S;
  clang::IdentifierInfo *II;
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

/// isTypeName - This looks at the clang::IdentifierInfo::FETokenInfo field to
/// determine whether the name is a type name (objc class name or typedef) or
/// not in this scope.
shared_ptr<Node>
InfernoMinimalAction::isTypeNameima(const clang::IdentifierInfo &II, clang::Scope *S) {
  if (TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>())
    if (TI->isTypeName)
      return TI->rcptr;
  return shared_ptr<Node>();
}

/// ActOnDeclarator - If this is a typedef declarator, we modify the
/// clang::IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
/// popped.
clang::Action::DeclTy *
InfernoMinimalAction::ActOnDeclarator(clang::Scope *S, clang::Declarator &D, DeclTy *LastInGroup, shared_ptr<Node> rcp) 
{
  clang::IdentifierInfo *II = D.getIdentifier();
  //  printf("pushing identifier \"%s\"\n", D.getIdentifier()->getName() );
  // If there is no identifier associated with this declarator, bail out.
  if (II == 0) return 0;
  
  bool isTypeName = D.getDeclSpec().getStorageClassSpec() == clang::DeclSpec::SCS_typedef;
  AddNakedIdentifier(S, II, rcp, isTypeName); 
  return 0;
}

void InfernoMinimalAction::AddNakedIdentifier(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Node> rcp, bool istype) 
{
  TypeNameInfo *weCurrentlyHaveTypeInfo = II->getFETokenInfo<TypeNameInfo>();
  TypeNameInfo *TI = new TypeNameInfo(istype, weCurrentlyHaveTypeInfo, rcp);
  II->setFETokenInfo(TI);
  
  // Remember that this needs to be removed when the scope is popped.
  S->AddDecl(II);
}

clang::Action::DeclTy *
InfernoMinimalAction::ActOnStartClassInterface(clang::SourceLocation AtInterfaceLoc,
                                        clang::IdentifierInfo *ClassName,
                                        clang::SourceLocation ClassLoc,
                                        clang::IdentifierInfo *SuperName,
                                        clang::SourceLocation SuperLoc,
                                        DeclTy * const *ProtoRefs,
                                        unsigned NumProtocols,
                                        clang::SourceLocation EndProtoLoc,
                                        AttributeList *AttrList,
                                        shared_ptr<Node> rcp) {
  TypeNameInfo *TI =
    new TypeNameInfo(1, ClassName->getFETokenInfo<TypeNameInfo>(), rcp);

  ClassName->setFETokenInfo(TI);
  return 0;
}

/// ActOnForwardClassDeclaration - 
/// clang::Scope will always be top level file scope. 
clang::Action::DeclTy *
InfernoMinimalAction::ActOnForwardClassDeclaration(clang::SourceLocation AtClassLoc,
                                clang::IdentifierInfo **IdentList, unsigned NumElts,
                                shared_ptr<Node> rcp) {
  for (unsigned i = 0; i != NumElts; ++i) 
  {
    TypeNameInfo *TI =
      new TypeNameInfo(1, IdentList[i]->getFETokenInfo<TypeNameInfo>(), rcp);

    IdentList[i]->setFETokenInfo(TI);
  
    // Remember that this needs to be removed when the scope is popped.
    TUScope->AddDecl(IdentList[i]);
  }
  return 0;
}

/// ActOnPopScope - When a scope is popped, if any typedefs are now
/// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
void InfernoMinimalAction::ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S) {
  for (clang::Scope::decl_iterator I = S->decl_begin(), E = S->decl_end();
       I != E; ++I) {
    clang::IdentifierInfo &II = *static_cast<clang::IdentifierInfo*>(*I);
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    ASSERT(TI && "This decl didn't get pushed??");
    if (TI) {
      TypeNameInfo *Next = TI->Prev;
      delete TI;
      
      II.setFETokenInfo(Next);
    }
  }
}

shared_ptr<Node> InfernoMinimalAction::GetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II )
{
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    if( !TI )
        printf("Node has no info: \"%s\"\n", II.getName());
    ASSERT(TI && "This decl didn't get pushed??"); // could remove this
    if( TI )
        return TI->rcptr;
    else
        return shared_ptr<Node>();
}
