#include "tree/tree.hpp"
#include "walk.hpp"
#include "misc.hpp"

shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d )
{
    if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
        return i->identifier;
    else if( shared_ptr<UserType> t = dynamic_pointer_cast<UserType>( d ) )
        return t->identifier;
    else if( shared_ptr<Label> l = dynamic_pointer_cast<Label>( d ) )
        return l->identifier;
    else
        ASSERTFAIL();
}

SharedPtr<Node> GetDeclaration::operator()( SharedPtr<Node> context, SharedPtr<Node> root )
{
	if( SharedPtr<TypeIdentifier> tid = dynamic_pointer_cast<TypeIdentifier>( root ) )
		return Get( context, tid );
	else if( SharedPtr<InstanceIdentifier> iid = dynamic_pointer_cast<InstanceIdentifier>( root ) )
		return Get( context, iid );
	else
		return SharedPtr<Node>();
}

SharedPtr<UserType> GetDeclaration::Get( SharedPtr<Node> context, SharedPtr<TypeIdentifier> id )
{
	Flattener<UserType> walkr(context);
	TRACE("GetDeclaration %d\n", walkr.size());
	ASSERT( (deque< SharedPtr<UserType> >::iterator)(walkr.begin()) !=
			(deque< SharedPtr<UserType> >::iterator)(walkr.end()) );
	FOREACH( SharedPtr<UserType> d, walkr )
	{
		TRACE("trying id of %d\n", walkr.size());
        if( id == GetIdentifier( d ) ) 
	        return d;
	}
	ASSERTFAIL();
}

SharedPtr<Instance> GetDeclaration::Get( SharedPtr<Node> context, SharedPtr<InstanceIdentifier> id )
{
    Walk w( context );
    while(!w.Done())
    {
        SharedPtr<Node> n = w.Get();

        if( SharedPtr<Instance> d = dynamic_pointer_cast<Instance>(n) )
            if( id == GetIdentifier( d ) )
	            return d;

	    w.AdvanceInto();
	}
	ASSERTFAIL("did not find instance declaration for identifier");
}

// Look for a record, skipping over typedefs. Returns NULL if not a record.
shared_ptr<Record> GetRecordDeclaration( shared_ptr<Node> context, shared_ptr<TypeIdentifier> id )
{
	shared_ptr<Node> ut = GetDeclaration()( context, id );
	while( shared_ptr<Typedef> td = dynamic_pointer_cast<Typedef>(ut) )
	{
	    shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = GetDeclaration()( context, ti);
	    else
	        return shared_ptr<Record>(); // not a record
	}
	shared_ptr<Record> r = dynamic_pointer_cast<Record>(ut);
	return r;
}


// Hunt through a record and its bases to find the named member
shared_ptr<Instance> FindMemberByName( shared_ptr<Program> program, shared_ptr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    FOREACH( shared_ptr<Declaration> d, r->members )
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(d) )
            if( shared_ptr<SpecificInstanceIdentifier> sss = dynamic_pointer_cast<SpecificInstanceIdentifier>(i->identifier) )
                if( (string)*sss == name )
                    return i;
                
    // Try recursing through the base classes, if there are any
    if( shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>( r ) )
        FOREACH( shared_ptr<Base> b, ir->bases )
        {
            shared_ptr<Node> ut = GetDeclaration()( program, b->record );
            shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( shared_ptr<Instance> i = FindMemberByName( program, ir, name ) )
                return i;
        }
                
    // We failed. Hang our head in shame.                
    return shared_ptr<Instance>();
}                

