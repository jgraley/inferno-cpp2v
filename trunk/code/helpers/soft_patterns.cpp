#include "soft_patterns.hpp"

RootedSearchReplace::Result SoftExpressonOfType::DecidedCompare( const RootedSearchReplace *sr,
		                                                         shared_ptr<Node> x,
		                                                         RootedSearchReplace::CouplingKeys *keys,
		                                                         bool can_key,
		                                                         RootedSearchReplace::Conjecture &conj ) const
{
	if( shared_ptr<Expression> xe = dynamic_pointer_cast<Expression>(x) )
	{
	    // Find out the type of the candidate expression	
	    shared_ptr<Type> xt = TypeOf( sr->GetProgram() ).Get( xe );
	    TRACE("TypeOf(%s) is %s\n", TypeInfo(xe).name().c_str(), TypeInfo(xt).name().c_str() );
	    ASSERT(xt);
	    
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xt, shared_ptr<Node>(type_pattern), keys, can_key, conj );
	}
	else
	{
	    // not even an expression lol that aint going to match (means this node must be in a wider
		// context eg Node or Statement, and the tree contained something other than Expression - so
		// we're restricting to Expressions in addition to checking the type)
	    return RootedSearchReplace::NOT_FOUND;
	}        
}     


RootedSearchReplace::Result SoftIdentifierOfInstance::DecidedCompare( const RootedSearchReplace *sr,
		                                                              shared_ptr<Node> x,
		                                                              RootedSearchReplace::CouplingKeys *keys,
		                                                              bool can_key,
		                                                              RootedSearchReplace::Conjecture &conj ) const
{
	if( shared_ptr<InstanceIdentifier> xid = dynamic_pointer_cast<InstanceIdentifier>(x) )
	{
	    // Find out the type of the candidate expression	
	    shared_ptr<Instance> xi = GetDeclaration( sr->GetProgram(), xid );
	    TRACE("GetDeclaration(%s) is %s\n", TypeInfo(xid).name().c_str(), TypeInfo(xi).name().c_str() );
	    ASSERT(xi);
	    
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xi, shared_ptr<Node>(decl_pattern), keys, can_key, conj );
	}
	else
	{
	    // not even an instance identifier lol that aint going to match (means this node must be in a wider
		// context eg Node or Statement, and the tree contained something other than Expression - so
		// we're restricting to IsntanceIdentifiers in addition to checking the type)
	    return RootedSearchReplace::NOT_FOUND;
	}        
}     


shared_ptr<Node> SoftMakeIdentifier::DuplicateSubtree( const RootedSearchReplace *sr,
		                                               RootedSearchReplace::CouplingKeys *keys,
		                                               bool can_key )
{
	string newname;
	if( source )
	{
		TRACE("Begin SoftMakeIdentifier recurse\n");
		// We have a child identifier - let replace algorithm run in the expectation it will
		// get subsitituted with a SpecificIdentifier from the original program tree
	    shared_ptr<Node> n = sr->DuplicateSubtree( shared_ptr<Node>(source), keys, can_key );
		TRACE("End SoftMakeIdentifier recurse\n");
	    ASSERT( n );
	    shared_ptr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
	    ASSERT( si )("trying to make an identifier from ")(*n)(" which should be a kind of SpecificIdentifier");
	    // Use sprintf to build a new identifier based on the found one. Obviously %s
	    // becomes the old identifier's name.
	    newname = SSPrintf(format.c_str(), ((string)*si).c_str());
	}
	else
	{
		newname = format;
	}
	return shared_ptr<SpecificInstanceIdentifier>( new SpecificInstanceIdentifier( newname ) );
}
