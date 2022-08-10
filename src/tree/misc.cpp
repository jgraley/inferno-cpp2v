#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"

using namespace CPPTree;

TreePtr<Identifier> GetIdentifier( TreePtr<Declaration> d )
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

TreePtr<Node> HasDeclaration::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	if( TreePtr<TypeIdentifier> tid = DynamicTreePtrCast<TypeIdentifier>( root ) )
		return Get( context, tid );
	else if( TreePtr<InstanceIdentifier> iid = DynamicTreePtrCast<InstanceIdentifier>( root ) )
		return Get( context, iid );
	else
		return TreePtr<Node>();
}

TreePtr<UserType> HasDeclaration::Get( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	Walk w(context, nullptr, nullptr);
	for( const TreePtrInterface &n : w )
	{
        if( TreePtr<UserType> d = DynamicTreePtrCast<UserType>((TreePtr<Node>)n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
    
	throw TypeDeclarationNotFound();
}

TreePtr<Instance> HasDeclaration::Get( TreePtr<Node> context, TreePtr<InstanceIdentifier> id )
{
	Walk w( context, nullptr, nullptr );
	for( const TreePtrInterface &n : w )
	{
        if( TreePtr<Instance> d = DynamicTreePtrCast<Instance>((TreePtr<Node>)n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
	
	throw InstanceDeclarationNotFound();
}

HasDeclaration HasDeclaration::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
TreePtr<Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	TreePtr<Node> ut = HasDeclaration()( context, id );
	while( TreePtr<Typedef> td = DynamicTreePtrCast<Typedef>(ut) )
	{
	    TreePtr<TypeIdentifier> ti = DynamicTreePtrCast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = HasDeclaration()( context, ti);
	    else
	        return TreePtr<Record>(); // not a record
	}
	TreePtr<Record> r = DynamicTreePtrCast<Record>(ut);
	return r;
}


// Hunt through a record and its bases to find the named member (actually, render string)
TreePtr<Instance> FindMemberByName( TreePtr<Program> program, TreePtr<Record> r, string name )
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
            TreePtr<Node> ut = HasDeclaration()( program, b->record );
            TreePtr<InheritanceRecord> ir = DynamicTreePtrCast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( TreePtr<Instance> i = FindMemberByName( program, ir, name ) )
                return i;
        }
    
    // We failed. Hang our head in shame.                
    return TreePtr<Instance>();
}                


TreePtr<Identifier> GetIdentifierOfDeclaration( TreePtr<Declaration> d )
{
	if( TreePtr<Instance> di = DynamicTreePtrCast<Instance>(d) )
		return di->identifier;
	else if( TreePtr<UserType> dut = DynamicTreePtrCast<UserType>(d) )
		return dut->identifier;
	else if( TreePtr<Label> dl = DynamicTreePtrCast<Label>(d) )
		return dl->identifier;
	else
		return TreePtr<Identifier>(); // no identifier, maybe because d is a Base node
}

