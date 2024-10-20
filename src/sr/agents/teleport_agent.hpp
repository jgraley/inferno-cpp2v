#ifndef TELEPORT_AGENT_HPP
#define TELEPORT_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "../db/domain_extension.hpp"
#include "special_agent.hpp"

namespace SR
{ 

class TeleportAgent : public PreRestrictedAgent, public DomainExtension::Extender
{
public:    
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
    
    /*
     * The teleport query can return any node within the XTree at any location or
     * it can return a "extra" subtree that isn't part of the X tree at all.
     * 
     * Terminology: _stimulus_ X link is the one RunTeleportQuery acts on. It generates
     * an in-tree _result_ XLlink or an _induced_ node. Both are bases of subtrees. 
     * In the induced case, this subtree is not part of the current input x tree and
     * has been created by tyhe query (i.e. it's a free zone). Dependencies (see later) 
     * are just called _deps_. 
     * 
     * In the former case, `.first` of the return value (a pair) is non-NULL
     * and is the XLink in question. `.second` must be NULL. Nothing more
     * of these instructions need to be observed in this case. Reason: no extension
     * is required so there's nothing to maintain.
     * 
     * In the latter case, `.first` is NULL and `.second` is the extra node/subtree. 
     * In this case, domain extension will be required, and Vida Nova needs to be 
     * able to indentify when that domain extension might have gone out of date. A 
     * teleporter implmentation must report certain information to the supplied 
     * `DependencyReporter` object.
     * 
     * 1. To call `ReportTreeNode()` for any node in the tree that is a part of
     * determining the new subtree, and supply a pointer to the TreePtr instance
     * that points to that node. Reason: the deps, are the tree nodes that, 
     * if changed by a replace operation, force us to re-create the domain 
     * extension in case that needs to change too.
     * 
     * 2. deleted - see #696
     * 
     * Related: #689 #693 #696. Also see AugTreePtr<>.
     */
    typedef pair<XLink, TreePtr<Node>> QueryReturnType;    // TODO if this is worth a comment that big, it's worth a struct, lol. 
    // Just use a unique_ptr<Zone>: TreeZone = result, FreeZone = induced
        
    virtual QueryReturnType RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink stimulus_xlink ) const = 0;
    
    DomainExtension::Extender::Info GetDomainExtension( const XTreeDatabase *db, XLink stimulus_xlink ) const override;

    virtual void Reset();    

	bool IsExtenderChannelLess( const Extender &r ) const override;

    class TeleportOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        explicit TeleportOperator( const TeleportAgent *agent,
                                   shared_ptr<SymbolExpression> keyer ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual unique_ptr<SYM::SymbolicResult> Evaluate( const EvalKit &kit,
                                                                 list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;

        Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
		const TeleportAgent *GetAgent() const;
        
    protected:
        const TeleportAgent *agent;
        shared_ptr<SymbolExpression> keyer;
    };
    
    class DepRep : public DependencyReporter
	{
	public:	
		void ReportTreeNode( TreePtr<Node> tree_ptr ) override;

		set<TreePtr<Node>> GetDeps() const;
		void Clear();
		
	private:
		set<TreePtr<Node>> deps;
	};
};

};

#endif
