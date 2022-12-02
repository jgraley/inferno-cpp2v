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
    typedef pair<XLink, TreePtr<Node>> TeleportResult;
    
    virtual TeleportResult RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink keyer_xlink ) const = 0;
    
    TreePtr<Node> ExpandNormalDomain( const XTreeDatabase *db, XLink xlink, set<XLink> &deps ) const override;

    virtual void Reset();    

	bool IsExtenderLess( const Extender &r ) const override;

    class TeleportOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        explicit TeleportOperator( const TeleportAgent *agent,
                                   shared_ptr<SymbolExpression> keyer ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual unique_ptr<SYM::SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                                 list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const override;

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
		void ReportTreeNode( const TreePtrInterface *p_tree_ptr ) override;
		set<XLink> GetDeps() const;
		void Clear();
		
	private:
		set<XLink> deps;
	};
};

};

#endif
