/*
 * validate.cpp
 *
 *  Created on: 30 Dec 2009
 *      Author: jgraley
 */

#include "validate.hpp"
#include "typeof.hpp"
#include "misc.hpp"
#include "tree/tree.hpp"

void Validate::operator()( TreePtr<Node> context,
						   TreePtr<Node> *proot )
{
	(void)context;

	decl_refs.clear();
	total_refs.clear();

	Walk w( *proot );
	FOREACH( const TreePtr<Node> x, w )
	{
		if( !is_pattern ) // Don't do these checks on search/replace patterns
		{
			// NULL pointers not allowed in program tree (though they are allowed in search/replace patterns)
			ASSERT( x )("Found NULL pointer in tree at ")( w );

			// Intermediate nodes are only allowed in search and replace patterns; the trees for programs
			// must be built from final nodes.
			ASSERT( x->IsFinal() )( "Found intermediate (non-final) node ")(*x)(" at ")(w);

			// Check that we can successfully call TypeOf on every Expression
			if( TreePtr<Expression> e = dynamic_pointer_cast<Expression>(x) )
			    (void)TypeOf()(context, e);

			// Check that every identifier has a declaration
			if( TreePtr<InstanceIdentifier> ii = dynamic_pointer_cast<InstanceIdentifier>(x) )
			    (void)GetDeclaration()(context, ii);

			// if x is missing it's NODE_FUNCTIONS macro, then the Clone we get (y) will be a clone
			// of the most specialised base of x that does have NODE_FUNCTIONS.
			TreePtr<Node> y = dynamic_pointer_cast<Node>((*x).Clone());
			ASSERT( typeid(*y)==typeid(*x) )(*x)(" apparently does not contain NODE_FUNCTIONS macro because it Clone()s to ")(*y)(" at ")(w);
        }

		if( x )
		{
			vector< Itemiser::Element * > members = x->Itemise();
		    FOREACH( Itemiser::Element *m, members )
			{
				if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(m) )
				{
					FOREACH( const TreePtrInterface &tpi, *con )
						OnLink( x, tpi );
				}
				else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(m) )
				{
					OnLink( x, *ptr );
				}
				else
				{
					ASSERTFAIL("Got something from itemise that isn't a container or a shared pointer at ")(w);
				}
			}
		}
	}

	Walk w2( *proot );
	FOREACH( const TreePtr<Node> x, w2 )
	{
		if( x == *proot )
			continue; // Skip the proot node, since we don't know what references we expect it to have

		if( x )
		{
			// Check incoming pointers rule: Non-identifier nodes should be referenced exactly once
			// Identifiers should be referenced exactly once by the node that declares them,
			// and may be referenced zero or more times by other nodes. We skip the
			// identifier checks for patterns though (TODO decide what the rule becomes in this case)
			if( dynamic_pointer_cast<Identifier>(x) )
			{
				if( !is_pattern )
					ASSERT( decl_refs[x] == 1 )("Identifier ")(*x)(" found with %d declaration references", decl_refs[x])(" at ")(w2)
					      ("\nThere must be exactly 1 declaration and zero or more usages");
			}
			else
				ASSERT( total_refs[x] == 1 )("Node ")(*x)(" found with %d references", total_refs[x] )(" at ")(w2)
					  ("\nThere must be exactly 1 reference to nodes (except identifiers)");
		}
	}
}

void Validate::OnLink( TreePtr<Node> p, TreePtr<Node> c )
{
	if( TreePtr<Instance> pi = dynamic_pointer_cast<Instance>(p) )
	{
		if( c == pi->identifier )
		    decl_refs[c]++;
	}
	else if( TreePtr<UserType> pu = dynamic_pointer_cast<UserType>(p) )
	{
		if( c == pu->identifier )
		    decl_refs[c]++;
	}
	else if( TreePtr<Label> pl = dynamic_pointer_cast<Label>(p) )
	{
		if( c == pl->identifier )
		    decl_refs[c]++;
	}

	total_refs[c]++;
}


