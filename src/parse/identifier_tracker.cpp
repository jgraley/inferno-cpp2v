#include "common/common.hpp"

#include "include_clang_llvm.hpp"
#include "identifier_tracker.hpp"


IdentifierTracker::IdentifierTracker( shared_ptr<Node> g )
{
	global = g;
    auto ts = make_shared<TNode>();    
    ts->cs = nullptr; // Will fix in SeenScope
    ts->parent = shared_ptr<TNode>(); 
    ts->II = nullptr;
    ts->node = g;
    tnodes.push_back( ts );
    scope_stack.push( ts );
    //TRACE("Global tnode ")(ts)("\n");  
}

shared_ptr<IdentifierTracker::TNode> IdentifierTracker::Find( shared_ptr<Node> node )
{
    ASSERT( node );
    shared_ptr<TNode> found;
    for( deque< shared_ptr<TNode> >::size_type i=0; i<tnodes.size(); i++ )
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
        ts = make_shared<TNode>();    
        ts->cs = nullptr;
        ts->parent = shared_ptr<TNode>(); 
        ts->II = nullptr;
        ts->node = shared_ptr<Node>();
    }
        
    ASSERT( ts );
    PushScope( S, ts );    
}


void IdentifierTracker::PushScope( clang::Scope *S, shared_ptr<TNode> ts )
{
	INDENT("u");
    // If scope has no decls, clang will not invoke ActOnPopScope()
    // so we set a non-nullptr rubbish value in there whenever we push, in order
    // to be sure of getting a corresponding pop
    TRACE("stack=")(scope_stack)("\npush new=")(ts)(" clang=S%p\n", S);
    ASSERT( ts ); 
    ASSERT( S );    
    clang::IdentifierInfo *fake_id = (clang::IdentifierInfo *)0xbad1dbad;    
    S->AddDecl(fake_id); 
    ts->cs = S;
    
    //ASSERT( ts->parent == scope_stack.top() );    
    scope_stack.push( ts );    
}


// Dump the current scope and move back to the parent
void IdentifierTracker::PopScope(clang::Scope *S) 
{
	INDENT("o");
    TRACE("stack=")(scope_stack)("\npop top, clang=S%p\n", S);
    ASSERT( S );
    
    // Now stricter after #802
    ASSERT( !scope_stack.empty() );
    ASSERT( scope_stack.top()->cs == S ); 
	
	TRACE("popping\n");
    scope_stack.pop();
    TRACE("after pop stack=")(scope_stack)("\n");
}


void IdentifierTracker::NewScope( clang::Scope *S )
{
    ASSERT( S );
	INDENT("n");
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
        auto ts = make_shared<TNode>();    
        ts->II = nullptr;
		ts->cs = S;
        ts->parent = scope_stack.empty()?shared_ptr<TNode>():scope_stack.top();
        PushScope( S, ts );
    }
}

void IdentifierTracker::SeenScope( clang::Scope *S )
{
	INDENT("s");
    // Detect a change of scope and create a new scope if required. Do not do anything for
    // global scope (=no parent) - in that case, we leave the current scope as nullptr
    TRACE("stack=")(scope_stack)("\nseen clang=S%p clang-parent=S%p\n", S, S->getParent());
    ASSERT(S);
    
    if( S->getParent() == nullptr && scope_stack.size() == 1 ) 
    {
		ASSERT( scope_stack.top()->node == global )(scope_stack)(" S%p", S);
		scope_stack.top()->cs = S;
	}

    if( scope_stack.top()->cs != S )
    {
        TRACE("Pushing S%p\n", S);
        NewScope( S );
    }
}


void IdentifierTracker::Add( clang::IdentifierInfo *II, shared_ptr<Node> node, clang::Scope *S ) 
{
    SeenScope( S );
    
    // Make the TNode for this identifier and fill in    
    auto i = make_shared<TNode>();    
    i->II = II;
    i->node = node;
    i->parent = scope_stack.top();  
    i->cs = nullptr; // Remember cs is the clang scope *owned* by i
    tnodes.push_back( i );
      
    //TRACE("stack=")(scope_stack)("\nadded %s new=%p clang=S%p\n", ToString( i ).c_str(), node.get(), S );    
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
int IdentifierTracker::IsIdentifierMatch( const clang::IdentifierInfo *to_find_II, shared_ptr<TNode> start_scope, shared_ptr<TNode> candidate, bool recurse )
{
    //string cs, ips;
    ASSERT( candidate );
    ASSERT( to_find_II ); // identifier being searched must have name

	TRACE("Candidate ")(candidate);		
     
    if( !(candidate->II) )
    {
		TRACE(" has no name\n");		
        return NOMATCH; // not all tnodes have a name eg global, which will never match here
    }
        
    if( to_find_II != candidate->II )
    {
		TRACE(" name mismatches: to find:")(to_find_II->getName())("\n");
        return NOMATCH; // different strings
    }
    
    shared_ptr<TNode> current_scope = start_scope; 
    shared_ptr<TNode> candidate_scope = candidate->parent;    
    int d = 0;
    
    TRACE(" name matches\nso check scopes: start_scope=")(start_scope)(" and candidate_scope=")(candidate_scope)("\n");
    
    // Try stepping out of the starting scope, one scope at a time,
    // until we match the identifier's scope.
    while( !IsIdentical( current_scope, candidate_scope ) )
    {
        if( current_scope == nullptr || !recurse )
        {
            return NOMATCH; 
        }
        current_scope = current_scope->parent;
        d++;
    }
    
    return d;
}


shared_ptr<Node> IdentifierTracker::Get( const clang::IdentifierInfo *to_find_II, shared_ptr<Node> cpp_scope_node )
{
    ASSERT(to_find_II);
    TRACE();
    shared_ptr<Node> n = TryGet( to_find_II, cpp_scope_node, true );
/*    if( !n ) // All just tracing to see what went wrong
    {
        TRACE("recurse=%s\n", recurse?"true":"false");
        if(cpp_scope_node)
        {
            TRACE("cpp_scope_node: ")(*iscope)("\n");
        }
        else
        {
            for( shared_ptr<TNode> x : tnodes )
            {
                TRACE(x)("\n");
            }
        }
    }*/
    ASSERT(n)("Decl not found : ")(to_find_II->getName()); 
    return n;
}


shared_ptr<Node> IdentifierTracker::TryGet( const clang::IdentifierInfo *to_find_II, shared_ptr<Node> cpp_scope_node, bool recurse )
{
	INDENT("t");
    ASSERT(to_find_II);
    
    // Choose the required identifier via a proximity search
    shared_ptr<TNode> start_scope;
    if( cpp_scope_node )
    {
        // A C++ style scope was supplied (iscope::II) - search in here and don't recurse
        start_scope = Find( cpp_scope_node );
        ASSERT(start_scope);
        recurse = false; // Never recurse with a C++ scope - they are exact specifications       
    }    
    else
    {
        // No C++ scope, so use the current scope and recurse through parents
        ASSERT( !scope_stack.empty() );
        start_scope = scope_stack.top();
        ASSERT(start_scope);
    }

    TRACE("TryGet to find ")(to_find_II->getName())(" in scope ")(start_scope)(cpp_scope_node?" C++ ":" ")(recurse?"":"not ")("recursive\n"); 
    TRACE("tnodes=")(tnodes)("\n");    
    int best_distance=NOMATCH;
    shared_ptr<TNode> best_tnode;
    for( deque< shared_ptr<TNode> >::size_type i=0; i<tnodes.size(); i++ )
    {
        int distance = IsIdentifierMatch( to_find_II, start_scope, tnodes[i], recurse );
        if( distance != NOMATCH )
			TRACE(tnodes[i])(" matches with distance %d\n", distance);
        if( distance < best_distance )
        {
            best_distance = distance;
            best_tnode = tnodes[i];            
        }
    }
	TRACE("Result is ")(best_tnode)("\n");
 
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

shared_ptr<Node> IdentifierTracker::global;


string Trace( const IdentifierTracker::TNode &tnode )
{
    string s;
	if( tnode.node == IdentifierTracker::global )
	{
		s += "global:";
	}
	if( tnode.II )
	{
		s += "\"" + string(tnode.II->getName()) + "\":";
	}
    
    s += SSPrintf("S%p", tnode.cs);

    return s;
}
