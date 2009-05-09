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
    
    void operator()( shared_ptr<Program> p );

    bool IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern ); // look at node and children
    shared_ptr<Program> GetProgram() const { ASSERT(program); return program; } 
    struct SoftSearchPattern : virtual Node
    {
        virtual bool IsMatchPattern( SearchReplace *sr, shared_ptr<Node> x ) = 0; 
    };

    static void Test();

private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
    const set<MatchSet> *matches;
    bool our_matches;
    shared_ptr<Program> program;
    
    bool IsMatchPatternLocal( shared_ptr<Node> x, shared_ptr<Node> pattern ); // only look inside node (type, value)
    bool IsMatchPatternNoKey( shared_ptr<Node> x, shared_ptr<Node> pattern ); // look at node and children
    GenericSharedPtr *Search( shared_ptr<Node> program );
    void ClearPtrs( shared_ptr<Node> dest );
    void OverlayMembers( shared_ptr<Node> dest, shared_ptr<Node> source, bool in_substitution );
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x, bool in_substitution=false );
    void Replace( GenericSharedPtr *target );    
    const MatchSet *FindMatchSet( shared_ptr<Node> node );
    void ClearKeys(); 
};



#endif
