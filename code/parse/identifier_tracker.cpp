
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

void IdentifierTracker::Add(clang::Scope *S, clang::Declarator &D, shared_ptr<Node> rcp) 
{
  clang::IdentifierInfo *II = D.getIdentifier();
  TRACE("pushing identifier \"%s\"\n", D.getIdentifier()->getName() );
  
  // If there is no identifier associated with this declarator, bail out.
  if (II == 0) 
      return;
  
  Add(S, II, rcp);
}

void IdentifierTracker::Add(clang::Scope *S, clang::IdentifierInfo *II, shared_ptr<Node> rcp) 
{
  TypeNameInfo *weCurrentlyHaveTypeInfo = II->getFETokenInfo<TypeNameInfo>();
  TypeNameInfo *TI = new TypeNameInfo(weCurrentlyHaveTypeInfo, rcp);
  II->setFETokenInfo(TI);
  
  // Remember that this needs to be removed when the scope is popped.
  S->AddDecl(II);
}

/// ActOnPopScope - When a scope is popped, if any typedefs are now
/// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
void IdentifierTracker::PopScope(clang::Scope *S) 
{
  for (clang::Scope::decl_iterator I = S->decl_begin(), E = S->decl_end();
       I != E; ++I) 
  {
    clang::IdentifierInfo &II = *static_cast<clang::IdentifierInfo*>(*I);
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    ASSERT(TI && "This decl didn't get pushed??");
    if (TI) 
    {
      TypeNameInfo *Next = TI->Prev;
      delete TI;
      
      II.setFETokenInfo(Next);
    }
  }
}

shared_ptr<Node> IdentifierTracker::Get( const clang::IdentifierInfo &II )
{
    shared_ptr<Node> n = TryGet( II );
    ASSERT(n.get() && "This decl didn't get pushed??"); // could remove this
    return n;
}

shared_ptr<Node> IdentifierTracker::TryGet( const clang::IdentifierInfo &II )
{
    TypeNameInfo *TI = II.getFETokenInfo<TypeNameInfo>();
    if( TI )
        return TI->rcptr;
    else
        return shared_ptr<Node>();
}
