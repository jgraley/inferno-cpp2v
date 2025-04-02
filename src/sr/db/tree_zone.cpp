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

// TODO 
// free_zone arg should be non-const by-ref and not returned!!
// Turn the new FZ terminii into placeholders with a new DetachChild() etc
//    BUT we shouldn't need to : exchanging PARENT should leave the CHILDren
//    unchanged: FZ terminus child is NULL on the way in and should still 
//    be NULL on the way out.
// Rename Mutate() to MutateChild()
// Have it return the old base TreePtr<Node>
// Avoid dodgy language by saying ExchangeChild() etc

FreeZone MutableTreeZone::Exchange( FreeZone &&free_zone )
{    
	ASSERT( !free_zone.IsEmpty() ); // Could add support but apparently don't need it rn
	        
    if( IsEmpty() )
        return FreeZone::CreateEmpty();
            
    // Do a co-walk and populate one at a time. Update our terminii as we go. Cannot use 
    // TPI because we need to change the PARENT node, so we need a whole new XLink. Do this
    // under a validity check that all our XLinks are inside the new tree.
    // This is needed because MainTreeExchange() will pass us to MainTreeInsertGeometric()
    FreeZone::TerminusIterator free_mut_it = free_zone.GetTerminiiBegin();    
    vector<shared_ptr<Mutator>>::iterator tree_mut_it = terminii_mutators.begin();
    for( XLink &tree_terminus_xlink : terminii )
    {
        ASSERT( free_mut_it != free_zone.GetTerminiiEnd() ); // length mismatch    
        ASSERT( tree_mut_it != terminii_mutators.end() ); // length mismatch    
                
        // Mutate the FZ terminus match the TZ terminus
        TreePtr<Node> tz_boundary_node = (*tree_mut_it)->GetXLink().GetChildTreePtr(); // outside the zone        
        ASSERT( !dynamic_cast<ContainerInterface *>(tz_boundary_node.get()) ); // requirement for GetTreePtrInterface()
        (*free_mut_it)->Mutate( tz_boundary_node );    
        
        // Update the tree zone terminus
        swap( *tree_mut_it, *free_mut_it );		

        tree_terminus_xlink = (*tree_mut_it)->GetXLink();   
        
        
        free_mut_it++;
        tree_mut_it++;
    } 
    ASSERT( free_mut_it == free_zone.GetTerminiiEnd() ); // length mismatch  
    ASSERT( tree_mut_it == terminii_mutators.end() ); // length mismatch    

    // Deal with the bases
    auto temp2 = base.GetChildTreePtr();    
    base_mutator->Mutate( free_zone.GetBaseNode() );	
	free_zone.SetBase( temp2 );

    return free_zone;
}



