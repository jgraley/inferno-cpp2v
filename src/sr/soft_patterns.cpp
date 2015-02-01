#include "soft_patterns.hpp"
#include "coupling.hpp"

using namespace SR;

shared_ptr<Key> TransformOfBase::MyCompare( const TreePtrInterface &x )
{
    INDENT;
    // Transform the candidate expression
    TreePtr<Node> xt = (*transformation)( *GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    bool r =Compare( xt, TreePtr<Node>(pattern) );
        if( r )
        {
            // If we have a match, make the output of the transformation be a terminus
            // for substitution during replace - if it is under the original node x
            // then the replace will resume overlaying at the correct place. If not, no
            // harm done since the replace won't see the terminus (there would be no
            // right place to overlay)
            shared_ptr<TerminusKey> k( new TerminusKey );
            k->root = x;
            k->terminus = xt;
            terminus = pattern; // TODO go through and replace pattern with terminus, and do not declare pattern in this class
            return k;
        }
        else
        {
            return shared_ptr<Key>();
        }
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was of the wrong
		// type, so just don't match
		// TODO no need for this, the pre-restriction will take care of wrong type. But maybe
		// want this for other invalid cases?
	    return shared_ptr<Key>();
	}
}

