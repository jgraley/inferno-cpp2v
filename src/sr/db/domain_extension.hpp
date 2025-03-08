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
			set<XLink> deps;
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
		
	typedef function<DBCommon::TreeOrdinal(TreePtr<Node>)> CreateExtraTreeFunction;
	typedef function<void(DBCommon::TreeOrdinal)> DestroyExtraTreeFunction;

	void SetOnExtraTreeFunctions( CreateExtraTreeFunction create_extra_tree,
                                  DestroyExtraTreeFunction destroy_extra_tree );

    // Gain access to a channel
    const DomainExtensionChannel *GetChannel( const Extender *extender ) const;

    // To be called after modifying the tree, and before any search/compare operation
    void InitialBuild();
    void PostUpdateActions();

    DBWalk::Action GetDeleteAction();
	DBWalk::Action GetInsertAction();
	
    void CheckRelations( const unordered_set<XLink> &xlinks );

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

	void SetOnExtraTreeFunctions( DomainExtension::CreateExtraTreeFunction create_extra_tree,
                                  DomainExtension::DestroyExtraTreeFunction destroy_extra_tree );
	XLink GetUniqueDomainExtension( XLink stimulus_xlink, TreePtr<Node> node ) const;
    void CreateExtraTree( TreePtr<Node> extra_root_node );
    void CheckStimulusXLink( XLink stimulus_xlink );
    void DropStimulusXLink( XLink stimulus_xlink );
    void Validate() const;
    void InitialBuild();
	void PostUpdateActions();

	void Insert(const DBWalk::WalkInfo &walk_info);
	void Delete(const DBWalk::WalkInfo &walk_info);

private:
    const XTreeDatabase *db;
	const DomainExtension::Extender *extender;

    DomainExtension::CreateExtraTreeFunction create_extra_tree;
    DomainExtension::DestroyExtraTreeFunction destroy_extra_tree;

    struct InducedAndDeps : Traceable
    {
        InducedAndDeps( TreePtr<Node> induced_base_node_, set<XLink> deps_ );
        string GetTrace() const override;
        
        TreePtr<Node> induced_base_node;
        set<XLink> deps;
    };
    
    struct ExtensionClass : Traceable
    {
        ExtensionClass( DBCommon::TreeOrdinal tree_ordinal_, int ref_count_ );
        string GetTrace() const override;
        
        DBCommon::TreeOrdinal tree_ordinal;
        int ref_count;
    };

    // One for each stimulus xlink, keeping track of the new node and dependencies
    map<XLink, InducedAndDeps> stimulus_to_induced_and_deps;
    
    // A reversal of stimulus_to_induced_and_deps for indexing on dependency
    map<XLink, set<XLink>> dep_to_all_stimulii;

    // Here we collect domain extension stimulus XLinks that we will re-create 
    // during PostUpdateActions() and then clear.
    set<XLink> stimulii_to_recheck;
    
    // SimpleCompare equivalence classes over the domain, with refcount = size of the class.
    map<TreePtr<Node>, ExtensionClass, SimpleCompare> induced_subtree_by_value_to_extra_subtree_and_refcount;
};
        
}

#endif
