#ifndef RELOCATING_AGENT_HPP
#define RELOCATING_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "../db/domain_extension.hpp"
#include "special_agent.hpp"

namespace SR
{ 
//---------------------------------- RelocatingAgent ------------------------------------    

class RelocatingAgent : public PreRestrictedAgent, public DomainExtension::Extender // names finalised in rule #729
{
public:    
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       
    
    class Dependencies
    {
    public:    
        Dependencies() = default;
        Dependencies( const Dependencies & ) = default;
        Dependencies &operator=( const Dependencies & ) = default;

        void AddDep( XLink dep );                        
        void AddChainTo( shared_ptr<Dependencies> chain );                        
        void AddAllFrom( const Dependencies &other );

        set<XLink> GetAll() const; 
        void Clear();
        
    private:
        set<XLink> deps; 
        set<shared_ptr<Dependencies>> chains; // TODO try weak_ptr
        // Note on chaining: this allows us to say, "This ATP's deps will always 
        // include that ATP's deps, even if that ATP's deps change later". It
        // effects propagation in the reverse direction of the chaining which
        // is executed at the end by GetAll().
    };
    
    //---------------------------------- RelocatingQueryResult ------------------------------------    
    
    class RelocatingQueryResult
    {
    public:
        RelocatingQueryResult(); // Invalid (relocation failed)
        RelocatingQueryResult( XLink base_xlink ); // Inside X tree
        RelocatingQueryResult( TreePtr<Node> induced_base_node, const set<XLink> &deps ); // Induced
        RelocatingQueryResult( TreePtr<Node> induced_base_node, const Dependencies &deps ); // Induced
        
        bool IsValid() const;
        bool IsXTree() const;
        bool IsInduced() const;
        DomainExtension::Extender::Info TryGetDEInfo() const;
        XLink GetBaseXLink() const;
    
    private:
        DomainExtension::Extender::Info de_info; // always filled in
        XLink base_xlink; // NULL if the base is outside of X tree
    };
        
       /*
     * The relocation query can return any node within the XTree at any location or
     * it can return an "induced" subtree that isn't part of the X tree at all.
     * 
     * Terminology: _stimulus_ X link is the one RunRelocatingQuery acts on. It generates
     * an in-tree _result_ XLlink or an _induced_ node. Both are bases of subtrees. 
     * In the induced case, this subtree is not part of the current input x tree and
     * has been created by tyhe query (i.e. it's a free zone). Dependencies (see later) 
     * are just called _deps_. 
     * 
     * Use RelocatingQueryResult for the results of a query. If the result is inside 
     * the X tree, use XLink constructor. If outside, use the induced_base_node/deps
     * constructor. Deps are required in this case: either provide a simple set of
     * XLinks (we're only interested in deps inside the x tree) or use the 
     * Dependencies class (chains are collapsed on passing to RelocatingQueryResult
     * constructor).
     */
    // TODO a checker: walk entire subtree: boundary XLinks to be in deps; they and all below should be in database
    virtual RelocatingQueryResult RunRelocatingQuery( const XTreeDatabase *db, XLink stimulus_xlink ) const = 0;
    
    DomainExtension::Extender::Info GetDomainExtension( const XTreeDatabase *db, XLink stimulus_xlink ) const override;

    virtual void Reset();    

    bool IsExtenderChannelLess( const Extender &r ) const override;

    class RelocateOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        explicit RelocateOperator( const RelocatingAgent *agent,
                                   shared_ptr<SymbolExpression> keyer ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual unique_ptr<SYM::SymbolicResult> Evaluate( const EvalKit &kit,
                                                          list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;

        Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
        const RelocatingAgent *GetAgent() const;
        
    protected:
        const RelocatingAgent *agent;
        shared_ptr<SymbolExpression> keyer;
    };    
};

};

#endif
