#include "cpptree.hpp"
#include "helpers/walk.hpp"

#include "misc.hpp"

// Generate messages when can't find a declaration. Vida Nova should be
// OK with this, but it may be unexpected for some/all test cases. See #617
//#define WARN_UNFOUND_DECL

using namespace CPPTree;

TreePtr<Identifier> GetIdentifierOfDeclaration( TreePtr<Declaration> d )
{
    if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>( d ) )
        return i->identifier;
    else if( TreePtr<UserType> t = DynamicTreePtrCast<UserType>( d ) )
        return t->identifier;
    else if( TreePtr<Label> l = DynamicTreePtrCast<Label>( d ) )
        return l->identifier;
    else
        return TreePtr<Identifier>(); // was a declaration without an identifier, ie a base class
}

	
AugTreePtr<Node> HasDeclaration::ApplyTransformation( const TreeKit &kit, TreePtr<Node> node ) try
{    
    set<NavigationUtils::LinkInfo> declarer_infos = kit.nav->GetDeclarers( node );
    
    if( declarer_infos.empty() )
    {
#ifdef WARN_UNFOUND_DECL
        FTRACE("Warning: declaration of ")(node)(" not found (has no declarer links)\n");
#endif        
        throw DeclarationNotFound();
    }
    
    // function decl/def are folded, so we expect only one declarer
    TreePtr<Node> declarer = OnlyElementOf( declarer_infos ).first;
    
    // If we don't require reports, just return the node and we're done
    if( !kit.nav->IsRequireReports() )
        return AugTreePtr<Node>(declarer);
    
    // To be able to report the declarer as a node in the tree, we
    // must find its parent link
    set<NavigationUtils::LinkInfo> parent_infos = kit.nav->GetParents( declarer );
    const TreePtrInterface *declarer_parent_link = OnlyElementOf( parent_infos ).second;

    // Report and return
    return AugTreePtr<Node>(declarer, declarer_parent_link, kit.dep_rep); 
}
catch( NavigationUtils::UnknownNode &) 
{
#ifdef WARN_UNFOUND_DECL
    FTRACE("Warning: declaration of ")(node)(" not found (UnknownNode)\n");
#endif        
    throw DeclarationNotFound();
}
    


HasDeclaration HasDeclaration::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
TreePtr<Record> GetRecordDeclaration( const TreeKit &kit, TreePtr<TypeIdentifier> id )
{
	TreePtr<Node> ut = HasDeclaration().ApplyTransformation( kit, id );
	while( TreePtr<Typedef> td = DynamicTreePtrCast<Typedef>(ut) )
	{
	    TreePtr<TypeIdentifier> ti = DynamicTreePtrCast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = HasDeclaration().ApplyTransformation( kit, ti );
	    else
	        return TreePtr<Record>(); // not a record
	}
	TreePtr<Record> r = DynamicTreePtrCast<Record>(ut);
	return r;
}


// Hunt through a record and its bases to find the named member (actually, render string)
TreePtr<Instance> FindMemberByName( const TreeKit &kit, TreePtr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    for( TreePtr<Declaration> d : r->members )
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>(d) )
            if( TreePtr<SpecificInstanceIdentifier> sss = DynamicTreePtrCast<SpecificInstanceIdentifier>(i->identifier) )
                if( sss->GetRender() == name )
                    return i;
                
    // Try recursing through the base classes, if there are any
    if( TreePtr<InheritanceRecord> ir = DynamicTreePtrCast<InheritanceRecord>( r ) )
        for( TreePtr<Base> b : ir->bases )
        {
            TreePtr<Node> ut = HasDeclaration().ApplyTransformation( kit, b->record );
            TreePtr<InheritanceRecord> ir = DynamicTreePtrCast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( TreePtr<Instance> i = FindMemberByName( kit, ir, name ) )
                return i;
        }
    
    // We failed. Hang our head in shame.                
    return TreePtr<Instance>();
}                
