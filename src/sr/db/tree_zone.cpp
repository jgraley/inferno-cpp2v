#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;

// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateSubtree( XLink base )
{
    return TreeZone( base, {} );
}


TreeZone TreeZone::CreateEmpty( XLink base )
{
    ASSERTS( base );
    return TreeZone( base, { base } ); // One element, same as base
}


TreeZone::TreeZone( XLink base_, vector<XLink> terminii_ ) :
    base( base_ ),
    terminii( terminii_ )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildTreePtr() ); // Cannot be empty
}


TreeZone::TreeZone( const TreeZone &other ) :
    base( other.base ),
    terminii( other.terminii )
{
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && OnlyElementOf(terminii)==base;
}


int TreeZone::GetNumTerminii() const
{
    return terminii.size();
}


TreePtr<Node> TreeZone::GetBaseNode() const
{
    return base.GetChildTreePtr();
}


XLink TreeZone::GetBaseXLink() const
{
    return base;
}


XLink &TreeZone::GetBaseXLink()
{
    return base;
}


vector<XLink> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


FreeZone TreeZone::Duplicate() const
{
    if( IsEmpty() )
        return FreeZone::CreateEmpty();
        
    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Mutator>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    list<shared_ptr<Mutator>> free_zone_terminii;
    for( XLink terminus_upd : GetTerminusXLinks() )
    {
        ASSERTS( duplicator_terminus_map[terminus_upd].mutator );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].mutator );
    }

    // Create a new zone for the result.
    return FreeZone( new_base_x, move(free_zone_terminii) );
}


FreeZone TreeZone::MakeFreeZone(const XTreeDatabase *db) const
{    
    if( IsEmpty() )
        return FreeZone::CreateEmpty();
        
    FreeZone free_zone( base.GetChildTreePtr(), {} );
    for( XLink terminus : terminii )
        free_zone.AddTerminus( db->GetMutator( terminus ) );
    
    return free_zone;
}


TreeZone::TerminusIterator TreeZone::GetTerminiiBegin() const
{
    return terminii.begin();
}


TreeZone::TerminusIterator TreeZone::GetTerminiiEnd() const
{
    return terminii.end();
}



string TreeZone::GetTrace() const
{
    string s;
    if( IsEmpty() )
    {
        s = " ↯ "; // Indicates zone is empty due to a terminus at base
                   // (we still give the base, for info)
    }
    else
    {
        s = Trace(base);
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "TreeZone(" + s +")";
}


void TreeZone::DBCheck(const XTreeDatabase *db) const // TODO maybe move to database?
{
    ASSERT( db->HasRow( base ) )(base);
    
    if( IsEmpty() )
        return; // We've checked enough for empty zones
    // Now we've excluded legit empty zones, checks can be strict
    
    DepthFirstRelation dfr( db );
    XLink prev_xlink = base;
    for( XLink terminus_xlink : terminii )
    {
        ASSERT( db->HasRow( terminus_xlink ) )(terminus_xlink);
        
        ASSERT( dfr.Compare3Way( prev_xlink, terminus_xlink ) < 0 ); // strict: no repeated XLinks
            
        prev_xlink = terminus_xlink;
    }    
}

// ------------------------- MutableTreeZone --------------------------

MutableTreeZone::MutableTreeZone( TreeZone tz, 
                                  unique_ptr<Mutator> &&base_mutator_, 
                                  vector<shared_ptr<Mutator>> &&terminii_mutators_ ) :
    TreeZone(tz),
    base_mutator(move(base_mutator_)),
    terminii_mutators(move(terminii_mutators_))
{
}

FreeZone MutableTreeZone::Exchange( FreeZone &&new_free_zone )
{    
    FreeZone old_free_zone;
        
    if( IsEmpty() )
        return old_free_zone = FreeZone::CreateEmpty();
        
    old_free_zone = FreeZone( base.GetChildTreePtr(), {} );
    for( shared_ptr<Mutator> tmut : terminii_mutators )
        old_free_zone.AddTerminus( tmut );
    terminii_mutators.clear();
    
    //FreeZone displaced_free_zone( base, {} );
    bool new_zone_is_empty = new_free_zone.IsEmpty();
    
    // Do a co-walk and populate one at a time. Update our terminii as we go. Cannot use 
    // TPI because we need to change the PARENT node, so we need a whole new XLink. Do this
    // under a validity check that all our XLinks are inside the new tree.
    // This is needed because MainTreeExchange() will pass us to MainTreeInsertGeometric()
    FreeZone::TerminusIterator new_it = new_free_zone.GetTerminiiBegin();    
    for( XLink &tree_terminus_xlink : terminii )
    {
        ASSERT( new_it != new_free_zone.GetTerminiiEnd() ); // length mismatch    
        
        // Make the FZ terminus match the TZ terminus
        shared_ptr<Mutator> new_mutator( *new_it );        
        TreePtr<Node> boundary_node = tree_terminus_xlink.GetChildTreePtr(); // outside the zone        
        ASSERT( !dynamic_cast<ContainerInterface *>(boundary_node.get()) ); // requirement for GetTreePtrInterface()
        new_it = new_free_zone.PopulateTerminus( new_it, boundary_node ); // Note: ensures free zone not empty    
        
        // Update the tree zone terminus
        if( !new_zone_is_empty )        
        {
            tree_terminus_xlink = new_mutator->GetXLink();   
            terminii_mutators.push_back( new_mutator );
		}
    } 
    ASSERT( new_it == new_free_zone.GetTerminiiEnd() ); // length mismatch  
    ASSERT( new_free_zone.GetNumTerminii() == 0 ); // PopulateTerminus() consumed them all  

    // Do this after, since free zone is now not empty 
    base_mutator->Mutate( new_free_zone.GetBaseNode() );

    if( new_zone_is_empty )
    {
        // Become an empty tree zone
        terminii = { base };
    }
    
    return old_free_zone;
}



