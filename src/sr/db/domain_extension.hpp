#ifndef DOMAIN_EXTENSION_HPP
#define DOMAIN_EXTENSION_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "helpers/simple_compare.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "node/specialise_oostd.hpp"
#include "../agents/agent.hpp"

#include <unordered_set>
#include <functional>

class SimpleCompare;


namespace SYM
{
    class Expression;
};
     
       
/// SR namespace contains the search and replace implementation
namespace SR 
{
class XTreeDatabase;
class DomainExtensionChannel;

// ------------------------- DomainExtension --------------------------

// Once instance in X tree database covers all domain and all agents
class DomainExtension
{   
public:
	class Extender : public virtual Traceable
	{
	public:
		virtual set<TreePtr<Node>> ExpandNormalDomain( const XTreeDatabase *db, const unordered_set<XLink> &xlinks ) const = 0;
		virtual bool IsExtenderLess( const Extender &r ) const = 0;
		virtual int GetExtenderOrdinal() const = 0;
	};

	class ExtenderClassRelation
	{
	public:
		/// Less operator: for use with set, map etc
		bool operator()( const Extender *l, const Extender *r ) const;		
	};
	
	typedef set<const Extender *, ExtenderClassRelation> ExtenderSet;

	static ExtenderSet DetermineExtenders( const set<const SYM::Expression *> &sub_exprs );

	DomainExtension( const XTreeDatabase *db, ExtenderSet extenders );
		
	typedef function<void(const TreeZone &)> OnExtraZoneFunction;

	void SetOnExtraXLinkFunctions( OnExtraZoneFunction on_insert_extra_zone,
                                   OnExtraZoneFunction on_delete_extra_zone = OnExtraZoneFunction() );

    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink GetUniqueDomainExtension( const Extender *extender, TreePtr<Node> node ) const; 
    
    // To be called after modifying the tree, and before any search/compare operation
    void InitialBuild();
    void Complete();

    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
    void PrepareDeleteExtra(DBWalk::Actions &actions);
	void PrepareInsertExtra(DBWalk::Actions &actions);

    void TestRelations( const unordered_set<XLink> &xlinks );

private:
	// Map equivalence classes of extender agents onto our channel objects
	// so that we have one for each equaivalence class.
  	map<const Extender *, unique_ptr<DomainExtensionChannel>, ExtenderClassRelation> channels;
};    
    
// ------------------------- DomainExtensionChannel --------------------------

// An instance for each equaivalence class of domain-extender agents, 
// covers whole domain for that extender algorithm.
class DomainExtensionChannel
{
public:	
   	DomainExtensionChannel( const XTreeDatabase *db, const DomainExtension::Extender *extender );

	void SetOnExtraXLinkFunctions( DomainExtension::OnExtraZoneFunction on_insert_extra_zone,
                                   DomainExtension::OnExtraZoneFunction on_delete_extra_zone = DomainExtension::OnExtraZoneFunction() );

	XLink GetUniqueDomainExtension( TreePtr<Node> node ) const;
    void ExtendDomainBaseXLink( TreePtr<Node> node );
    void ExtendDomain(const unordered_set<XLink> &new_domain );
	void InitialBuild();
	void Complete();

	void Insert(const DBWalk::WalkInfo &walk_info);
	void Delete(const DBWalk::WalkInfo &walk_info);
	void InsertExtra(const DBWalk::WalkInfo &walk_info);
	void DeleteExtra(const DBWalk::WalkInfo &walk_info);

private:
    const XTreeDatabase *db;
	const DomainExtension::Extender *extender;

    DomainExtension::OnExtraZoneFunction on_insert_extra_zone;
    DomainExtension::OnExtraZoneFunction on_delete_extra_zone;

    // SimpleCompare equivalence classes over the domain.
    map<TreePtr<Node>, XLink, SimpleCompare> domain_extension_classes;
};
    
    
}

#endif
