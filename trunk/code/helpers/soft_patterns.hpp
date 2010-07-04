#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "tree/tree.hpp"
#include "transformation.hpp"



// Make an identifer based on an existing one. New identfier is named using
// sprintf( format, source->name )
// You must key the source identifier to somehting in the search pattern (so it
// will get substitued to the real Specific identifier found in the tree) and
// you may substitute the SoftMakeIdentifier in the replace pattern if you need
// to specify it in more than one place. Note that SoftMakeIdentifier is stateless
// and cannot therefore keep track of uniqueness - you'll get a new one each time
// and must rely on a replace coupling to get multiple reference to the same
// new identifier. Rule is: ONE of these per new identifier.
struct SoftMakeIdentifier : InstanceIdentifier, // TODO other kinds of identifier
                            RootedSearchReplace::SoftReplacePattern
{
	SoftMakeIdentifier( string s ) : format(s) {}
	SoftMakeIdentifier() : format("___UNNAMED___") {}
	NODE_FUNCTIONS
	string format;
	TreePtr<Identifier> source;
private:
    virtual TreePtr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
    		                                   CouplingKeys *keys,
    		                                   bool can_key );
};

#endif
