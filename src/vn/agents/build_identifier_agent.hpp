#ifndef BUILD_IDENTIFIER_AGENT_HPP
#define BUILD_IDENTIFIER_AGENT_HPP

#include "vn/agents/relocating_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/star_agent.hpp"
#include "vn/agents/builder_agent.hpp"
#include "vn/sym/expression.hpp"

using namespace VN;

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
    BuildIdentifierAgent( string s ) : format(s) {}

    TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) final;
    string GetNewName(const SCREngine *acting_engine);
    virtual TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const = 0;

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    bool IsFixedType() const final;    
    virtual string GetIdentifierSubTypeName() const = 0;
    NodeBlock GetGraphBlockInfo() const final;
	static TreePtr<Node> TryMakeFromDestignatedType( string type_ns, string type_name, string matching_name );

    Sequence<Node> sources;
    string format;
};


struct BuildInstanceIdentifierAgent : Special<CPPTree::InstanceIdentifier>,                             
                                      BuildIdentifierAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    BuildInstanceIdentifierAgent( string s="" ) : BuildIdentifierAgent(s) {}

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
    
    BuildTypeIdentifierAgent( string s="" ) : BuildIdentifierAgent(s) {}

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
    
    BuildLabelIdentifierAgent( string s="" ) : BuildIdentifierAgent(s) {}

private:
    TreePtr<CPPTree::SpecificIdentifier> BuildSpecificIdentifier( string name ) const final;
    string GetIdentifierSubTypeName() const final;
};

#endif
