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

class DomainExtension
{   
public:
	class Extender : public virtual Traceable
	{
	public:
		virtual set<TreePtr<Node>> ExpandNormalDomain( const TreeKit &kit, const unordered_set<XLink> &xlinks ) const = 0;
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
    XLink GetUniqueDomainExtension( TreePtr<Node> node ) const; 
    
    void ExtendDomainBaseXLink( const TreeKit &kit, TreePtr<Node> node );
    void ExtendDomainPatternWalk( const TreeKit &kit );
    void ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink );

    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
    void PrepareDeleteExtra(DBWalk::Actions &actions);
	void PrepareInsertExtra(DBWalk::Actions &actions);

    void TestRelations( const unordered_set<XLink> &xlinks );
        
    // SimpleCompare equivalence classes over the domain.
    map<TreePtr<Node>, XLink, SimpleCompare> domain_extension_classes;

private:
    const XTreeDatabase *db;
  	set<unique_ptr<DomainExtensionChannel>> channels;
  	
    OnExtraZoneFunction on_insert_extra_zone;
    OnExtraZoneFunction on_delete_extra_zone;
};    
    
// ------------------------- DomainExtensionChannel --------------------------

class DomainExtensionChannel
{
public:	
   	DomainExtensionChannel( const XTreeDatabase *db, const DomainExtension::Extender *extender );

	void SetOnExtraXLinkFunctions( DomainExtension::OnExtraZoneFunction on_insert_extra_zone,
                                   DomainExtension::OnExtraZoneFunction on_delete_extra_zone = DomainExtension::OnExtraZoneFunction() );

//private:
    const XTreeDatabase *db;
	const DomainExtension::Extender *extender;

    DomainExtension::OnExtraZoneFunction on_insert_extra_zone;
    DomainExtension::OnExtraZoneFunction on_delete_extra_zone;
};
    
    
}

#endif
