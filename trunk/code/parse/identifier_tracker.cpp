
#include "common/common.hpp"

#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Scope.h"

#include "identifier_tracker.hpp"

/// TypeNameInfo - A link exists here for each scope that an identifier is
/// defined.
struct TypeNameInfo {
  TypeNameInfo *Prev;
  shared_ptr<Node> rcptr;
  TypeNameInfo(TypeNameInfo *prev, shared_ptr<Node> rcp) {
    Prev = prev;
    rcptr = rcp;
  }
};

void IdentifierTracker::ActOnTranslationUnitScope(clang::SourceLocation Loc, clang::Scope *S) 
{
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


/// ActOnDeclarator - If this is a typedef declarator, we modify the
/// clang::IdentifierInfo::FETokenInfo field to keep track of this fact, until S is
/// popped.
clang::Action::DeclTy *
IdentifierTracker::ActOnDeclarator(clang::Scope *S, clang::Declarator &D, clang::Action::DeclTy *LastInGroup, shared_ptr<Node> rcp) 
{
  clang::IdentifierInfo *II = D.getIdentifier();
  TRACE("pushing identifier \"%s\"\n", D.getIdentifier()->getName() );
  // If there is no identifier associated with this declarator, bail out.
  if (II == 0) return 0;
  
  AddNakedIdentifier(S, II, rcp); 
  return 0;
}

void IdentifierTracker::AddNakedIdentifier(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Node> rcp) 
{
  TypeNameInfo *weCurrentlyHaveTypeInfo = II->getFETokenInfo<TypeNameInfo>();
  TypeNameInfo *TI = new TypeNameInfo(weCurrentlyHaveTypeInfo, rcp);
  II->setFETokenInfo(TI);
  
  // Remember that this needs to be removed when the scope is popped.
  S->AddDecl(II);
}

/// ActOnPopScope - When a scope is popped, if any typedefs are now
/// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
void IdentifierTracker::ActOnPopScope(clang::SourceLocation Loc, clang::Scope *S) {
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

shared_ptr<Node> IdentifierTracker::GetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II )
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

shared_ptr<Node> IdentifierTracker::TryGetCurrentIdentifierRCPtr( const clang::IdentifierInfo &II )
{
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    if( TI )
        return TI->rcptr;
    else
        return shared_ptr<Node>();
}
