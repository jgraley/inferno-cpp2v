#ifndef DOMAIN_EXTENSION_HPP
#define DOMAIN_EXTENSION_HPP

#include "../up/zone.hpp"
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
		virtual TreePtr<Node> GetDomainExtraNode( const XTreeDatabase *db, XLink xlink, set<TreePtr<Node>> &deps ) const = 0;
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
		
	typedef function<void(XLink)> OnExtraSubtreeFunction;

	void SetOnExtraXLinkFunctions( OnExtraSubtreeFunction on_insert_extra_subtree,
                                   OnExtraSubtreeFunction on_delete_extra_zone = OnExtraSubtreeFunction() );

    // Gain access to a channel
    const DomainExtensionChannel *GetChannel( const Extender *extender ) const;

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

	void SetOnExtraXLinkFunctions( DomainExtension::OnExtraSubtreeFunction on_insert_extra_subtree,
                                   DomainExtension::OnExtraSubtreeFunction on_delete_extra_zone = DomainExtension::OnExtraSubtreeFunction() );

	XLink GetUniqueDomainExtension( XLink start_xlink, TreePtr<Node> node ) const;
    void AddExtraNode( TreePtr<Node> node );
    void TryAddStartXLink( XLink start_xlink );
    void DropStartXlink( XLink start_xlink );
    void Validate() const;
    void InitialBuild();
	void Complete();

	void Insert(const DBWalk::WalkInfo &walk_info);
	void Delete(const DBWalk::WalkInfo &walk_info);
	void InsertExtra(const DBWalk::WalkInfo &walk_info);
	void DeleteExtra(const DBWalk::WalkInfo &walk_info);

private:
    const XTreeDatabase *db;
	const DomainExtension::Extender *extender;

    DomainExtension::OnExtraSubtreeFunction on_insert_extra_subtree;
    DomainExtension::OnExtraSubtreeFunction on_delete_extra_zone;

    struct TrackingRow : Traceable
    {
        TrackingRow( TreePtr<Node> extra_node_, set<TreePtr<Node>> deps_ );
        string GetTrace() const override;
        
        TreePtr<Node> extra_node;
        set<TreePtr<Node>> deps;
    };
    
    struct ExtClass : Traceable
    {
        ExtClass( XLink extra_xlink_, int count_ );
        string GetTrace() const override;
        
        XLink extra_xlink;
        int count;
    };

    // One for each start xlink, keeping track of the new node and dependencies
    map<XLink, TrackingRow> start_to_tracking;
    
    // A reversal of start_to_tracking for indexing on dependency
    map<TreePtr<Node>, set<XLink>> dep_to_starts;

    // Here we collect domain extension start XLinks that we will re-create 
    // during Complete() and then clear.
    set<XLink> starts_to_redo;
    
    // SimpleCompare equivalence classes over the domain, with refcount = size of the class.
    map<TreePtr<Node>, ExtClass, SimpleCompare> domain_extension_classes;
};
        
}

#endif
