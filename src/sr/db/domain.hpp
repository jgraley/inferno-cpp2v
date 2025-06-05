#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../link.hpp"
#include "helpers/simple_compare.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "node/specialise_oostd.hpp"
#include "../agents/agent.hpp"
#include "tree_zone.hpp"

#include <unordered_set>
#include <functional>
  
/// SR namespace contains the search and replace implementation
namespace SR 
{
    
class Domain
{   
public:
    Domain();

	void InsertTree(TreeZone &zone);
	void DeleteTree(TreeZone &zone);

	void DeleteAction(const DBWalk::WalkInfo &walk_info);
	void InsertAction(const DBWalk::WalkInfo &walk_info);
    
    static void CheckEqual( shared_ptr<Domain> l, shared_ptr<Domain> r );    
    
    DBWalk db_walker;  

    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;          
};    
    
}

#endif
