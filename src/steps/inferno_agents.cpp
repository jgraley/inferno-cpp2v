#include "inferno_agents.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "sr/agents/agent.hpp"
#include "sr/agents/standard_agent.hpp"
#include "sr/scr_engine.hpp"
#include "sr/search_replace.hpp"
#include "sr/link.hpp"
#include "sr/sym/primary_expressions.hpp"
#include "sr/sym/result.hpp"
#include "sr/sym/overloads.hpp"
#include "sr/sym/boolean_operators.hpp"

// Not pulling in SYM because it clashes with CPPTree
//using namespace SYM;
using namespace CPPTree;

//---------------------------------- BuildIdentifierAgent ------------------------------------    

Graphable::Block BuildIdentifierAgent::GetGraphBlockInfo() const
{
	// The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
	// the printf format string that controls the name of the generated identifier inside it.
	// TODO indicate whether it's building instance, label or type identifier
    Block block;
	block.bold = true;
	block.title = "'"+format+"'!"; // text from program code, so use single quotes
	block.shape = "parallelogram";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.sub_blocks = Node::GetSubblocks(const_cast<Sequence<CPPTree::Identifier> *>(&sources), phase);
    return block;
}


string BuildIdentifierAgent::GetNewName()
{
    INDENT("B");
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    vector<string> vs;
    bool all_same = true;
    FOREACH( TreePtrInterface &source, sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        PatternLink source_plink(this, &source);
        TreePtr<Node> new_identifier = source_plink.GetChildAgent()->BuildReplace(source_plink);
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( new_identifier );
        TreePtr<SpecificIdentifier> si = DynamicTreePtrCast<SpecificIdentifier>( new_identifier );
        ASSERT( si )("BuildIdentifier: ")(*new_identifier)(" should be a kind of SpecificIdentifier (format is %s)", format.c_str());
        string s = si->GetRender();
        if( !vs.empty() )
            all_same = all_same && (s == vs.back());
        vs.push_back( s );
    }

    // Optional functionality: when every identifier has the same name, just return that
    // name. Handy for "merging" operations.
    if( (flags & BYPASS_WHEN_IDENTICAL) && all_same )
        return vs[0];  

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( vs.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), vs[0].c_str() );
        case 2:
            return SSPrintf( format.c_str(), vs[0].c_str(), vs[1].c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}

//---------------------------------- BuildInstanceIdentifierAgent ------------------------------------    

TreePtr<Node> BuildInstanceIdentifierAgent::BuildNewSubtree()
{
    return MakeTreePtr<CPPTree::SpecificInstanceIdentifier>( GetNewName() ); 
}

//---------------------------------- BuildTypeIdentifierAgent ------------------------------------    

TreePtr<Node> BuildTypeIdentifierAgent::BuildNewSubtree()
{
    return MakeTreePtr<CPPTree::SpecificTypeIdentifier>( GetNewName() ); 
}                                                   

//---------------------------------- BuildLabelIdentifierAgent ------------------------------------    

TreePtr<Node> BuildLabelIdentifierAgent::BuildNewSubtree()
{
    return MakeTreePtr<CPPTree::SpecificLabelIdentifier>( GetNewName() ); 
}                                                   

//---------------------------------- IdentifierByNameAgent ------------------------------------    

Graphable::Block IdentifierByNameAgent::GetGraphBlockInfo() const
{
	// The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
	// the string that must be matched inside it.
	// TODO indicate whether it's matching instance, label or type identifier
    // Update: PreRestriction indicator seems to be doing that now
    Block block;
	block.bold = true;
    block.title = "'" + name + "'?";	
	block.shape = "trapezium";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    return block;
}


void IdentifierByNameAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                                 XLink keyer_xlink ) const                
{
    string newname = name; 
    TreePtr<Node> base_x = keyer_xlink.GetChildX(); // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( auto si_x = DynamicTreePtrCast<CPPTree::SpecificIdentifier>(base_x) )
    {
        TRACE("Comparing ")(si_x->GetRender())(" with ")(newname);
        if( si_x->GetRender() == newname )
        {
            TRACE(" : same\n");
            return;
        }
        TRACE(" : different\n");
    }
    throw Mismatch();  
}           
                     

SYM::Over<SYM::BooleanExpression> IdentifierByNameAgent::SymbolicNormalLinkedQueryPRed() const
{
    auto keyer_expr = SYM::MakeOver<SYM::SymbolVariable>(keyer_plink);
    return SYM::MakeOver<IdentifierByNameOperator>(name, keyer_expr);
}


IdentifierByNameAgent::IdentifierByNameOperator::IdentifierByNameOperator( string name_,
                                                                           shared_ptr<SYM::SymbolExpression> a_ ) :
    a( a_ ),
    name( name_ )
{    
}                                                


list<shared_ptr<SYM::SymbolExpression>> IdentifierByNameAgent::IdentifierByNameOperator::GetSymbolOperands() const
{
    return { a };
}


shared_ptr<SYM::BooleanResultInterface> IdentifierByNameAgent::IdentifierByNameOperator::Evaluate( const EvalKit &kit,
                                                                                          const list<shared_ptr<SYM::SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SYM::SymbolResultInterface> ra = OnlyElementOf(op_results);
    if( !ra->IsDefinedAndUnique() )
        return make_shared<SYM::BooleanResult>( SYM::BooleanResult::UNDEFINED );
    
    TreePtr<Node> base_x = ra->GetAsXLink().GetChildX(); // TODO dynamic_pointer_cast support for TreePtrInterface #27
    if( auto si_x = DynamicTreePtrCast<CPPTree::SpecificIdentifier>(base_x) )
    {
        TRACE("Comparing ")(si_x->GetRender())(" with ")(name);
        if( si_x->GetRender() == name )
        {
            TRACE(" : same\n");
            return make_shared<SYM::BooleanResult>( SYM::BooleanResult::DEFINED, true );
        }
        TRACE(" : different\n");
    }
    return make_shared<SYM::BooleanResult>( SYM::BooleanResult::DEFINED, false );
}


Orderable::Result IdentifierByNameAgent::IdentifierByNameOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                                      OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);
    return name.compare(c->name);
}  


string IdentifierByNameAgent::IdentifierByNameOperator::Render() const
{
    return "IdentifierByName<\"" + name  + "\">(" + a->Render() + ")"; 
}


SYM::Expression::Precedence IdentifierByNameAgent::IdentifierByNameOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

//---------------------------------- NestedAgent ------------------------------------    

shared_ptr<PatternQuery> NestedAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, &terminus) );
	if( depth )
		pq->RegisterNormalLink( PatternLink(this, &depth) ); // local
    return pq;
}

    
SYM::Over<SYM::BooleanExpression> NestedAgent::SymbolicNormalLinkedQueryPRed() const                                      
{                 
    shared_ptr<PatternQuery> my_pq = GetPatternQuery();         
    PatternLink child_plink = my_pq->GetNormalLinks().front();
    
    SYM::Over<SYM::SymbolExpression> keyer_expr = SYM::MakeOver<SYM::SymbolVariable>(keyer_plink);
    SYM::Over<SYM::SymbolExpression> child_expr = SYM::MakeOver<SYM::SymbolVariable>(child_plink);
    
    SYM::Over<SYM::BooleanExpression> expr = SYM::MakeOver<NestingOperator>( this, keyer_expr ) == child_expr;
    
    if( depth )
        expr &= TeleportAgent::SymbolicNormalLinkedQueryPRed();
        
    return expr;
}                     


LocatedLink NestedAgent::RunTeleportQuery( XLink keyer_xlink ) const
{
    LocatedLink tp_link;
    
    // Compare the depth with the supplied pattern if present
    if( depth )
    {
        string s;
        // Keep advancing until we get nullptr, and remember the last non-null position
        int i = 0;
        XLink xlink = keyer_xlink;
        while( XLink next_xlink = Advance(xlink, &s) )
            xlink = next_xlink;

        TreePtr<Node> cur_depth( new SpecificString(s) );
        XLink new_xlink = XLink::CreateDistinct(cur_depth); // cache will un-distinct
        tp_link = LocatedLink(PatternLink(this, &depth), new_xlink);
    }
    
    return tp_link;
}    


Graphable::Block NestedAgent::GetGraphBlockInfo() const
{
    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "plaintext";
    block.block_type = Graphable::NODE_EXPANDED;
    block.node = GetPatternPtr();
    if( terminus )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(terminus.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &terminus );
        block.sub_blocks.push_back( { "terminus", 
                                      "", 
                                      false,
                                      { link } } );
    }
    if( depth )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(depth.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &depth );
        block.sub_blocks.push_back( { "depth", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}


NestedAgent::NestingOperator::NestingOperator( const NestedAgent *agent_,
                                               shared_ptr<SymbolExpression> keyer_ ) :
    agent( agent_ ),
    keyer( keyer_ )
{    
}                                                


list<shared_ptr<SYM::SymbolExpression>> NestedAgent::NestingOperator::GetSymbolOperands() const
{
    return { keyer };
}


shared_ptr<SYM::SymbolResultInterface> NestedAgent::NestingOperator::Evaluate( const EvalKit &kit,
                                                                          const list<shared_ptr<SYM::SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SYM::SymbolResultInterface> keyer_result = OnlyElementOf(op_results);
    if( !keyer_result->IsDefinedAndUnique() )
        return make_shared<SYM::SymbolResult>( SYM::SymbolResult::UNDEFINED );
    XLink keyer_xlink = keyer_result->GetAsXLink();
    
    // Keep advancing until we get nullptr, and remember the last non-null position
    string s;
    int i = 0;
    XLink xlink = keyer_xlink;
    while( XLink next_xlink = agent->Advance(xlink, &s) )
        xlink = next_xlink;
        
    return make_shared<SYM::SymbolResult>( SYM::ResultInterface::DEFINED, xlink );        
}


Orderable::Result NestedAgent::NestingOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                      OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses address to ensure different dirty_grass sets compare differently
        r = (int)(agent > c->agent) - (int)(agent < c->agent);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
        // Repeatable ordering stops after name check since address compare is not repeatable
        r = Orderable::EQUAL;
        break;
    }
    return r;
}  


string NestedAgent::NestingOperator::Render() const
{
    return "Nesting<" + agent->GetName() + ">(" + keyer->Render() + ")"; 
}


SYM::Expression::Precedence NestedAgent::NestingOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

//---------------------------------- NestedArrayAgent ------------------------------------    

XLink NestedArrayAgent::Advance( XLink xlink, 
                                 string *depth ) const
{
    if( auto a = TreePtr<Array>::DynamicCast(xlink.GetChildX()) )         
        return XLink(a, &(a->element)); // TODO support depth string (or integer)
    else
        return XLink();
}

//---------------------------------- NestedSubscriptLookupAgent ------------------------------------    

XLink NestedSubscriptLookupAgent::Advance( XLink xlink, 
                                           string *depth ) const
{
    if( auto s = DynamicTreePtrCast<Subscript>(xlink.GetChildX()) )            
    {
        *depth += "S";
        return XLink(s, &(s->operands.front())); // the base, not the index
    }
    else if( auto l = DynamicTreePtrCast<Lookup>(xlink.GetChildX()) )            
    {
        *depth += "L";
        return XLink(l, &(l->member)); 
    }
    else
    {
        return XLink();
    }
}

//---------------------------------- BuildContainerSizeAgent ------------------------------------    

TreePtr<Node> BuildContainerSizeAgent::BuildNewSubtree()
{
	ASSERT( container );
    PatternLink container_plink(this, &container);
    TreePtr<Node> new_node = container_plink.GetChildAgent()->BuildReplace(container_plink);
    ASSERT( new_node );
	ContainerInterface *new_container = dynamic_cast<ContainerInterface *>(new_node.get());
	ASSERT( new_container );
	int size = new_container->size();
    return MakeTreePtr<SpecificInteger>(size); 
}                                                   


Graphable::Block BuildContainerSizeAgent::GetGraphBlockInfo() const
{

    Block block;
	block.bold = false;
	block.title = GetName();
	block.shape = "egg";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    if( container )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(container.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &container );
        block.sub_blocks.push_back( { "container", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}

