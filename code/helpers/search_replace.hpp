#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"
#include "pass.hpp"

struct MatchSet : public set< shared_ptr<Node> > 
{
private: 
    // This is filled in by the search and replace engine
    mutable shared_ptr<Node> key;

    friend class SearchReplace;
};


class SearchReplace : Pass
{  
public:
    SearchReplace( shared_ptr<Node> sp=shared_ptr<Node>(), 
                   shared_ptr<Node> rp=shared_ptr<Node>(),
                   const set<MatchSet> *m = NULL );    
    ~SearchReplace();
    
    bool IsInteriorMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern ); // only look inside node (type, value)
    bool IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern ); // look at node and children
    GenericSharedPtr *Search( shared_ptr<Node> program );
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x );
    void operator()( shared_ptr<Node> program );

    static void Test();

private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
    const set<MatchSet> *matches;
    bool our_matches;
    
    void ClearPtrs( shared_ptr<Node> dest );
    void OverlayMembers( shared_ptr<Node> dest, shared_ptr<Node> source );
    void Replace( GenericSharedPtr *target );    
    const MatchSet *FindMatchSet( shared_ptr<Node> node );
    void ClearKeys(); 
};

/*
struct SoftSearchPattern : virtual Node
{
    virutal bool IsMatchPattern( SearchReplace *sr, shared_ptr<Node> x ) = 0; 
};


struct WildExpressonOfType : Expression
{
    NODE_FUNCTIONS;
    SharedPtr<Type> type_pattern;
    
    virutal bool IsMatchPattern( SearchReplace *sr, shared_ptr<Node> x )
    {
        if( shared_ptr<Expression> xe = dynamic_pointer_cast<Expression>(x) )
        {
            // Find out the type of the candidate expression
            shared_ptr<Type> xt = TypeOf().Get( program, xe );
            ASSERT(xt);
            
            // Punt it back into the search/replace engine
            return sr->IsMatchPattern( xt, type_pattern );
        }
        else
        {
            // not even an expression lol that aint going to match
            return false;
        }        
    }     
};
*/
#endif
