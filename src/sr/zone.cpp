#include "zone.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- FreeZone --------------------------

FreeZone::FreeZone( TreePtr<Node> base_ ) :
    base( base_ )
{
    ASSERT( base ); // FreeZone is not nullable
}


TreePtr<Node> FreeZone::GetBase() const
{
    return base;
}


string FreeZone::GetTrace() const
{
    list<string> elts;
    for( const XLink &p : terminii )
        elts.push_back( Trace(p) );

    string arrow;
    if( terminii.empty() )
        arrow = " →"; // Indicates the subtree goes all the way to leaves
    else
        arrow = " ⇥ "; // Indicates the subtree terminates
    
    // TODO show empty zone using ↯
    
    return "FreeZone(" + Trace(base) + arrow + Join(elts, ", ") +")";
}


// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateFromExclusions( XLink base_xlink, const unordered_set<XLink> &exclusions )
{
    TreeZone zone( base_xlink );
    zone.CreateFromExclusionsWalker( base_xlink, exclusions );
    return zone;
}


TreeZone::TreeZone( XLink base_ ) :
    base( base_ )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildX() ); // Cannot be empty
}


TreeZone::TreeZone( XLink base, const FreeZone &free_zone ) :
    TreeZone( base )
{
    ASSERT( base.GetChildX() == free_zone.GetBase() )
          ( "Making TreeZone from FreeZone but bases do not match:\n")
          (base)("\n")
          (free_zone.GetBase())("\n");
    // TODO copy terminii out of free_zone
}    


XLink TreeZone::GetBase() const
{
    return base;
}


set<XLink> TreeZone::GetTerminii() const
{
    return terminii;
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && OnlyElementOf(terminii)==base;
}


void TreeZone::CreateFromExclusionsWalker( XLink xlink, const unordered_set<XLink> &exclusions )
{
    if( exclusions.count(xlink) > 0 )
    {
        terminii.insert( xlink ); // Exclusive terminus!
        // Don't look past this new terminus
        return;
    }

    // Recurse through chidren of node
    TreePtr<Node> node = xlink.GetChildX();
    FlattenNode flat( node );
    for(const TreePtrInterface &child_node : flat )
    {
        SR::XLink child_xlink( node, &child_node );
        CreateFromExclusionsWalker( child_xlink, exclusions );
    }
}
    

string TreeZone::GetTrace() const
{
    list<string> elts;
    for( const XLink &p : terminii )
        elts.push_back( Trace(p) );

    string arrow;
    if( terminii.empty() )
        arrow = " →"; // Indicates the subtree goes all the way to leaves
    else
        arrow = " ⇥ "; // Indicates the subtree terminates
        
    string rhs = arrow + Join(elts, ", ");
    if( IsEmpty() )
        rhs = " ↯"; // Indicates zone is empty due to a terminus at base
                    // (we still give the base, for info)
    
    return "TreeZone(" + Trace(base) + rhs +")";
}
