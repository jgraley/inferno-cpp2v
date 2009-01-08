#include "common/common.hpp"
#include "clang/Parse/Parser.h"
#include "clang/Parse/DeclSpec.h"
#include "clang/Parse/Scope.h"
#include "identifier_tracker.hpp"


IdentifierTracker::IdentifierTracker( shared_ptr<Node> g ) :
    global( g )
{
    shared_ptr<TNode> ts( new TNode );    
    ts->cs = NULL;
    ts->parent = shared_ptr<TNode>(); 
    ts->II = NULL;
    ts->node = g;
    ts->decl = shared_ptr<Declaration>(); 
    tnodes.push_back( ts );
    TRACE("Global tnode t%p\n", ts.get() );
    current = ts;
}

shared_ptr<IdentifierTracker::TNode> IdentifierTracker::Find( shared_ptr<Node> node )
{
    ASSERT( node );
    shared_ptr<TNode> found;
    for( int i=0; i<tnodes.size(); i++ )
        if( tnodes[i]->node == node )
            return tnodes[i];
        
    return shared_ptr<TNode>();
}

void IdentifierTracker::PushScope( clang::Scope *S, shared_ptr<TNode> ts )
{
    ts->cs = S;
    ts->parent = current; 
    
    TRACE("push %s %p\n", ToString( ts ).c_str(), S );
    current = ts;    
}

void IdentifierTracker::PushScope( clang::Scope *S, shared_ptr<Node> n )
{
    shared_ptr<TNode> ts;
    ts = Find( n );
    ASSERT( ts );
    ASSERT( ts->parent == current ); // TODO I think the PopScope won't work properly if this isn't true
                                     // but what if the found scope is more than 1 level deep?
                                     // Perhaps push repeatedly and hope we get the right number of ActOnPopScope()?
    
    PushScope( S, ts );    
}

void IdentifierTracker::NewScope( clang::Scope *S, const clang::CXXScopeSpec *SS )
{
    // See if we already know about the "next_record" specified during parse.
    // If so, make this tnode be the parent. 
    shared_ptr<TNode> ts;
    if( next_record )
    {
        PushScope( S, next_record );
        return;        
    }

    // We were not warned about the new scope, so just make a new anonymous one.
    ts = shared_ptr<TNode>( new TNode );    
    ts->II = NULL;
    PushScope( S, ts );
}

void IdentifierTracker::SeenScope( clang::Scope *S )
{
 
    // Detect a change of scope and create a new scope if required. Do not do anything for
    // global scope (=no parent) - in that case, we leave the current scope as NULL
    ASSERT(S);
    if( S->getParent() && (!current || current->cs != S) )
    {
        clang::IdentifierInfo *fake_id = (clang::IdentifierInfo *)0xbad1dbad;    
        S->AddDecl(fake_id); // if scope has no decls, clang will not invoke ActOnPopScope()
        NewScope( S, NULL );
    }
}


void IdentifierTracker::Add( clang::IdentifierInfo *II, shared_ptr<Node> node, shared_ptr<Declaration> decl, clang::Scope *S, const clang::CXXScopeSpec *SS ) 
{
    SeenScope( S );
    
    // Make the TNode for this identifier and fill in    
    shared_ptr<TNode> i( new TNode );    
    i->II = II;
    i->node = node;
    i->decl = decl;
    i->parent = current;  
    i->cs = NULL; // Remember cs is the clang scope *owned* by i
    tnodes.push_back( i );
      
    TRACE("added %s (%p) at %s t%p\n", ToString( i ).c_str(), node.get(), ToString( current ).c_str(), current.get() );    
}

// Dump the current scope and move back to the parent
void IdentifierTracker::PopScope(clang::Scope *S) 
{
    TRACE("pop %s %p %p\n", ToString(current).c_str(), current->cs, S );
    if( current && current->cs == S ) // do not pop if we never pushed because didnt get an Add() for this scope
    {
        current = current->parent;
    }
    TRACE("done pop %s %p\n", ToString(current).c_str(), S );
}

// Just for debug; make a pretty string of the scope
string IdentifierTracker::ToString( shared_ptr<TNode> ts )
{
    if( !ts )
        return string("<nil>");

    string s;
    while( ts )
    {        
        if( ts->node == global )
            break;
            
        if( ts->II )
        {
            s = string(ts->II->getName()) + s;
        }
        else
        {
            char c[100];
            sprintf( c, "t%p", ts.get() );
            s = string(c) + s;
        }
        s = "::" + s;
        ts = ts->parent;        
    }
    
    if( s.empty() )
        s = "::"; // global scope
    
    return s;
}


#define NOMATCH 1000000

// Does identifier "II" found in scope "current" match rooted "ident"? if not return NOMATCH otherwise
// return the number of scopes we went down through from current to get a match (effectively a distance)
bool IdentifierTracker::IsIdentical( shared_ptr<TNode> current, shared_ptr<TNode> ident )
{
    return current == ident;
}

// Does identifier "II" found in scope "current" match rooted "ident"? if not return NOMATCH otherwise
// return the number of scopes we went down through from current to get a match (effectively a distance)
int IdentifierTracker::IsMatch( const clang::IdentifierInfo *II, shared_ptr<TNode> start, shared_ptr<TNode> ident, bool recurse )
{
    string cs, ips;
    cs = ToString( start );
    ASSERT( ident );
    ips = ToString( ident->parent );
    ASSERT( II ); // identifier being searched must have name
     
    if( !(ident->II) )
        return NOMATCH; // not all tnodes have a name eg global, which will never match here
        
    TRACE("Match %p(%s) %s %s%p(%s) ",
          II, II->getName(), 
          cs.c_str(),
          ips.c_str(), ident->II, ident->II->getName() );

    if( II != ident->II )
    {
        TRACE("NOMATCH name\n");
        return NOMATCH; // different strings
    }
    
    shared_ptr<TNode> cur_it = start; 
    shared_ptr<TNode> id_it = ident->parent;
    TRACE("t%p t%p t%p ", current.get(), start.get(), id_it.get() );
    int d = 0;
    
    // Try stepping out of the starting scope, one scope at a time,
    // until we match the identifier's scope.
    while( !IsIdentical( cur_it, id_it ) )
    {
        if( cur_it == NULL || !recurse )
        {
            TRACE("NOMATCH scope\n");
            return NOMATCH; 
        }
        cur_it = cur_it->parent;
        d++;
    }
    
    TRACE("%d\n", d);
    return d;
}


shared_ptr<Node> IdentifierTracker::Get( const clang::IdentifierInfo *II, clang::Scope *S, shared_ptr<Node> iscope, shared_ptr<Declaration> *decl, bool recurse )
{
    shared_ptr<Node> n = TryGet( II, S, iscope, decl, recurse );
    ASSERT(n.get() && "This decl didn't get pushed??"); // could remove this
    return n;
}


shared_ptr<Node> IdentifierTracker::TryGet( const clang::IdentifierInfo *II, clang::Scope *S, shared_ptr<Node> iscope, shared_ptr<Declaration> *decl, bool recurse )
{
    TRACE();
       
    // Choose the required identifier via a proximity search
    shared_ptr<TNode> start;
    if( iscope )
    {
        // A C++ style scope was supplied (iscope::II) - search in here and don't recurse
        TRACE("%p %p %p\n", iscope.get(), global.get(), tnodes[0]->node.get() );
        start = Find( iscope );
        ASSERT(start);
        recurse = false; // Never recurse with a C++ scope - they are exact specifications
    }    
    else
    {
        // No C++ scope, so use the current scope and recurse through parents
        start = current;
        ASSERT(start);
    }
        
    int best_distance=NOMATCH;
    shared_ptr<TNode> best_tnode;
    for( int i=0; i<tnodes.size(); i++ )
    {
        
        int distance = IsMatch( II, start, tnodes[i], recurse );
        if( distance < best_distance )
        {
            best_distance = distance;
            best_tnode = tnodes[i];            
        }
    }
 
    if( best_tnode )
    {
        if( decl )
            *decl = best_tnode->decl;
        return best_tnode->node;
    }
    else
    {
        if( decl )
            *decl = shared_ptr<Declaration>();
        return shared_ptr<Node>();
    }
}


// Untested
shared_ptr<Node> IdentifierTracker::FindMemberNode( const clang::IdentifierInfo *II, shared_ptr<Node> record )
{
    // Find the tnode for the record, based on the supplied node
    shared_ptr<TNode> found_record = Find( record );
        
    if( !found_record )
        return shared_ptr<Node>();
    
    // Find the member based on matching (a) the name and (b) the parent matching
    ASSERT( II );
    for( int i=0; i<tnodes.size(); i++ )
        if( tnodes[i]->II == II && tnodes[i]->parent == found_record )
        {
            return tnodes[i]->node;            
        }
        
    return shared_ptr<Node>();
}

