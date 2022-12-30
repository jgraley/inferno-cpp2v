/*
 * validate.cpp
 *
 *  Created on: 30 Dec 2009
 *      Author: jgraley
 */

#include "validate.hpp"
#include "hastype.hpp"
#include "misc.hpp"
#include "cpptree.hpp"

using namespace CPPTree;

void Validate::operator()( TreePtr<Node> root, 
                           TreePtr<Node> context )
{
	decl_refs.clear();
	total_refs.clear();

	// Is the root reachable from context? If not that's probably because we haven't inserted
	// the subtree at root into the program tree at context yet.
	bool connected = false;

	if( context )
    {
        // First walk over the entire context counting incoming links (because
        // incoming links from other than the subtree of interest still count
        // when validating link counts).
        Walk wcon( context, nullptr, nullptr );
        for( const TreePtrInterface &x : wcon )
        {
            if( x )
            {
                // TODO use UniqueWalk for this!
                vector< Itemiser::Element * > items = ((TreePtr<Node>)x)->Itemise();
                for( Itemiser::Element *item : items )
                {
                    if( ContainerInterface *con = dynamic_cast<ContainerInterface *>(item) )
                    {
                        for( const TreePtrInterface &tpi : *con )
                            OnLink( (TreePtr<Node>)x, (TreePtr<Node>)tpi );
                    }
                    else if( TreePtrInterface *singular = dynamic_cast<TreePtrInterface *>(item) )
                    {
                        OnLink( (TreePtr<Node>)x, (TreePtr<Node>)*singular );
                    }
                }
                if( (TreePtr<Node>)x == root )
                    connected = true;
            }
        }
    }
    
	// Now do the actual validation, only on the specified subtree
	Walk w( root, nullptr, nullptr );
	for( Walk::iterator wit = w.begin(); wit != w.end(); ++wit )
	{
		const auto x = (TreePtr<Node>)*wit;
		if( !is_pattern ) // Don't do these checks on search/replace patterns
		{
			// nullptr pointers not allowed in program tree (though they are allowed in search/replace patterns)
			ASSERT( x )("Found nullptr pointer in tree at ")( wit );

			// Intermediate nodes are only allowed in search and replace patterns; the trees for programs
			// must be built from final nodes.
            //TRACE("validating finality of ")(*x)(" as %d\n", (int)x->IsFinal() );
			ASSERT( x->IsFinal() )( "Found intermediate (non-final) node ")(*x)(" at ")(wit);

			// Check that we can successfully call HasType on every Expression
			if( context )
            {
                if(TreePtr<Expression> e = DynamicTreePtrCast<Expression>(x) )
                    (void)HasType::instance(e, context);

                // Check that every identifier has a declaration
                if( TreePtr<InstanceIdentifier> ii = DynamicTreePtrCast<InstanceIdentifier>(x) )
                    (void)HasDeclaration()(ii, context);
            }

			// if x is missing its NODE_FUNCTIONS macro, then the Clone we get (y) will be a clone
			// of the most specialised base of x that does have NODE_FUNCTIONS.
			TreePtr<Node> y( dynamic_pointer_cast<Node>((*x).Clone()) );
			ASSERT( y->GetName()==x->GetName() )(*x)(" apparently does not contain NODE_FUNCTIONS macro because it Clone()s to ")(*y)(" at ")(wit);
        }

		if( context && connected && x && x != context ) // Skip the root, since we won't have counted any refs to it
			                                 // Also, these rules may be broken for disconnected subtrees
		{
			//TRACE("decl_refs=%d total refs=%d\n", decl_refs[x], total_refs[x] );
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
	if( TreePtr<Instance> pi = DynamicTreePtrCast<Instance>(p) )
	{
		if( c == pi->identifier )
		    decl_refs[c]++;
	}
	else if( TreePtr<UserType> pu = DynamicTreePtrCast<UserType>(p) )
	{
		if( c == pu->identifier )
		    decl_refs[c]++;
	}
	else if( TreePtr<Label> pl = DynamicTreePtrCast<Label>(p) )
	{
		if( c == pl->identifier )
		    decl_refs[c]++;
	}

	total_refs[c]++;
}


