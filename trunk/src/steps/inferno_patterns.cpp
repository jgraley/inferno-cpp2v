#include "inferno_patterns.hpp"
#include "tree/cpptree.hpp"

using namespace CPPTree;

string BuildIdentifierBase::GetNewName()
{
    //INDENT;
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    vector<string> vs;
    bool all_same = true;
    FOREACH( TreePtr<Node> source, sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        TreePtr<Node> n = DoBuildReplace( TreePtr<Node>(source) );
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( n );
        TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
        ASSERT( si )("BuildIdentifier: ")(*n)(" should be a kind of SpecificIdentifier (format is %s)", format.c_str());
        string s = si->GetName();
        if( !vs.empty() )
            all_same = all_same && (s == vs.back());
        vs.push_back( s );
    }

    // Optional functionality: when every identifier has the same name, just return that
    // name. Handy for "merging" operations.
    if( (flags & BYPASS_WHEN_IDENTICAL) && all_same )
        return vs[0];  

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( vs.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), vs[0].c_str() );
        case 2:
            return SSPrintf( format.c_str(), vs[0].c_str(), vs[1].c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}

bool IdentifierByNameBase::IsMatch( const TreePtrInterface &x )
{
    string newname = name; 
    TreePtr<Node> nx = x; // TODO dynamic_pointer_cast support for TreePtrInterface
    if( TreePtr<CPPTree::SpecificIdentifier> si = dynamic_pointer_cast<CPPTree::SpecificIdentifier>(nx) )
    {
        TRACE("IsMatch comparing ")(si->GetName())(" with ")(newname);
        if( si->GetName() == newname )
        {
            TRACE(" : same\n");
            return true;
        }
        TRACE(" : different\n");
    }
    return false;
}


shared_ptr<Key> NestedBase::MyCompare( const TreePtrInterface &x )
{
    INDENT;
    string s;
    // Keep advancing until we get NULL, and remember the last non-null position
    TreePtr<Node> xt = x;
    int i = 0;
    while( TreePtr<Node> tt = Advance(xt, &s) )
    {
        xt = tt;
    } 
            
    // Compare the last position with the terminus pattern
    bool r = NormalCompare( xt, TreePtr<Node>(terminus) );
    
    // Compare the depth with the supplied pattern if present
    if( r && depth )
    {
        TreePtr<Node> cur_depth( new SpecificString(s) );
        r = NormalCompare( cur_depth, TreePtr<Node>(depth) );
    }
    
    if( r )
    {
        // Ensure the replace can terminate and overlay
        shared_ptr<TerminusKey> k( new TerminusKey );
        k->root = x;
        k->terminus = xt;
        return k;
    }
    else
    {
        return shared_ptr<Key>();
    }
}    


TreePtr<Node> NestedArray::Advance( TreePtr<Node> n, string *depth )
{
    if( TreePtr<Array> a = dynamic_pointer_cast<Array>(n) )                          
        return a->element;
    else
        return TreePtr<Node>();
}


TreePtr<Node> NestedSubscriptLookup::Advance( TreePtr<Node> n, string *depth )
{
    if( TreePtr<Subscript> s = dynamic_pointer_cast<Subscript>(n) )            
    {
        *depth += "S";
        return s->operands[0]; // the base, not the index
    }
    else if( TreePtr<Lookup> l  = dynamic_pointer_cast<Lookup>(n) )            
    {
        *depth += "L";
        return l->member; 
    }
    else
    {
        return TreePtr<Node>();
    }
}
