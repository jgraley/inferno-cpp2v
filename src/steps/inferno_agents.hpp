#ifndef INFERNO_AGENTS_HPP
#define INFERNO_AGENTS_HPP

#include "sr/agents/teleport_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "sr/agents/star_agent.hpp"
#include "sr/cache.hpp"
#include "sr/agents/builder_agent.hpp"
#include "sr/sym/expression.hpp"

using namespace SR;
// TODO pollutes client namespace
#define BYPASS_WHEN_IDENTICAL 1

//---------------------------------- BuildIdentifierAgent ------------------------------------    

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
    virtual Block GetGraphBlockInfo() const;
    Sequence<CPPTree::Identifier> sources;
    string GetNewName();
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
    virtual TreePtr<Node> BuildNewSubtree() override;
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
    virtual TreePtr<Node> BuildNewSubtree() override;
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
    virtual TreePtr<Node> BuildNewSubtree() override;
};

//---------------------------------- IdentifierByNameAgent ------------------------------------    

/// These can be used in search pattern to match a SpecificIdentifier by name.
/// The identifier must have a name that matches the string in `name`. One
/// cannot do this using a SpecificIdentifier in the search pattern because
/// the address of the node would be compared, not the name string.
/// Wildcarding is not supported and use of this node should be kept to
/// a minimum due to the risk of co-incidentla unwanted matches and the 
/// general principle that identifier names should not be important (it is
/// the identiy proprty itself that matters with identifiers). 
struct IdentifierByNameAgent : public virtual SearchLeafAgent 
{
    IdentifierByNameAgent( string n ) : name(n) {}
    virtual Block GetGraphBlockInfo() const;
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
    virtual pair<TreePtr<Node>, TreePtr<Node>> GetBounds( string name ) const { ASSERTFAIL(); } // TODO implemnt all and put back = 0
    string name;

    // We use the term _similar_ to describe nodes whose internal values and 
    // children are the same under Simple Compare, but can be different 
    // instances (which implies that root arrow-head identity is ignored). For 
    // most nodes, similar => equal under SC, but not for identifiers. Rule #528
    class AllIdentifiersNamedOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        AllIdentifiersNamedOperator( const IdentifierByNameAgent *iba,
                                     string name );
        KnowledgeLevel GetRequiredKnowledgeLevel() const override;
        list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
        unique_ptr<SYM::SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const final;
        string Render() const override;
        Precedence GetPrecedence() const override;
        
    private:
        const IdentifierByNameAgent * const iba;
        const string name;
        const pair<TreePtr<Node>, TreePtr<Node>> bounds;
    };

    class IsIdentifierNamedOperator : public SYM::PredicateOperator
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit IsIdentifierNamedOperator( const IdentifierByNameAgent *iba,
                                            string name,
                                            shared_ptr<SYM::SymbolExpression> a); 
        IsIdentifierNamedOperator *Clone() const override;
                                                   
        list<shared_ptr<SYM::SymbolExpression> *> GetSymbolOperandPointers() override;
        virtual unique_ptr<SYM::BooleanResult> Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const override;

        shared_ptr<SYM::Expression> TrySolveForToEqualNT( shared_ptr<SYM::Expression> target, 
                                                          shared_ptr<SYM::BooleanExpression> to_equal ) const override;

        virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
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

//---------------------------------- NestedAgent ------------------------------------    

/// Matching for the nested nature of array and struct nodes, both when declaring and 
/// when accessing arrays. The `terminus` is the node to be found at the end of
/// the recursion and `depth` is a string matching the steps taken to 
/// reach the terminus.
struct NestedAgent : public virtual TeleportAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       

    virtual LocatedLink RunTeleportQuery( XLink keyer_xlink ) const;                

    virtual XLink Advance( XLink xlink, 
                           string *depth ) const = 0;
    virtual Block GetGraphBlockInfo() const;
    
    TreePtr<Node> terminus; 
    TreePtr<CPPTree::String> depth;      

    class NestingOperator : public SYM::SymbolToSymbolExpression
    {
    public:    
        typedef SymbolExpression NominalType;
        explicit NestingOperator( const NestedAgent *agent,
                                  shared_ptr<SymbolExpression> keyer ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual unique_ptr<SYM::SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                                 list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const override;

        virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
        
    protected:
        const NestedAgent *agent;
        shared_ptr<SymbolExpression> keyer;
    };
};


/// Recurse through a number of nested `Array` nodes, but only by going through
/// the "element" member, not the "size" member. So this will get you from the type
/// of an instance to the type of the eventual element in a nested array decl.
/// `depth` is not checked.
struct NestedArrayAgent : NestedAgent, Special<CPPTree::Type>
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    virtual XLink Advance( XLink xlink, 
                           string *depth ) const;
};


/// Recurse through a number of `Subscript` nodes, but only going through
/// the base, not the index. Thus we seek the instance that contains the 
/// data we started with. Also go through member field of `Lookup` nodes.
/// The `depth` pointer must match a `String` node corresponding to the
/// nodes that were seen during traversal, where the letter `S` corresponds 
/// to a `Substript` and `L` corresponds to a `Lookup`.
struct NestedSubscriptLookupAgent : NestedAgent, Special<CPPTree::Expression>
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    virtual XLink Advance( XLink xlink, 
                           string *depth ) const;
};

//---------------------------------- BuildContainerSizeAgent ------------------------------------    

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
    virtual TreePtr<Node> BuildNewSubtree() override;
    virtual Block GetGraphBlockInfo() const;
}; 

#endif
