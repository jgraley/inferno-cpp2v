#ifndef DOMAIN_EXTENSION_HPP
#define DOMAIN_EXTENSION_HPP

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
		struct Info
		{
			TreePtr<Node> induced_base_node;
			set<TreePtr<Node>> deps;
		};
		virtual Info GetDomainExtension( const XTreeDatabase *db, XLink xlink ) const = 0;
		virtual bool IsExtenderChannelLess( const Extender &r ) const = 0;
		virtual int GetExtenderChannelOrdinal() const = 0;
	};

	class ExtenderChannelRelation
	{
	public:
		/// Less operator: for use with set, map etc
		bool operator()( const Extender *l, const Extender *r ) const;		
	};
	
	typedef set<const Extender *, ExtenderChannelRelation> ExtenderSet;

	static ExtenderSet DetermineExtenders( const set<const SYM::Expression *> &sub_exprs );

	DomainExtension( const XTreeDatabase *db, ExtenderSet extenders );
		
	typedef function<void(XLink)> OnExtraTreeFunction;

	void SetOnExtraTreeFunctions( OnExtraTreeFunction on_insert_extra_tree,
                                   OnExtraTreeFunction on_delete_extra_tree = OnExtraTreeFunction() );

    // Gain access to a channel
    const DomainExtensionChannel *GetChannel( const Extender *extender ) const;

    // To be called after modifying the tree, and before any search/compare operation
    void InitialBuild();
    void PostUpdateActions();

    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
    void PrepareDeleteExtra(DBWalk::Actions &actions);
	void PrepareInsertExtra(DBWalk::Actions &actions);

    void TestRelations( const unordered_set<XLink> &xlinks );

private:
	// Map equivalence classes of extender agents onto our channel objects
	// so that we have one for each equaivalence class.
  	map<const Extender *, unique_ptr<DomainExtensionChannel>, ExtenderChannelRelation> channels;
};    
    
// ------------------------- DomainExtensionChannel --------------------------

// An instance for each equaivalence class of domain-extender agents, 
// covers whole domain for that extender algorithm.
class DomainExtensionChannel
{
public:	
   	DomainExtensionChannel( const XTreeDatabase *db, const DomainExtension::Extender *extender );

	void SetOnExtraTreeFunctions( DomainExtension::OnExtraTreeFunction on_insert_extra_tree,
                                  DomainExtension::OnExtraTreeFunction on_delete_extra_tree = DomainExtension::OnExtraTreeFunction() );

	XLink GetUniqueDomainExtension( XLink stimulus_xlink, TreePtr<Node> node ) const;
    void InsertExtraTree( TreePtr<Node> extra_root_node );
    void CheckStimulusXLink( XLink stimulus_xlink );
    void DropStimulusXLink( XLink stimulus_xlink );
    void Validate() const;
    void InitialBuild();
	void PostUpdateActions();

	void Insert(const DBWalk::WalkInfo &walk_info);
	void Delete(const DBWalk::WalkInfo &walk_info);
	void InsertExtra(const DBWalk::WalkInfo &walk_info);
	void DeleteExtra(const DBWalk::WalkInfo &walk_info);

private:
    const XTreeDatabase *db;
	const DomainExtension::Extender *extender;

    DomainExtension::OnExtraTreeFunction on_insert_extra_tree;
    DomainExtension::OnExtraTreeFunction on_delete_extra_tree;

    struct TrackingRow : Traceable
    {
        TrackingRow( TreePtr<Node> extra_node_, set<TreePtr<Node>> deps_ );
        string GetTrace() const override;
        
        TreePtr<Node> induced_base_node;
        set<TreePtr<Node>> deps;
    };
    
    struct ExtensionClass : Traceable
    {
        ExtensionClass( XLink induced_base_xlink_, int ref_count_ );
        string GetTrace() const override;
        
        XLink induced_base_xlink;
        int ref_count;
    };

    // One for each stimulus xlink, keeping track of the new node and dependencies
    map<XLink, TrackingRow> stimulus_to_induced_and_deps;
    
    // A reversal of stimulus_to_induced_and_deps for indexing on dependency
    map<TreePtr<Node>, set<XLink>> dep_to_all_stimulii;

    // Here we collect domain extension stimulus XLinks that we will re-create 
    // during PostUpdateActions() and then clear.
    set<XLink> stimulii_to_recheck;
    
    // SimpleCompare equivalence classes over the domain, with refcount = size of the class.
    map<TreePtr<Node>, ExtensionClass, SimpleCompare> extra_root_node_to_xlink_and_refcount;
};
        
}

#endif
