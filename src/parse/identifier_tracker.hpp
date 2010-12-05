#ifndef IDENTIFIER_TRACKER
#define IDENTIFIER_TRACKER

#include "clang/Parse/Action.h"
#include "rc_hold.hpp"
#include "tree/tree.hpp"

// Semantic.
class DeclSpec;
class ObjCDeclSpec;
class AttributeList;
struct FieldDeclarator;
// Parse.
class Selector;
// Lex.
class Token;


class IdentifierTracker
{
    // The scope tree is made up of these. Each one represents an object, user type
    // or compound statement. II is null if no name; parent is null if in global (root)
    // scope. A TNode t should only have other TNodes pointing to it if it is a scope.
    // Example:
    // class C { void F() { struct S { int X; }; } };
    // has parent pointers as: X -> S -> F -> C -> (NULL)
    // F has no identifier info, since it's name is not relevent to scopes (anonymous scope)
    // All should have valid node pointers (TODO fill in for compound statements)
    // C, F, S should have clang scope cs filled in.      
	// TODO use Scope and Identifier instead of Node everywhere
    struct TNode
    {
        shared_ptr<TNode> parent;
        shared_ptr<Node> node;
        clang::Scope *cs; // Note: this is the *corresponding* scope, not the containing scope.
                          // Eg given struct A { int B; }; then A->cs is the struct scope and B->cs is NULL
        clang::IdentifierInfo *II;
    };
    
    // Our best effort to determine the current scope
    shared_ptr<TNode> current;
    
    // Every TNode we ever create goes in this list, and is never deleted. 
    deque< shared_ptr<TNode> > tnodes; 
    
    // Parser can "warn" us that the next clang::Scope we see will correspond to
    // the supplied node (a Record node in fact).
    stack< shared_ptr<Node> > next_record;

    // Enter a new scope - clang doesn't tell us when to do this, so we deduce from
    // calls to Add.
    shared_ptr<TNode> Find( shared_ptr<Node> node );
    void PushScope( clang::Scope *S, shared_ptr<TNode> ts );
    void NewScope( clang::Scope *S );
    string ToString( shared_ptr<TNode> ts );
    bool IsIdentical( shared_ptr<TNode> current, shared_ptr<TNode> ident );
    int IsMatch( const clang::IdentifierInfo *II, shared_ptr<TNode> current, shared_ptr<TNode> ident, bool recurse );

    shared_ptr<Node> global;

public:
    IdentifierTracker( shared_ptr<Node> g );
    
    /// Associate supplied node with supplied identifier and scope. Will remain 
    /// until the scope is popped. S must be current scope due to implementation.
    void Add( clang::IdentifierInfo *II, shared_ptr<Node> node, clang::Scope *S );
  
    /// Push a scope based on supplied Inferno tree Node
    void PushScope( clang::Scope *S, shared_ptr<Node> n );

    /// Let identifier tracker know we saw a scope. We must do this before calling other functions so we
    /// catch all the scope changes that are not communicated to us any other way.
    void SeenScope( clang::Scope *S );

    /// ActOnPopScope - When a scope is popped, if any typedefs are now 
    /// out-of-scope, they are removed from the clang::IdentifierInfo::FETokenInfo field.
    virtual void PopScope(clang::Scope *S);
  
    // Extract the Declaration for the clang::Identifier. Where the identifier is differently declared
    // in nested scopes, we get the one that applies currently (which is the innermost one)  
    // Optionally: Can specify a C++ scope, which must match exactly (NULL, falls back to current scope)
    //             Can ask for the corresponding decl node for the found node
    //             Can turn off recursion so only a direct match allowed
    shared_ptr<Node> Get( const clang::IdentifierInfo *II, shared_ptr<Node> iscope = shared_ptr<Node>(), bool recurse = true );                                         
  
    // Version that just results NULL if identifier has not been added yet
    shared_ptr<Node> TryGet( const clang::IdentifierInfo *II, shared_ptr<Node> iscope = shared_ptr<Node>(), bool recurse = true );      
    
    // Indicate that the next Add() call will have the supplied node as parent.
    // Omit to clear (eg after the struct)
    void SetNextRecord( shared_ptr<Node> n = shared_ptr<Node>() )
    {
        TRACE("next record is %p\n", n.get() );
        
        if( n )
            next_record.push(n);
        else
        {
            ASSERT( !next_record.empty() );
            next_record.pop();
        }
    }                             
    
    shared_ptr<Node> GetCurrent()
    {
        return current->node;
    }
};

#endif
