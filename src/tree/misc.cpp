#include "cpptree.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"

using namespace CPPTree;

TreePtr<Identifier> GetIdentifier( TreePtr<Declaration> d )
{
    if( TreePtr<Instance> i = TreePtrCast<Instance>( d ) )
        return i->identifier;
    else if( TreePtr<UserType> t = TreePtrCast<UserType>( d ) )
        return t->identifier;
    else if( TreePtr<Label> l = TreePtrCast<Label>( d ) )
        return l->identifier;
    else
        return TreePtr<Identifier>(); // was a declaration without an identifier, ie a base class
}

TreePtr<Node> GetDeclaration::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	if( TreePtr<TypeIdentifier> tid = TreePtrCast<TypeIdentifier>( root ) )
		return Get( context, tid );
	else if( TreePtr<InstanceIdentifier> iid = TreePtrCast<InstanceIdentifier>( root ) )
		return Get( context, iid );
	else
		return TreePtr<Node>();
}

TreePtr<UserType> GetDeclaration::Get( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	Walk w(context);
	FOREACH( const TreePtrInterface &n, w )
	{
        if( TreePtr<UserType> d = TreePtrCast<UserType>((TreePtr<Node>)n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
    
	throw TypeDeclarationNotFound();
}

TreePtr<Instance> GetDeclaration::Get( TreePtr<Node> context, TreePtr<InstanceIdentifier> id )
{
	Walk w( context );
	FOREACH( const TreePtrInterface &n, w )
	{
        if( TreePtr<Instance> d = TreePtrCast<Instance>((TreePtr<Node>)n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
	
	throw InstanceDeclarationNotFound();
}

GetDeclaration GetDeclaration::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)

// Look for a record, skipping over typedefs. Returns nullptr if not a record.
TreePtr<Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	TreePtr<Node> ut = GetDeclaration()( context, id );
	while( TreePtr<Typedef> td = TreePtrCast<Typedef>(ut) )
	{
	    TreePtr<TypeIdentifier> ti = TreePtrCast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = GetDeclaration()( context, ti);
	    else
	        return TreePtr<Record>(); // not a record
	}
	TreePtr<Record> r = TreePtrCast<Record>(ut);
	return r;
}


// Hunt through a record and its bases to find the named member (actually, render string)
TreePtr<Instance> FindMemberByName( TreePtr<Program> program, TreePtr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    FOREACH( TreePtr<Declaration> d, r->members )
        if( TreePtr<Instance> i = TreePtrCast<Instance>(d) )
            if( TreePtr<SpecificInstanceIdentifier> sss = TreePtrCast<SpecificInstanceIdentifier>(i->identifier) )
                if( sss->GetRender() == name )
                    return i;
                
    // Try recursing through the base classes, if there are any
    if( TreePtr<InheritanceRecord> ir = TreePtrCast<InheritanceRecord>( r ) )
        FOREACH( TreePtr<Base> b, ir->bases )
        {
            TreePtr<Node> ut = GetDeclaration()( program, b->record );
            TreePtr<InheritanceRecord> ir = TreePtrCast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( TreePtr<Instance> i = FindMemberByName( program, ir, name ) )
                return i;
        }
    
    // We failed. Hang our head in shame.                
    return TreePtr<Instance>();
}                


TreePtr<Identifier> GetIdentifierOfDeclaration( TreePtr<Declaration> d )
{
	if( TreePtr<Instance> di = TreePtrCast<Instance>(d) )
		return di->identifier;
	else if( TreePtr<UserType> dut = TreePtrCast<UserType>(d) )
		return dut->identifier;
	else if( TreePtr<Label> dl = TreePtrCast<Label>(d) )
		return dl->identifier;
	else
		return TreePtr<Identifier>(); // no identifier, maybe because d is a Base node
}

