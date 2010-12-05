#include "tree/tree.hpp"
#include "walk.hpp"
#include "misc.hpp"

TreePtr<Identifier> GetIdentifier( TreePtr<Declaration> d )
{
    if( TreePtr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
        return i->identifier;
    else if( TreePtr<UserType> t = dynamic_pointer_cast<UserType>( d ) )
        return t->identifier;
    else if( TreePtr<Label> l = dynamic_pointer_cast<Label>( d ) )
        return l->identifier;
    else
        return TreePtr<Identifier>(); // was a declaration without an identifier, ie a base class
}

TreePtr<Node> GetDeclaration::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	if( TreePtr<TypeIdentifier> tid = dynamic_pointer_cast<TypeIdentifier>( root ) )
		return Get( context, tid );
	else if( TreePtr<InstanceIdentifier> iid = dynamic_pointer_cast<InstanceIdentifier>( root ) )
		return Get( context, iid );
	else
		return TreePtr<Node>();
}

TreePtr<UserType> GetDeclaration::Get( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	Walk w(context);
	FOREACH( TreePtr<Node> n, w )
	{
        if( TreePtr<UserType> d = dynamic_pointer_cast<UserType>(n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
	ASSERTFAIL();
}

TreePtr<Instance> GetDeclaration::Get( TreePtr<Node> context, TreePtr<InstanceIdentifier> id )
{
	Walk w( context );
	FOREACH( TreePtr<Node> n, w )
	{
        if( TreePtr<Instance> d = dynamic_pointer_cast<Instance>(n) )
            if( id == GetIdentifier( d ) )
	            return d;
	}
	ASSERTFAIL("did not find instance declaration for identifier");
}

// Look for a record, skipping over typedefs. Returns NULL if not a record.
TreePtr<Record> GetRecordDeclaration( TreePtr<Node> context, TreePtr<TypeIdentifier> id )
{
	TreePtr<Node> ut = GetDeclaration()( context, id );
	while( TreePtr<Typedef> td = dynamic_pointer_cast<Typedef>(ut) )
	{
	    TreePtr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = GetDeclaration()( context, ti);
	    else
	        return TreePtr<Record>(); // not a record
	}
	TreePtr<Record> r = dynamic_pointer_cast<Record>(ut);
	return r;
}


// Hunt through a record and its bases to find the named member
TreePtr<Instance> FindMemberByName( TreePtr<Program> program, TreePtr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    FOREACH( TreePtr<Declaration> d, r->members )
        if( TreePtr<Instance> i = dynamic_pointer_cast<Instance>(d) )
            if( TreePtr<SpecificInstanceIdentifier> sss = dynamic_pointer_cast<SpecificInstanceIdentifier>(i->identifier) )
                if( (string)*sss == name )
                    return i;
                
    // Try recursing through the base classes, if there are any
    if( TreePtr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>( r ) )
        FOREACH( TreePtr<Base> b, ir->bases )
        {
            TreePtr<Node> ut = GetDeclaration()( program, b->record );
            TreePtr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( TreePtr<Instance> i = FindMemberByName( program, ir, name ) )
                return i;
        }
                
    // We failed. Hang our head in shame.                
    return TreePtr<Instance>();
}                


TreePtr<Identifier> GetIdentifierOfDeclaration( TreePtr<Declaration> d )
{
	if( TreePtr<Instance> di = dynamic_pointer_cast<Instance>(d) )
		return di->identifier;
	else if( TreePtr<UserType> dut = dynamic_pointer_cast<UserType>(d) )
		return dut->identifier;
	else if( TreePtr<Label> dl = dynamic_pointer_cast<Label>(d) )
		return dl->identifier;
	else
		return TreePtr<Identifier>(); // no identifier, maybe because d is a Base node
}

