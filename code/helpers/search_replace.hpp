#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"
#include "pass.hpp"

class SearchReplace : Pass
{  
public:
    SearchReplace( shared_ptr<Node> sp, shared_ptr<Node> rp=shared_ptr<Node>() ) :
        search_pattern( sp ),
        replace_pattern( rp )
    {  
        ASSERT( !!sp );
    }

    static bool IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern );
    GenericSharedPtr *Search( shared_ptr<Node> program );
    static shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x );
    void operator()( shared_ptr<Node> program );

    static void Test();

private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;

    void Replace( GenericSharedPtr *target );    
};

#endif
