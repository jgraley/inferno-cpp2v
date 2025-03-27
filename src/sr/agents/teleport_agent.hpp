#ifndef RELOCATED_AGENT_HPP
#define RELOCATED_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "../db/domain_extension.hpp"
#include "special_agent.hpp"

namespace SR
{ 

class RelocatedAgent : public PreRestrictedAgent, public DomainExtension::Extender // names finalised in rule #729
{
public:    
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
    
    class Dependencies
    {
    public:    
        void AddDep( XLink dep );                        
        void AddChainTo( shared_ptr<Dependencies> chain );                        
        void CopyAllFrom( const Dependencies &other );

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
    
    /*
     * The teleport query can return any node within the XTree at any location or
     * it can return an "induced" subtree that isn't part of the X tree at all.
     * 
     * Terminology: _stimulus_ X link is the one RunRelocatedQuery acts on. It generates
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
     * `Dependencies` object.
     * 
     * 1. To call `AddDep()` for any node in the tree that is a part of
     * determining the new subtree, and supply a pointer to the TreePtr instance
     * that points to that node. Reason: the deps, are the tree nodes that, 
     * if changed by a replace operation, force us to re-create the domain 
     * extension in case that needs to change too.
     * 
     * 2. deleted - see #696
     * 
     * Related: #689 #693 #696. Also see AugTreePtr<>.
     */
    // TODO a checker: walk entire subtree: boundary XLinks to be in deps; they and all below should be in database
    // TODO rewrite above comment

    //---------------------------------- RelocatedQueryResult ------------------------------------    
    
    class RelocatedQueryResult
    {
    public:
        RelocatedQueryResult(); // Invalid (relocation failed)
        RelocatedQueryResult( XLink base_xlink ); // Inside X tree
        RelocatedQueryResult( TreePtr<Node> induced_base_node, const set<XLink> &deps ); // Induced
        RelocatedQueryResult( TreePtr<Node> induced_base_node, const Dependencies &deps ); // Induced
        
        bool IsValid() const;
        bool IsXTree() const;
        bool IsInduced() const;
        DomainExtension::Extender::Info TryGetDEInfo() const;
        XLink GetBaseXLink() const;
    
    private:
        DomainExtension::Extender::Info de_info; // always filled in
        XLink base_xlink; // NULL if the base is outside of X tree
    };
        
    //---------------------------------- RelocatedAgent ------------------------------------    

    virtual RelocatedQueryResult RunRelocatedQuery( const XTreeDatabase *db, XLink stimulus_xlink ) const = 0;
    
    DomainExtension::Extender::Info GetDomainExtension( const XTreeDatabase *db, XLink stimulus_xlink ) const override;

    virtual void Reset();    

    bool IsExtenderChannelLess( const Extender &r ) const override;

    class TeleportOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        explicit TeleportOperator( const RelocatedAgent *agent,
                                   shared_ptr<SymbolExpression> keyer ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual unique_ptr<SYM::SymbolicResult> Evaluate( const EvalKit &kit,
                                                          list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;

        Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
        const RelocatedAgent *GetAgent() const;
        
    protected:
        const RelocatedAgent *agent;
        shared_ptr<SymbolExpression> keyer;
    };    
};

};

#endif
