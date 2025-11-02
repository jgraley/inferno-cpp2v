#include "vn_parse.hpp"

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
#include "vn_lang.ypp.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.location.hpp"
#include "vn/agents/all.hpp"

#include <iostream>
#include <fstream>


using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

TreePtr<Node> VNParse::DoParse(string filepath)
{
    FILE *file = fopen(filepath.c_str(), "r");

    ASSERT(file != NULL)("Cannot open VN file: ")(filepath);

    YY::VNLangScanner scanner(file, std::cout);
    scanner.filename = filepath;    
    YY::VNLangParser parser(scanner, this);
    
    saw_error = false;
    script = nullptr;
    int pr = parser.parse();    
    ASSERT(pr==EXIT_SUCCESS);
    ASSERT(script);
    
    return script;
}


void VNParse::OnError()
{
	saw_error = true;
}


void VNParse::OnVNScript( Production script_ )
{
	if( saw_error )
		return;
		
	FTRACE("Got ")(script_)("\n");
	script = script_;
}

 
Production VNParse::OnEngine( Production stem )
{
	FTRACE("Got engine stem: ")(stem)("\n");
	auto pure_engine = MakeTreeNode<PureEngine>();
	pure_engine->stem = stem;
	return pure_engine;
}


Production VNParse::OnStuff( Production terminus )
{
	auto stuff = MakePatternNode<Stuff<Node>>();
	stuff->terminus = terminus;
	return stuff;
}


Production VNParse::OnDelta( Production through, Production overlay )
{
	auto delta = MakePatternNode<Delta<Node>>();
	delta->through = through;
	delta->overlay = overlay;
	return delta;
}


Production VNParse::OnPrefixOperator( string tok, Production operand )
{
    if( tok=="-" )
    {
		auto op = MakePatternNode<Negate>();
		op->operands = (operand);
		return op;
	}
	else
	{
		ASSERTFAIL(); // TODO use operator_data.inc
	}
}


Production VNParse::OnSpecificInteger( int value )
{
	return MakePatternNode<SpecificInteger>(value);
}
