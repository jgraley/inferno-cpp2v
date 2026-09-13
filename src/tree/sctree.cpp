#include "sctree.hpp"
#include "node/syntax.hpp"
#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/type_data.hpp"
#include "vn/agents/standard_agent.hpp"
#include "vn/lang/vn_lang.ypp.hpp"
#include "vn/lang/vn_lang.location.hpp"
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


string Event::GetKeyword( Policy ) const
{
	return "event";
}


Syntax::Token Event::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_TYPE_KEYWORD;
}

//////////////////////////// Module ///////////////////////////////

string Module::GetLoweredIdOrMacroName() const 
{ 
	return "sc_module"; 
}


string Module::GetKeyword( Policy ) const
{
	return "module";
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


string Interface::GetKeyword( Policy ) const
{
	return "interface";
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


Syntax::Token Wait::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}

//////////////////////////// WaitDynamic ///////////////////////////////

string WaitDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::COMMA_SEP, policy) +
	       " )";
}


string WaitDynamic::GetKeyword( Policy ) const 
{
	return "wait"; 
}


Syntax::Token WaitDynamic::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> WaitDynamic::OnSoloArg( TreePtr<Node> arg, Location )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// WaitStatic ///////////////////////////////

string WaitStatic::GetKeyword( Policy ) const 
{
	return "wait"; 
}


TreePtr<Node> WaitStatic::OnSoloArg( TreePtr<Node> arg, Location loc )
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


Syntax::Token NextTrigger::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}

//////////////////////////// NextTriggerDynamic ///////////////////////////////

string NextTriggerDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy) + 
	       "( " + 
	       renderer->DoRender(&event, Production::COMMA_SEP, policy) +
	       " )";
}


string NextTriggerDynamic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}


Syntax::Token NextTriggerDynamic::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> NextTriggerDynamic::OnSoloArg( TreePtr<Node> arg, Location )
{
	event = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// NextTriggerStatic ///////////////////////////////

string NextTriggerStatic::GetKeyword( Policy ) const 
{
	return "next_trigger"; 
}


TreePtr<Node> NextTriggerStatic::OnSoloArg( TreePtr<Node> arg, Location loc )
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
	return renderer->DoRender(&event, Production::POSTFIX, policy) +
		   "." +
		   renderer->GetKeyword(this, policy) + 
	       "()";
}


string Notify::GetLoweredIdOrMacroName() const 
{ 
	return "notify"; 
}


Syntax::Token Notify::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_METHOD;
}


TreePtr<Node> Notify::OnObject( TreePtr<Node> object, Location )
{
	event = object;
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
	return renderer->DoRender(&event, Production::POSTFIX, policy) +
	       "." + 
	       renderer->GetKeyword(this, policy) + 
	       "( " + // TODO factor out a render for the args
	       renderer->DoRender(&time, Production::COMMA_SEP, policy) +
	       " )";
}


string NotifyTimed::GetKeyword( Policy ) const 
{
	return "notify"; 
}


Syntax::Token NotifyTimed::GetSignifierToken() const
{
	return Syntax::GetSignifierToken(); // retract the token - we want to recognise WaitStatic instead
}


TreePtr<Node> NotifyTimed::OnSoloArg( TreePtr<Node> arg, Location )
{
	time = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// Process //////////////////////////// 

Syntax::Production Process::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_TYPE; 
}


Syntax::Token Process::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_TYPE_KEYWORD;
}

//////////////////////////// Method ///////////////////////////////

string Method::GetLoweredIdOrMacroName() const 
{ 
	return "SC_METHOD"; 
}


string Method::GetKeyword( Policy ) const 
{
	return "method"; 
}

//////////////////////////// Thread ///////////////////////////////

string Thread::GetLoweredIdOrMacroName() const 
{ 
	return "SC_THREAD"; 
}


string Thread::GetKeyword( Policy ) const 
{
	return "thread"; 
}

//////////////////////////// ClockedThread ///////////////////////////////

string ClockedThread::GetLoweredIdOrMacroName() const 
{ 
	return "SC_CTHREAD"; 
}

string ClockedThread::GetKeyword( Policy ) const 
{
	return "cthread"; 
}

//////////////////////////// DeltaCount ///////////////////////////////

string DeltaCount::GetLoweredIdOrMacroName() const 
{ 
	return "sc_delta_count"; 
}    


Syntax::Production DeltaCount::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	// renders like a function call
}


string DeltaCount::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{				
	return renderer->GetKeyword(this, policy) + "()";
}


string DeltaCount::GetKeyword( Policy ) const 
{
	return "delta_count"; 
}


Syntax::Token DeltaCount::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}

//////////////////////////// TerminationFunction ///////////////////////////////

Syntax::Production TerminationFunction::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	
}


string TerminationFunction::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{				
	return renderer->GetKeyword(this, policy) + 
	       "( " +
	       renderer->DoRender(&code, Production::COMMA_SEP, policy) +
	       " )";
}


Syntax::Token TerminationFunction::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_LIBRARY_FUNC;
}


TreePtr<Node> TerminationFunction::OnSoloArg( TreePtr<Node> arg, Location )
{
	code = arg;
	return TreePtr<Node>( shared_from_this() );	
}

//////////////////////////// Exit ///////////////////////////////

string Exit::GetLoweredIdOrMacroName() const 
{ 
	return "exit"; 
}


string Exit::GetKeyword( Policy ) const
{
	return "exit";
}	

//////////////////////////// Cease ///////////////////////////////

string Cease::GetLoweredIdOrMacroName() const 
{ 
	return "cease"; 
}


string Cease::GetKeyword( Policy ) const
{
	return "cease";
}

