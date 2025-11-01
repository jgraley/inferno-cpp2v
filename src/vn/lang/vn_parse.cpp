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

#include <iostream>
#include <fstream>


using namespace CPPTree; // TODO should not need
using namespace VN;
using namespace reflex;

void VNParse::DoParse(string filepath)
{
    FILE *file = fopen(filepath.c_str(), "r");

    ASSERT(file != NULL)("Cannot open VN file: ")(filepath);

    YY::VNLangScanner scanner(file, std::cout);
    scanner.filename = filepath;    
    YY::VNLangParser parser(scanner, this);
    int pr = parser.parse();
    
    // TODO provisional, until we can do something with the parsed pattern
    FTRACE("parse result: ")(pr)(", stopping\n");
    exit(pr);
}


Production VNParse::OnEngine( Production stem )
{
	FTRACE("Got engine stem:%d\n")(stem)("\n");
	return stem*10;
}
