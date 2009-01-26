
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"

class SearchReplace
{
    shared_ptr<Declaration> search;
    shared_ptr<Declaration> replace;

public:
    SearchReplace()
    {  
        shared_ptr<Program> pattern(new Program);  
    
        Parse p("prototype/search_replace/simple.cpp"); // TODO sort out paths   
        p( pattern );
        FOREACH( shared_ptr<Declaration> d, *pattern )
            if( shared_ptr<Record> sr = dynamic_pointer_cast<Record>(d) )
            {
                if( sr->name == string("SEARCH") )
                    search = sr->members[0];
                else if( sr->name == string("REPLACE") )
                    replace = sr->members[0];
                else
                    ASSERT( !"Only \"struct SEARCH\" or \"struct REPLACE\" at top level");
                ASSERT( sr->members.size()==1 && "only one item in SEARCH and REPLACE structs");           
            }
        ASSERT(search);
        ASSERT(replace);    
    }
};
