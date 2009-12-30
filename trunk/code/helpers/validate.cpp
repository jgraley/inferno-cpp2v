/*
 * validate.cpp
 *
 *  Created on: 30 Dec 2009
 *      Author: jgraley
 */

#include "validate.hpp"
#include "tree/tree.hpp"

void Validate::operator()( shared_ptr<Node> context,
						   shared_ptr<Node> *proot )
{
	(void)context;

	decl_refs.clear();
	total_refs.clear();

	Walk w( *proot );
	while(!w.Done())
	{
		shared_ptr<Node> x = w.Get();

		if( !is_pattern ) // Don't do this check on search/replace patterns
		{
			// NULL pointers not allowed in program tree (though they are allowed in search/replace patterns)
			ASSERT( x )("Found NULL pointer in tree at ")( w );
		}

		if( x )
		{
			vector< Itemiser::Element * > members = x->Itemise();
			for( int i=0; i<members.size(); i++ )
			{
				if( GenericContainer *con = dynamic_cast<GenericContainer *>(members[i]) )
				{
					for( GenericContainer::iterator i=con->begin(); i!=con->end(); ++i )
						OnLink( x, *i );
				}
				else if( GenericSharedPtr *ptr = dynamic_cast<GenericSharedPtr *>(members[i]) )
				{
					OnLink( x, *ptr );
				}
				else
				{
					ASSERTFAIL("Got something from itemise that isn't a container or a shared pointer");
				}
			}
		}

		w.AdvanceInto();
	}

	Walk w2( *proot );
	if(!w2.Done())
		w2.AdvanceInto(); // Skip the proot node, since we don't know what references we expect it to have
	while(!w2.Done())
	{
		SharedPtr<Node> x = w2.Get();

		if( x )
		{
			if( dynamic_pointer_cast<Identifier>(x) )
			{
				if( !is_pattern )
					ASSERT( decl_refs[x] == 1 )("Identifier ")(*x)(" found with %d declaration references\n", decl_refs[x])
					      ("There must be exactly 1 declaration and zero or more usages");
			}
			else
				ASSERT( total_refs[x] == 1 )("Node ")(*x)(" found with %d references\n", total_refs[x] )
					  ("There must be exactly 1 reference to nodes (except identifiers)");
		}

		w2.AdvanceInto();
	}
}

void Validate::OnLink( shared_ptr<Node> p, shared_ptr<Node> c )
{
	if( shared_ptr<Instance> pi = dynamic_pointer_cast<Instance>(p) )
	{
		if( c == pi->identifier )
		    decl_refs[c]++;
	}
	else if( shared_ptr<UserType> pu = dynamic_pointer_cast<UserType>(p) )
	{
		if( c == pu->identifier )
		    decl_refs[c]++;
	}
	else if( shared_ptr<Label> pl = dynamic_pointer_cast<Label>(p) )
	{
		if( c == pl->identifier )
		    decl_refs[c]++;
	}

//	if( !(total_refs.IsExist(c)) )
	//	total_refs[c] = 0;

	total_refs[c]++;
}


