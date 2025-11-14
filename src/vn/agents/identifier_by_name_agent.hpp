#ifndef IDENTIFIER_BY_NAME_AGENT_HPP
#define IDENTIFIER_BY_NAME_AGENT_HPP

#include "vn/agents/relocating_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/star_agent.hpp"
#include "vn/agents/builder_agent.hpp"
#include "vn/sym/expression.hpp"

using namespace VN;

//---------------------------------- IdentifierByNameAgent ------------------------------------    
// 
/// These can be used in search pattern to match a SpecificIdentifier by name.
/// The identifier must have a name that matches the string in `name`. One
/// cannot do this using a SpecificIdentifier in the search pattern because
/// the address of the node would be compared, not the name string.
/// Wildcarding is not supported and use of this node should be kept to
/// a minimum due to the risk of co-incidental unwanted matches and the 
/// general principle that identifier names should not be important (it is
/// the identiy proprty itself that matters with identifiers). 
struct IdentifierByNameAgent : public virtual NonlocatingAgent 
{
    IdentifierByNameAgent( string n ) : name(n) {}

    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       
    virtual pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string ) const = 0;
    
    // We use the term _similar_ to describe nodes whose internal values and 
    // children are the same under Simple Compare, but can be different 
    // instances (which implies that root arrow-head identity is ignored). For 
    // most nodes, similar => equal under SC, but not for identifiers. Rule #528
    class IsIdentifierNamedOperator : public SYM::PredicateOperator
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit IsIdentifierNamedOperator( const IdentifierByNameAgent *iba,
                                            string name,
                                            shared_ptr<SYM::SymbolExpression> a); 
        shared_ptr<PredicateOperator> Clone() const override;
                                                   
        list<shared_ptr<SYM::SymbolExpression> *> GetSymbolOperandPointers() override;
        virtual unique_ptr<SYM::BooleanResult> Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;

        shared_ptr<SYM::SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SYM::SymbolVariable> target ) const override;

        Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;

        virtual string RenderNF() const override;
        virtual Precedence GetPrecedenceNF() const override;
        
    protected:
        const IdentifierByNameAgent * const iba;
        shared_ptr<SYM::SymbolExpression> a;
        const string name;
    };

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    bool IsFixedType() const final;
    virtual string GetIdentifierSubTypeName() const = 0;
    NodeBlock GetGraphBlockInfo() const final;
	static TreePtr<Node> TryMakeFromDestignatedType( string type_ns, string type_name, string matching_name );

    string name;

};

//---------------------------------- InstanceIdentifierByNameAgent ------------------------------------    

struct InstanceIdentifierByNameAgent : Special<CPPTree::InstanceIdentifier>,                             
                                       IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
        
    InstanceIdentifierByNameAgent() : IdentifierByNameAgent(string()) {}    
    InstanceIdentifierByNameAgent( string n ) : IdentifierByNameAgent(n) {}
    pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string name ) const override;
    string GetIdentifierSubTypeName() const final;
};

//---------------------------------- TypeIdentifierByNameAgent ------------------------------------    

struct TypeIdentifierByNameAgent : Special<CPPTree::TypeIdentifier>,                             
                                   IdentifierByNameAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TypeIdentifierByNameAgent() : IdentifierByNameAgent(string()) {}    
    TypeIdentifierByNameAgent( string n ) : IdentifierByNameAgent(n) {}                         
    pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string name ) const override;
    string GetIdentifierSubTypeName() const final;
};

//---------------------------------- LabelIdentifierByNameAgent ------------------------------------    

// OUT OF TEST COVERAGE - keep consistent or de-duplicate/template/macro
struct LabelIdentifierByNameAgent : Special<CPPTree::LabelIdentifier>,                             
                                    IdentifierByNameAgent  
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    LabelIdentifierByNameAgent() : IdentifierByNameAgent(string()) {}    
    LabelIdentifierByNameAgent( string n ) : IdentifierByNameAgent(n) {}                    
    pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string name ) const override;
    string GetIdentifierSubTypeName() const final;
};

//---------------------------------- PreprocessorIdentifierByNameAgent ------------------------------------    

struct PreprocessorIdentifierByNameAgent : Special<CPPTree::PreprocessorIdentifier>,                             
                                           IdentifierByNameAgent  
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    PreprocessorIdentifierByNameAgent() : IdentifierByNameAgent(string()) {}    
    PreprocessorIdentifierByNameAgent( string n ) : IdentifierByNameAgent(n) {}                    
    pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string name ) const override;
    string GetIdentifierSubTypeName() const final;
};

#endif
