#include "cpptree.hpp"
#include "helpers/walk.hpp"

#include "misc.hpp"

//#define WARN_UNFOUND_DECL

// Generate messages when can't find a declaration. Vida Nova should be
// OK with this, but it may be unexpected for some/all test cases. See #617
//#define WARN_UNFOUND_DECL

using namespace CPPTree;

AugTreePtr<Identifier> GetIdentifierOfDeclaration( AugTreePtr<Declaration> d )
{
	ASSERT(d);
    if( auto i = AugTreePtr<Instance>::DynamicCast( d ) )
        return GET_CHILD(i, identifier);
    else if( auto t = AugTreePtr<UserType>::DynamicCast( d ) )
        return GET_CHILD(t, identifier);
    else if( auto l = AugTreePtr<Label>::DynamicCast( d ) )
        return GET_CHILD(l, identifier);
    else
        return AugTreePtr<Identifier>(); // was a declaration without an identifier, ie a base class
}

    
AugTreePtr<Node> DeclarationOf::TryApplyTransformation( const TransKit &kit, AugTreePtr<Node> node ) const try
{    
    set<AugTreePtr<Node>> declarers = kit.utils->GetDeclarers( node );
    
    if( declarers.empty() )
    {
#ifdef WARN_UNFOUND_DECL
        FTRACE("Warning: declaration of ")(node)(" not found (has no declarer links)\n");
#endif        
        throw DeclarationNotFound();
    }
    
    if( declarers.size() > 1 )
    {
        FTRACE("Warning: declaration of ")(node)(" is ambiguous (has %u declarer links)\n", declarers.size());
        throw DeclarationNotFound();
    }
    
    // function decl/def are folded, so we expect only one declarer
    return SoloElementOf( declarers ); 
}
catch( TransUtilsInterface::UnknownNode &) 
{
#ifdef WARN_UNFOUND_DECL
    FTRACE("Warning: declaration of ")(node)(" not found (UnknownNode)\n");
#endif        
    throw DeclarationNotFound();
}
    


DeclarationOf DeclarationOf::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
AugTreePtr<Record> GetRecordDeclaration( const TransKit &kit, AugTreePtr<TypeIdentifier> id )
{
    AugTreePtr<Node> ut = DeclarationOf().TryApplyTransformation( kit, id );
    while( auto td = AugTreePtr<Typedef>::DynamicCast(ut) )
    {
        auto ti = AugTreePtr<TypeIdentifier>::DynamicCast( GET_CHILD(td, type) );
        if(ti)
            ut = DeclarationOf().TryApplyTransformation( kit, ti );
        else
            return AugTreePtr<Record>(); // not a record
    }
    auto r = AugTreePtr<Record>::DynamicCast(ut);
    return r;
}


// Hunt through a record and its bases to find the named member (actually, render string)
AugTreePtr<Instance> FindMemberByName( const TransKit &kit, AugTreePtr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    AugTreePtr<Instance> i;
    FOR_AUG_CONTAINER( r, members, [&](AugTreePtr<Declaration> d_atp)
    {
		i = AugTreePtr<Instance>::DynamicCast(d_atp);
        if( i )
            if( auto sss = AugTreePtr<SpecificInstanceIdentifier>::DynamicCast(GET_CHILD(i, identifier)) )
                if( sss->GetToken() == name )
                    LLBreak();
    } );
    if( i )
        return i;
                
    // Try recursing through the base classes, if there are any
    if( auto ir = AugTreePtr<InheritanceRecord>::DynamicCast( r ) )
    {
        FOR_AUG_CONTAINER( ir, bases, [&](AugTreePtr<Base> b_atp)
        {
            AugTreePtr<Node> ut = DeclarationOf().TryApplyTransformation( kit, GET_CHILD(b_atp, record) );
            auto ir = AugTreePtr<InheritanceRecord>::DynamicCast(ut);
            ASSERT(ir);
            if( AugTreePtr<Instance> i_in_base = FindMemberByName( kit, ir, name ) )
            {
				i = i_in_base;
                LLBreak();
			}
        } );
    }
    if( i )
        return i;
            
    // We failed. Hang our head in shame.                
    return AugTreePtr<Instance>();
}                
