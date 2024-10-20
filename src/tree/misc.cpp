#include "cpptree.hpp"
#include "helpers/walk.hpp"

#include "misc.hpp"

// Generate messages when can't find a declaration. Vida Nova should be
// OK with this, but it may be unexpected for some/all test cases. See #617
//#define WARN_UNFOUND_DECL

using namespace CPPTree;

AugTreePtr<Identifier> GetIdentifierOfDeclaration( AugTreePtr<Declaration> d )
{
    if( auto i = AugTreePtr<Instance>::DynamicCast( d ) )
        return GET_CHILD(i, identifier);
    else if( auto t = AugTreePtr<UserType>::DynamicCast( d ) )
        return GET_CHILD(t, identifier);
    else if( auto l = AugTreePtr<Label>::DynamicCast( d ) )
        return GET_CHILD(l, identifier);
    else
        return AugTreePtr<Identifier>(); // was a declaration without an identifier, ie a base class
}

	
AugTreePtr<Node> HasDeclaration::ApplyTransformation( const TreeKit &kit, AugTreePtr<Node> node ) const try
{    
    set<NavigationInterface::LinkInfo> declarer_infos = kit.utils->GetDeclarers( node );
    
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
    if( !kit.utils->IsRequireReports() )
        return AugTreePtr<Node>(declarer);
    
    // To be able to report the declarer as a node in the tree, we
    // must find its parent link
    set<NavigationInterface::LinkInfo> parent_infos = kit.utils->GetParents( declarer );
    const TreePtrInterface *declarer_parent_link = OnlyElementOf( parent_infos ).second;

    // Report and return
    return kit.utils->CreateAugTree<Node>(declarer_parent_link); 
}
catch( NavigationInterface::UnknownNode &) 
{
#ifdef WARN_UNFOUND_DECL
    FTRACE("Warning: declaration of ")(node)(" not found (UnknownNode)\n");
#endif        
    throw DeclarationNotFound();
}
    


HasDeclaration HasDeclaration::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
AugTreePtr<Record> GetRecordDeclaration( const TreeKit &kit, AugTreePtr<TypeIdentifier> id )
{
	AugTreePtr<Node> ut = HasDeclaration().ApplyTransformation( kit, id );
	while( auto td = AugTreePtr<Typedef>::DynamicCast(ut) )
	{
	    auto ti = AugTreePtr<TypeIdentifier>::DynamicCast( GET_CHILD(td, type) );
	    if(ti)
	        ut = HasDeclaration().ApplyTransformation( kit, ti );
	    else
	        return AugTreePtr<Record>(); // not a record
	}
	auto r = AugTreePtr<Record>::DynamicCast(ut);
	return r;
}


// Hunt through a record and its bases to find the named member (actually, render string)
AugTreePtr<Instance> FindMemberByName( const TreeKit &kit, AugTreePtr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    for( TreePtr<Declaration> &d : r->members )
    {
		AugTreePtr<Declaration> d_atp = r.GetChild(&d);
        if( auto i = AugTreePtr<Instance>::DynamicCast(d_atp) )
            if( auto sss = AugTreePtr<SpecificInstanceIdentifier>::DynamicCast(GET_CHILD(i, identifier)) )
                if( sss->GetRender() == name )
                    return i;
	}
                
    // Try recursing through the base classes, if there are any
    if( auto ir = AugTreePtr<InheritanceRecord>::DynamicCast( r ) )
    {
        for( TreePtr<Base> &b : ir->bases )
        {
			AugTreePtr<Base> b_atp = ir.GetChild(&b);
            AugTreePtr<Node> ut = HasDeclaration().ApplyTransformation( kit, GET_CHILD(b_atp, record) );
            auto ir = AugTreePtr<InheritanceRecord>::DynamicCast(ut);
            ASSERT(ir);
            if( AugTreePtr<Instance> i = FindMemberByName( kit, ir, name ) )
                return i;
        }
	}
    
    // We failed. Hang our head in shame.                
    return AugTreePtr<Instance>();
}                
