#include "sctree.hpp"
#include "node/syntax.hpp"
#include "node/specialise_oostd.hpp"
//#include "clang/Parse/DeclSpec.h"
#include "tree/type_data.hpp"
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
/*
Syntax::Production Wait::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}
*/

string Wait::GetLoweredIdOrMacroName() const 
{ 
	return "wait"; 
}

/*
YY::VNLangParser::token::token_kind_type Wait::GetSignifierToken() const
{
	return YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT;
}
*/
//////////////////////////// WaitDynamic ///////////////////////////////

/*
string WaitDynamic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}

string WaitDynamic::GetKeyword( Policy ) const 
{
	return "wait_dynamic"; 
}*/

//////////////////////////// WaitStatic ///////////////////////////////
/*
string WaitStatic::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string WaitStatic::GetKeyword( Policy ) const 
{
	return "wait_static"; // TODO just be "wait" and evolve into WaitDynamic in OnArgsList()
}
*/
//////////////////////////// WaitDelta ///////////////////////////////

/*string WaitDelta::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->GetKeyword(this, policy);
}


string WaitDelta::GetKeyword( Policy ) const 
{
	return "wait_delta"; // Keep as wait_delta to differentiate from WaitStatic which is just wait with no args
}
*/
//////////////////////////// NextTrigger ///////////////////////////////

string NextTrigger::GetLoweredIdOrMacroName() const 
{ 
	return "next_trigger"; 
}

//////////////////////////// Notify ///////////////////////////////

string Notify::GetLoweredIdOrMacroName() const 
{ 
	return "notify"; 
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

