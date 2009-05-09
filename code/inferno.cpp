
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/search_replace.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/validate.hpp"

void SelfTest();

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    if( ReadArgs::selftest )
        SelfTest();

    if( ReadArgs::infile.empty() )
        return 0;

    shared_ptr<Program> program(new Program);  

    Parse p(ReadArgs::infile);        
    p( program );
              
    Validate()(program);                
                    
    {
        shared_ptr<Dereference> sd( new Dereference );     
        shared_ptr<Add> sa( new Add );
        sd->operands.push_back( sa );
        shared_ptr<SoftExpressonOfType> sseot( new SoftExpressonOfType );
        sa->operands.push_back( sseot );
        shared_ptr<Array> sar( new Array );
        sseot->type_pattern = sar;        
        shared_ptr<Expression> se( new Expression );
        sa->operands.push_back( se );
    
        shared_ptr<Subscript> rs( new Subscript );
        shared_ptr<Expression> rar( new Expression );
        rs->base = rar;
        shared_ptr<Expression> re( new Expression );
        rs->index = re;
                       
        MatchSet mar;
        mar.insert( sseot );
        mar.insert( rar );
        MatchSet me;
        me.insert( se );
        me.insert( re );            
        set<MatchSet> sm;
        sm.insert( mar );
        sm.insert( me );
                       
        SearchReplace(sd, rs, &sm)(program);                            
    }
                 
    Validate()(program);                

    if(ReadArgs::graph)
    {
        Graph g;
        g( program );
    }
    else
    {
        Render r;
        r( program );
    }    
    
    return 0;
}

void SelfTest()
{
    SearchReplace::Test();
}
