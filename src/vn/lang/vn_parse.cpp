#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "helpers/walk.hpp"
#include "helpers/simple_duplicate.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "sort_decls.hpp"
#include "render.hpp"
#include "clang/Parse/DeclSpec.h"
#include "uniquify_identifiers.hpp"
#include "search_replace.hpp"
#include "agents/special_agent.hpp"
#include "vn_parse.hpp"
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include <iostream>
#include <fstream>


using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

void VN::Parse(string filepath)
{
	ifstream ifs; 
	ifs.open(filepath, ifstream::in);
	reflex::Input input(ifs);
	
	YY::VNLangScanner scanner( input, cerr );
	
	// I'm guessing this will invoke actions and then return a root symbol?
	YY::VNLangParser::symbol_type symbol0 = scanner.lex();
}
