#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../link.hpp"
#include "helpers/simple_compare.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "node/specialise_oostd.hpp"
#include "../agents/agent.hpp"

#include <unordered_set>
#include <functional>
  
/// SR namespace contains the search and replace implementation
namespace SR 
{
    
class Domain
{   
public:
	Domain();

    DBWalk::Action GetDeleteGeometricAction();
	DBWalk::Action GetInsertGeometricAction();
	
	static void CheckEqual( shared_ptr<Domain> l, shared_ptr<Domain> r );	
    
    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
};    
    
}

#endif
