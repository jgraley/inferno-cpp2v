#include "sctree.hpp"
#include "node/syntax.hpp"
#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/type_data.hpp"
#include "vn/agents/standard_agent.hpp"
#include <string>

/// SCTree namespace contains node definitions that represent elements of SystemC, as native language elements
using namespace SCTree;

//////////////////////////// SCFunction ///////////////////////////////

Syntax::Production SCFunction::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	
}

//////////////////////////// Event ///////////////////////////////

string Event::GetLoweredIdOrMacroName() const 
{ 
	return "sc_event";
}

//////////////////////////// Module ///////////////////////////////

string Module::GetLoweredIdOrMacroName() const 
{ 
	return "sc_module"; 
}


any Module::GetStartingScopeContext() const 
{ 
	return (TreePtr<CPPTree::AccessSpec>)MakeTreeNode<CPPTree::Private>(); 
}   

//////////////////////////// Interface ///////////////////////////////

string Interface::GetLoweredIdOrMacroName() const 
{ 
	return "sc_interface"; 
}


any Interface::GetStartingScopeContext() const 
{ 
	return (TreePtr<CPPTree::AccessSpec>)MakeTreeNode<CPPTree::Private>(); 
} 

//////////////////////////// Wait ///////////////////////////////

Syntax::Production Wait::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Wait::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + "()";
}


string Wait::GetLoweredIdOrMacroName() const 
{ 
	return "wait"; 
}


YY::VNLangParser::token::token_kind_type Wait::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}

//////////////////////////// WaitDynamic ///////////////////////////////

string WaitDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy) +
	       " )";
}


string WaitDynamic::GetKeyword( Policy ) const 
{
	return "wait"; 
}


YY::VNLangParser::token::token_kind_type WaitDynamic::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> WaitDynamic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// WaitStatic ///////////////////////////////

string WaitStatic::GetKeyword( Policy ) const 
{
	return "wait"; 
}


TreePtr<Node> WaitStatic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type loc )
{
	// Seeing an argument makes this node evolve into WaitDynamic
	return MakeTreeNode<VN::StandardAgentWrapper<WaitDynamic>>()->OnSoloArg( arg, loc ); 
}

//////////////////////////// WaitDelta ///////////////////////////////

string WaitDelta::GetKeyword( Policy ) const 
{
	// Keep as wait_delta to differentiate from WaitStatic which is just wait with no args
	return "wait_delta"; 
}

//////////////////////////// NextTrigger ///////////////////////////////

Syntax::Production NextTrigger::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string NextTrigger::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + "()";
}


string NextTrigger::GetLoweredIdOrMacroName() const 
{ 
	return "next_trigger"; 
}


YY::VNLangParser::token::token_kind_type NextTrigger::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}

//////////////////////////// NextTriggerDynamic ///////////////////////////////

string NextTriggerDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy) +
	       " )";
}


string NextTriggerDynamic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}


YY::VNLangParser::token::token_kind_type NextTriggerDynamic::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> NextTriggerDynamic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// NextTriggerStatic ///////////////////////////////

string NextTriggerStatic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}


TreePtr<Node> NextTriggerStatic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type loc )
{
	// Evolve into NextTriggerDynamic and try again
	return MakeTreeNode<VN::StandardAgentWrapper<NextTriggerDynamic>>()->OnSoloArg(arg, loc);
}

//////////////////////////// NextTriggerDelta ///////////////////////////////

string NextTriggerDelta::GetKeyword( Policy ) const 
{
	// Keep as next_trigger_delta to differentiate from NextTriggerStatic which is just next trigger with no args
	return "next_trigger_delta"; 
}

//////////////////////////// Notify ///////////////////////////////

Syntax::Production Notify::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Notify::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy) +
	       " )";
}


string Notify::GetLoweredIdOrMacroName() const 
{ 
	return "notify"; 
}


YY::VNLangParser::token::token_kind_type Notify::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}


TreePtr<Node> Notify::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// NotifyImmediate ///////////////////////////////

string NotifyImmediate::GetKeyword( Policy ) const 
{
	return "notify"; 
}

//////////////////////////// NotifyDelta ///////////////////////////////

string NotifyDelta::GetKeyword( Policy ) const 
{
	// Keep as notify_delta to differentiate from NotifyImmediate which is just notify with one arg
	return "notify_delta"; 
}

//////////////////////////// NotifyTimed ///////////////////////////////

string NotifyTimed::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy) +
	       ", " + 
	       renderer->DoRender(&time, Production::SPACE_SEP_STMT_DECL, policy) +
	       " )";
}


string NotifyTimed::GetKeyword( Policy ) const 
{
	return "notify"; 
}


YY::VNLangParser::token::token_kind_type NotifyTimed::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> NotifyTimed::OnArgsList( list<TreePtr<Node>> args, YY::VNLangParser::location_type )
{
	event = args.front();
	time = args.back();
	return TreePtr<Node>( shared_from_this() );	
}


//////////////////////////// Method ///////////////////////////////

string Method::GetLoweredIdOrMacroName() const 
{ 
	return "SC_METHOD"; 
}

//////////////////////////// Thread ///////////////////////////////

string Thread::GetLoweredIdOrMacroName() const 
{ 
	return "SC_THREAD"; 
}

//////////////////////////// ClockedThread ///////////////////////////////

string ClockedThread::GetLoweredIdOrMacroName() const 
{ 
	return "SC_CTHREAD"; 
}

//////////////////////////// DeltaCount ///////////////////////////////

string DeltaCount::GetLoweredIdOrMacroName() const 
{ 
	return "sc_delta_count"; 
}    

//////////////////////////// Exit ///////////////////////////////

string Exit::GetLoweredIdOrMacroName() const 
{ 
	return "exit"; 
}

//////////////////////////// Cease ///////////////////////////////

string Cease::GetLoweredIdOrMacroName() const 
{ 
	return "cease"; 
}

