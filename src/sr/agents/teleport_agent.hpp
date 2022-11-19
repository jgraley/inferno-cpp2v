#ifndef TELEPORT_AGENT_HPP
#define TELEPORT_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "special_agent.hpp"

namespace SR
{ 

class TeleportAgent : public PreRestrictedAgent
{
public:    
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
    typedef pair<XLink, TreePtr<Node>> TeleportResult;
    
    virtual TeleportResult RunTeleportQuery( const TreeKit &kit, XLink keyer_xlink ) const = 0;
    
    set<TreePtr<Node>> ExpandNormalDomain( const TreeKit &kit, const unordered_set<XLink> &xlinks ) override;

    virtual void Reset();    

private:
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
        
    protected:
        const TeleportAgent *agent;
        shared_ptr<SymbolExpression> keyer;
    };
};

};

#endif
