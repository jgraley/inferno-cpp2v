#ifndef COUPLING_HPP
#define COUPLING_HPP

#include "common/common.hpp"
#include "node/node.hpp"
#include "conjecture.hpp"
#include <set> 

namespace SR 
{

// Base class for coupling keys; this deals with individual node matches, and also with stars
// by means of pointing "root" at a SubCollection or SubSequence
class CompareReplace;
struct Key
{
	virtual ~Key(){}  // be a virtual hierarchy
	TreePtr<Node> root; // Input program node for this coupling
	TreePtr<Node> replace_pattern; // Pattern node for this coupling TODO rename to just pattern
	Conjecture::Choice *governing_choice;
	int governing_offset;
};

class CouplingKeys
{
// TODO const-correctness, so master can be const. Means using find() etc instead of []
// when we only want to look - this will make things faster too I suspect
public:
	CouplingKeys();
	/// Key a node to a pattern (generates a default key structure)
    void DoKey( TreePtr<Node> x, TreePtr<Node> pattern, Conjecture::Choice *gc=NULL, int go=0 );
    /// Key some key to a pattern - key is supplied by user, can be subclass of Key
    void DoKey( shared_ptr<Key> key, TreePtr<Node> pattern, Conjecture::Choice *gc=NULL, int go=0 );
    /// Get the node to which a pattern was keyed, or NULL if pattern has not been keyed
    TreePtr<Node> GetCoupled( TreePtr<Node> pattern );									
    /// Get the key for a given pattern, or NULL if pattern has not been keyed
    shared_ptr<Key> GetKey( TreePtr<Node> pattern );	
    /// Get all the keys in set form
    Set< TreePtr<Node> > GetAllKeys();		
    /// Provide a pointer to another (read-only) instance of this class that will 
    /// be merged for the purposes of searching, but will not receive new couplings 
    void SetMaster( CouplingKeys *m );				
    /// Clear the couplings
    void Clear();
private:
	Map< TreePtr<Node>, shared_ptr<Key> > keys_map;
	CouplingKeys *master; 
};

};
#endif
