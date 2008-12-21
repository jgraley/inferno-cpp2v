
#include "common/common.hpp"
#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Scope.h"
#include "identifier_tracker.hpp"

typedef stack< shared_ptr<Node> > Identifiers;

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
  Identifiers *ti = II->getFETokenInfo<Identifiers>();
  if( !ti )
  {
    ti = new Identifiers;
    II->setFETokenInfo(ti);
  }
    
  ti->push( rcp );  
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
    Identifiers *ti = II.getFETokenInfo<Identifiers>();
    ASSERT(ti && "type name info missing in pop???");
    ti->pop();
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
    Identifiers *ti = II.getFETokenInfo<Identifiers>();
    if( ti )
        return ti->top();
    else
        return shared_ptr<Node>();
}
