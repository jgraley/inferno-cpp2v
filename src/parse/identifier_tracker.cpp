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
    tnodes.push_back( ts );
    TRACE("Global tnode %s\n", ToString(ts).c_str() );
    scope_stack.push( ts );
}

shared_ptr<IdentifierTracker::TNode> IdentifierTracker::Find( shared_ptr<Node> node )
{
    ASSERT( node );
    shared_ptr<TNode> found;
    for( int i=0; i<tnodes.size(); i++ )
    {
        //TRACE("%d of %d T%p\n", i, tnodes.size(), tnodes[i].get() );
        if( tnodes[i]->node == node )
            return tnodes[i];
    }
    
    return shared_ptr<TNode>();
}

void IdentifierTracker::PushScope( clang::Scope *S, shared_ptr<Node> n )
{
    shared_ptr<TNode> ts;
    if( n )
    {
        ts = Find( n );
    }
    else
    {
        ts = shared_ptr<TNode>( new TNode );    
        ts->cs = NULL;
        ts->parent = shared_ptr<TNode>(); 
        ts->II = NULL;
        ts->node = shared_ptr<Node>();
    }
        
    ASSERT( ts );
    PushScope( S, ts );    
}


void IdentifierTracker::PushScope( clang::Scope *S, shared_ptr<TNode> ts )
{
    // If scope has no decls, clang will not invoke ActOnPopScope()
    // so we set a non-NULL rubbish value in there whenever we push, in order
    // to be sure of getting a corresponding pop
    TRACE("push new=%s clang=S%p top=%s\n", ToString( ts ).c_str(), S, ToString( scope_stack.top() ).c_str() );
    
    if( S )
    {
        clang::IdentifierInfo *fake_id = (clang::IdentifierInfo *)0xbad1dbad;    
        S->AddDecl(fake_id); 
        ts->cs = S;
    }
    else
    {
        ASSERT( ts->cs ); 
    }
    
    //ASSERT( ts->parent == scope_stack.top() );     
    scope_stack.push( ts );    
}


// Dump the current scope and move back to the parent
void IdentifierTracker::PopScope(clang::Scope *S) 
{
    TRACE("pop top=%s clang=S%p\n", ToString(scope_stack.top()).c_str(), S );
    if( !scope_stack.empty() && scope_stack.top() && (!S || scope_stack.top()->cs == S) ) // do not pop if we never pushed because didnt get an Add() for this scope
    {
        scope_stack.pop();
    }
    //TRACE("done pop %s S%p\n", ToString(scope_stack.top()).c_str(), S );
}


void IdentifierTracker::NewScope( clang::Scope *S )
{
    // See if we already know about the "next_record" specified during parse.
    // If so, make this tnode be the parent. 
    if( !next_record.empty() && (S->getFlags() & clang::Scope::CXXClassScope) ) // Only use next_record if scope is actually a Record
    {
        TRACE("got next_record; \n");
        ASSERT( next_record.top() ); // probably tried to create more than one scope for this record
        PushScope( S, next_record.top() );
        next_record.pop();
        next_record.push( shared_ptr<Node>() );   // Remove since we've used it and shouldn't use it again     
    }
    else
    {
        // We were not warned about the new scope, so just make a new anonymous one.
        TRACE("no next_record; \n");
        shared_ptr<TNode> ts = shared_ptr<TNode>( new TNode );    
        ts->II = NULL;
        ts->parent = scope_stack.top();
        PushScope( S, ts );
    }
}

void IdentifierTracker::SeenScope( clang::Scope *S )
{
    // Detect a change of scope and create a new scope if required. Do not do anything for
    // global scope (=no parent) - in that case, we leave the current scope as NULL
    ASSERT(S);
    if( S->getParent() && (scope_stack.empty() || !scope_stack.top() || scope_stack.top()->cs != S) )
    {
        TRACE("Seen new clang=S%p", S);
        if( !scope_stack.empty() && scope_stack.top() )
            TRACE(" top=")(ToString(scope_stack.top()));
        TRACE("\n");
       NewScope( S );
    }
}


void IdentifierTracker::Add( clang::IdentifierInfo *II, shared_ptr<Node> node, clang::Scope *S ) 
{
    SeenScope( S );
    
    // Make the TNode for this identifier and fill in    
    shared_ptr<TNode> i( new TNode );    
    i->II = II;
    i->node = node;
    i->parent = scope_stack.top();  
    i->cs = NULL; // Remember cs is the clang scope *owned* by i
    tnodes.push_back( i );
      
    TRACE("added %s new=%p top=%s clang=S%p\n", ToString( i ).c_str(), node.get(), ToString( scope_stack.top() ).c_str(), S );    
}


// Just for debug; make a pretty string of the scope
string IdentifierTracker::ToString( shared_ptr<TNode> tss )
{
    if( !tss )
        return string("T<nil>");

    string s;
    int i=0;
    shared_ptr<TNode> ts = tss;
    while( ts )
    {        
        if( ts->node == global )
            break;
        string ss;    
        if( ts->II )
        {
            ss = string(ts->II->getName()) + s;
        }
        else
        {
            char c[100];
            sprintf( c, "T%p", ts.get() );
            ss = string(c) + s;
        }
        s = "::" + ss;
        ts = ts->parent;        
        i++;
        ASSERT( i<=tnodes.size() )( "TNode loop detected!!!!!!!!! OH NO!!!!!1\n" );
    }
    
    if( s.empty() )
        s = "::"; // global scope
    
    s = SSPrintf("T%pS%p\"", tss.get(), tss->cs) + s + "\"";

    if( tss->parent )
        s += SSPrintf("->T%p", tss->parent.get() );
    
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
        
    if( II != ident->II )
    {
        return NOMATCH; // different strings
    }
    
    shared_ptr<TNode> cur_it = start; 
    shared_ptr<TNode> id_it = ident->parent;
    int d = 0;
    
    // Try stepping out of the starting scope, one scope at a time,
    // until we match the identifier's scope.
    while( !IsIdentical( cur_it, id_it ) )
    {
        if( cur_it == NULL || !recurse )
        {
            return NOMATCH; 
        }
        cur_it = cur_it->parent;
        d++;
    }
    
    return d;
}


shared_ptr<Node> IdentifierTracker::Get( const clang::IdentifierInfo *II, shared_ptr<Node> iscope, bool recurse )
{
    ASSERT(II);
    TRACE();
    shared_ptr<Node> n = TryGet( II, iscope, recurse );
    if( !n ) // All just tracing to see what went wrong
    {
        TRACE("recurse=%s\n", recurse?"true":"false");
        if(iscope)
        {
            TRACE("iscope: ")(*iscope)("\n");
        }
        else
        {
            FOREACH( shared_ptr<TNode> x, tnodes )
            {
                TRACE(ToString(x))("\n");
            }
        }
    }
    ASSERT(n)("Decl not found : ")(II->getName()); 
    return n;
}


shared_ptr<Node> IdentifierTracker::TryGet( const clang::IdentifierInfo *II, shared_ptr<Node> iscope, bool recurse )
{
    ASSERT(II);
    
    // Choose the required identifier via a proximity search
    shared_ptr<TNode> start;
    if( iscope )
    {
        // A C++ style scope was supplied (iscope::II) - search in here and don't recurse
        start = Find( iscope );
        ASSERT(start);
        recurse = false; // Never recurse with a C++ scope - they are exact specifications
    }    
    else
    {
        // No C++ scope, so use the current scope and recurse through parents
        start = scope_stack.top();
        ASSERT(start);
    }

    TRACE("TryGet ")(II->getName())(" in scope ")(ToString(start))(" only, ")(iscope?"C++ ":"")(recurse?"":"not ")("recursive\n"); 
        
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
        return best_tnode->node;
    else
        return shared_ptr<Node>();
}


/* Untested
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
*/
