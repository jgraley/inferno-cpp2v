
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"

class SearchReplace
{
    shared_ptr<Node> search;
    shared_ptr<Node> replace;

    shared_ptr<AnyString> CreateString( const char *s )
    {
        shared_ptr<String> st( new String );
        st->value = s;
        return st;
    }
    
public:
    static bool IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern );
    static shared_ptr<Node> Search( shared_ptr<Node> program, shared_ptr<Node> pattern );

    SearchReplace()
    {  
        shared_ptr<Program> pattern(new Program);  
    
        Parse p("prototype/search_replace/simple.cpp"); // TODO sort out paths   
        p( pattern );
        FOREACH( shared_ptr<Declaration> d, *pattern )
            if( shared_ptr<Record> sr = dynamic_pointer_cast<Record>(d) )
            {
            }
        ASSERT(search);
        ASSERT(replace);    
    }

    static void Test();
};
