#ifndef INFERNO_AGENTS_HPP
#define INFERNO_AGENTS_HPP

#include "vn/agents/relocating_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/star_agent.hpp"
#include "vn/agents/builder_agent.hpp"
#include "vn/sym/expression.hpp"

using namespace VN;
// TODO pollutes client namespace
#define BYPASS_WHEN_IDENTICAL 1

//---------------------------------- BuildIdentifierAgent ------------------------------------    
// 
/// Make an identifer based on an existing set, `sources` and a printf
/// format string, `format`. The new identfier is named using
/// `sprintf( format, sources[0].name, source[1].name, etc )`
/// You must key the source identifier to somehting in the search pattern (so it
/// will get substitued to the real Specific identifier found in the tree) and
/// you may couple the `BuildIdentifierAgent` in the replace pattern if you need
/// to specify it in more than one place. Note that `BuildIdentifierAgent` is stateless
/// and cannot therefore keep track of uniqueness - you'll get a new one each time
/// and must rely on a replace coupling to get multiple reference to the same
/// new identifier. Rule is: ONE of these per new identifier. 
/// `flags` permits special behaviour when nonzero, as follows.
/// `BYPASS_WHEN_IDENTICAL` means if all the names of the source nodes are the
/// same, that name is used. This reduces verbosity and is a good fit when
/// in some sense you are "merging" objects with identifiers.
struct BuildIdentifierAgent : public virtual BuilderAgent    
{
    // TODO do this via a transformation as with TransformOf/TransformOf
    BuildIdentifierAgent( string s, int f=0 ) : format(s), flags(f) {}

    TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) final;
    string GetNewName(const SCREngine *acting_engine);
    virtual TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const = 0;

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    virtual string GetIdentifierSubTypeName() const = 0;
    NodeBlock GetGraphBlockInfo() const final;

    Sequence<CPPTree::Identifier> sources;
    string format;
    int flags;
};


struct BuildInstanceIdentifierAgent : Special<CPPTree::InstanceIdentifier>,                             
                                      BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    BuildInstanceIdentifierAgent( string s, int f=0 ) : BuildIdentifierAgent(s,f) {}
    BuildInstanceIdentifierAgent() : BuildIdentifierAgent("unnamed") {}

private:    
    TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const final;
    string GetIdentifierSubTypeName() const final;
};


struct BuildTypeIdentifierAgent : Special<CPPTree::TypeIdentifier>,                             
                                  BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    BuildTypeIdentifierAgent( string s="Unnamed", int f=0 ) : BuildIdentifierAgent(s,f) {}

private:
    TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const final;
    string GetIdentifierSubTypeName() const final;
};


struct BuildLabelIdentifierAgent : Special<CPPTree::LabelIdentifier>,                             
                                   BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    BuildLabelIdentifierAgent() : BuildIdentifierAgent("UNNAMED") {}
    BuildLabelIdentifierAgent( string s, int f=0 ) : BuildIdentifierAgent(s,f) {}

private:
    TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const final;
    string GetIdentifierSubTypeName() const final;
};

//---------------------------------- StringizeAgent ------------------------------------    

// Make a String node containing the token string of an identifier. No uniquing is attempted.
struct StringizeAgent : Special<CPPTree::String>,
                        public virtual BuilderAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }

    StringizeAgent() {}

    TreePtr<CPPTree::Identifier> source;

private:
    TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) override;    
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    NodeBlock GetGraphBlockInfo() const override;
};

//---------------------------------- IdentifierByNameAgent ------------------------------------    
// 🞊
/// These can be used in search pattern to match a SpecificIdentifier by name.
/// The identifier must have a name that matches the string in `name`. One
/// cannot do this using a SpecificIdentifier in the search pattern because
/// the address of the node would be compared, not the name string.
/// Wildcarding is not supported and use of this node should be kept to
/// a minimum due to the risk of co-incidentla unwanted matches and the 
/// general principle that identifier names should not be important (it is
/// the identiy proprty itself that matters with identifiers). 
struct IdentifierByNameAgent : public virtual NonlocatingAgent 
{
    IdentifierByNameAgent( string n ) : name(n) {}
    NodeBlock GetGraphBlockInfo() const final;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       

    virtual pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string ) const { ASSERTFAIL(); } // TODO implemnt all and put back = 0
    string name;

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
};

//---------------------------------- BuildContainerSizeAgent ------------------------------------    
// ⧆
/// `BuildContainerSizeAgent` is used in replace context to create an integer-valued
/// constant that is the size of a `Star` node pointed to by `container`. The
/// container should couple the star node.
struct BuildContainerSizeAgent : public virtual BuilderAgent,
                                 Special<CPPTree::Integer>
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<Node> container;
private:
    virtual TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) override;
    NodeBlock GetGraphBlockInfo() const final;
}; 

#endif
