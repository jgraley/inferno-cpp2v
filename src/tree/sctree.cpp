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


string Wait::GetLoweredIdOrMacroName() const 
{ 
	return "wait"; 
}

//////////////////////////// WaitDynamic ///////////////////////////////

string WaitDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + " " + renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy);
}

string WaitDynamic::GetKeyword( Policy ) const 
{
	return "wait"; 
}

//////////////////////////// WaitStatic ///////////////////////////////

string WaitStatic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string WaitStatic::GetKeyword( Policy ) const 
{
	return "wait"; 
}


YY::VNLangParser::token::token_kind_type WaitStatic::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}


TreePtr<Node> WaitStatic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	// Seeing an argument makes this node evolve into WaitDynamic
	auto wd = MakeTreeNode<VN::StandardAgentWrapper<WaitDynamic>>();
	wd->event = arg;
	return wd; 
}

//////////////////////////// WaitDelta ///////////////////////////////

string WaitDelta::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string WaitDelta::GetKeyword( Policy ) const 
{
	// Keep as wait_delta to differentiate from WaitStatic which is just wait with no args
	return "wait_delta"; 
}


YY::VNLangParser::token::token_kind_type WaitDelta::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}

//////////////////////////// NextTrigger ///////////////////////////////

Syntax::Production NextTrigger::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string NextTrigger::GetLoweredIdOrMacroName() const 
{ 
	return "next_trigger"; 
}

//////////////////////////// NextTriggerDynamic ///////////////////////////////

string NextTriggerDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + " " + renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy);
}

string NextTriggerDynamic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}

//////////////////////////// NextTriggerStatic ///////////////////////////////

string NextTriggerStatic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string NextTriggerStatic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}


YY::VNLangParser::token::token_kind_type NextTriggerStatic::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}


TreePtr<Node> NextTriggerStatic::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	// Seeing an argument makes this node evolve into WaitDynamic
	auto wd = MakeTreeNode<VN::StandardAgentWrapper<NextTriggerDynamic>>();
	wd->event = arg;
	return wd; 
}

//////////////////////////// NextTriggerDelta ///////////////////////////////

string NextTriggerDelta::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string NextTriggerDelta::GetKeyword( Policy ) const 
{
	// Keep as next_trigger_delta to differentiate from NextTriggerStatic which is just next trigger with no args
	return "next_trigger_delta"; 
}


YY::VNLangParser::token::token_kind_type NextTriggerDelta::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}

//////////////////////////// Notify ///////////////////////////////

Syntax::Production Notify::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Notify::GetLoweredIdOrMacroName() const 
{ 
	return "notify"; 
}


TreePtr<Node> Notify::OnSoloArg( TreePtr<Node> arg, YY::VNLangParser::location_type )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// NotifyImmediate ///////////////////////////////

string NotifyImmediate::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + " " + renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy);
}


YY::VNLangParser::token::token_kind_type NotifyImmediate::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}

//////////////////////////// NotifyDelta ///////////////////////////////

string NotifyDelta::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + " " + renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy);
}


YY::VNLangParser::token::token_kind_type NotifyDelta::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}

//////////////////////////// NotifyTimed ///////////////////////////////

string NotifyTimed::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       " " + 
	       renderer->DoRender(&event, Production::SPACE_SEP_STMT_DECL, policy) +
	       ", " + 
	       renderer->DoRender(&time, Production::SPACE_SEP_STMT_DECL, policy);
}


YY::VNLangParser::token::token_kind_type NotifyTimed::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
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

