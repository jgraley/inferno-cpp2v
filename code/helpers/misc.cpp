#ifndef MISC_HPP
#define MISC_HPP

#include "tree/tree.hpp"
#include "walk.hpp"

shared_ptr<Identifier> GetIdentifier( shared_ptr<Declaration> d )
{
    if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
        return i->identifier;
    else if( shared_ptr<UserType> t = dynamic_pointer_cast<UserType>( d ) )
        return t->identifier;
    else if( shared_ptr<Label> l = dynamic_pointer_cast<Label>( d ) )
        return l->identifier;
    else
        ASSERT(0);
}

shared_ptr<UserType> GetDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id )
{
	Flattener<UserType> walkr(program);
	FOREACH( shared_ptr<UserType> d, walkr )
	{
        if( id == GetIdentifier( d ) ) 
	        return d;
	}
	ASSERT(0);
}

// Look for a record, skipping over typedefs. Returns NULL if not a record.
shared_ptr<Record> GetRecordDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id )
{
	shared_ptr<UserType> ut = GetDeclaration( program, id );
	while( shared_ptr<Typedef> td = dynamic_pointer_cast<Typedef>(ut) )
	{
	    shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = GetDeclaration(program, ti);
	    else
	        return shared_ptr<Record>(); // not a record
	}
	shared_ptr<Record> r = dynamic_pointer_cast<Record>(ut);
	return r;
}

shared_ptr<Instance> GetDeclaration( shared_ptr<Program> program, shared_ptr<InstanceIdentifier> id )
{
    Walk w( program );
    while(!w.Done())
    {
        shared_ptr<Node> n = w.Get();
        
        if( shared_ptr<Instance> d = dynamic_pointer_cast<Instance>(n) )
            if( id == GetIdentifier( d ) ) 
	            return d;

	    w.Advance();
	}
	ASSERT(!"did not find instance declaration for identifier");
}

// Hunt through a record and its bases to find the named member
shared_ptr<Instance> FindMemberByName( shared_ptr<Program> program, shared_ptr<Record> r, string name )
{
    TRACE("Record has %d members\n", r->members.size() );
    
    // Try the instance members (objects and functions) for a name match
    FOREACH( shared_ptr<Declaration> d, r->members )
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(d) )
            if( shared_ptr<String> sss = dynamic_pointer_cast<String>(i->identifier) )
                if( sss->value == name )
                    return i;
                
    // Try recursing through the base classes, if there are any
    if( shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>( r ) )
        FOREACH( shared_ptr<Base> b, ir->bases )
        {
            shared_ptr<UserType> ut = GetDeclaration( program, b->record );
            shared_ptr<InheritanceRecord> ir = dynamic_pointer_cast<InheritanceRecord>(ut);
            ASSERT(ir);
            if( shared_ptr<Instance> i = FindMemberByName( program, ir, name ) )
                return i;
        }
                
    // We failed. Hang our head in shame.                
    return shared_ptr<Instance>();
}                

#endif
