/*
 * validate.cpp
 *
 *  Created on: 30 Dec 2009
 *      Author: jgraley
 */

#include "validate.hpp"
#include "typeof.hpp"
#include "misc.hpp"
#include "cpptree.hpp"

using namespace CPPTree;

void Validate::operator()( TreePtr<Node> context,
						   TreePtr<Node> *proot )
{
	(void)context;
	decl_refs.clear();
	total_refs.clear();

	// Is the proot reachable from context? If not that's probably because we haven't inserted
	// the subtree at proot into the program tree at context yet.
	bool connected = false;

	// First walk over the entire context counting incoming links (because
	// incoming links from other than the subtree of interest still count
	// when validating link counts).
	Walk wcon( context );
	FOREACH( const TreePtrInterface &x, wcon )
	{
		if( x )
		{
			// TODO use UniqueWalk for this!
			vector< Itemiser::Element * > members = ((TreePtr<Node>)x)->Itemise();
		    FOREACH( Itemiser::Element *m, members )
			{
				if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(m) )
				{
					FOREACH( const TreePtrInterface &tpi, *con )
						OnLink( (TreePtr<Node>)x, (TreePtr<Node>)tpi );
				}
				else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(m) )
				{
					OnLink( (TreePtr<Node>)x, (TreePtr<Node>)*ptr );
				}
			}
			if( (TreePtr<Node>)x == *proot )
				connected = true;
		}
	}

	// Now do the actual validation, only on the specified subtree
	Walk w( *proot );
	for( Walk::iterator wit = w.begin(); wit != w.end(); ++wit )
	{
		const auto x = (TreePtr<Node>)*wit;
		if( !is_pattern ) // Don't do these checks on search/replace patterns
		{
			// nullptr pointers not allowed in program tree (though they are allowed in search/replace patterns)
			ASSERT( x )("Found nullptr pointer in tree at ")( wit );

			// Intermediate nodes are only allowed in search and replace patterns; the trees for programs
			// must be built from final nodes.
            TRACE("validating finality of ")(*x)(" as %d\n", (int)x->IsFinal() );
			ASSERT( x->IsFinal() )( "Found intermediate (non-final) node ")(*x)(" at ")(wit);

			// Check that we can successfully call TypeOf on every Expression
			if( TreePtr<Expression> e = dynamic_pointer_cast<Expression>(x) )
			    (void)TypeOf::instance(context, e);

			// Check that every identifier has a declaration
			if( TreePtr<InstanceIdentifier> ii = dynamic_pointer_cast<InstanceIdentifier>(x) )
			    (void)GetDeclaration()(context, ii);

			// if x is missing it's NODE_FUNCTIONS macro, then the Clone we get (y) will be a clone
			// of the most specialised base of x that does have NODE_FUNCTIONS.
			TreePtr<Node> y = dynamic_pointer_cast<Node>((*x).Clone());
			ASSERT( typeid(*y)==typeid(*x) )(*x)(" apparently does not contain NODE_FUNCTIONS macro because it Clone()s to ")(*y)(" at ")(wit);
        }

		if( x && x != context && connected ) // Skip the root, since we won't have counted any refs to it
			                                 // Also, these rules may be broken for disconnected subtrees
		{
			TRACE("decl_refs=%d total refs=%d\n", decl_refs[x], total_refs[x] );
			// Check incoming pointers rule: Non-identifier nodes should be referenced exactly once
			// Identifiers should be referenced exactly once by the node that declares them,
			// and may be referenced zero or more times by other nodes. We skip the
			// identifier checks for patterns though (TODO decide what the rule becomes in this case)
/*			if( dynamic_pointer_cast<Identifier>(x) )
			{
				if( !is_pattern && connected )
					ASSERT( decl_refs[x] == 1 )("Identifier ")(*x)(" found with %d declaration references", decl_refs[x])(" at ")(wit)
					      ("\nThere must be exactly 1 declaration and zero or more usages");
			}
			else
				ASSERT( total_refs[x] == 1 )("Node ")(*x)(" found with %d references", total_refs[x] )(" at ")(wit)
					  ("\nThere must be exactly 1 reference to nodes (except identifiers)");*/
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


