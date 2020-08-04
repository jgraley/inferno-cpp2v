#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/scr_engine.hpp"

//
// Generate a GraphViz compatible graph description from a subtree (when used as a
// Transformation) or a search/replace pattern set.
//
class Graph : public OutOfPlaceTransformation
{
public:
    Graph( string of = string() );
	using Transformation::operator();
    void operator()( Transformation *root ); // Graph the search/replace pattern
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node
private:
    string Header();
    string Footer();
    string MakeGraphTx(Transformation *root);
    void Disburse( string s );
    string UniqueWalk( TreePtr<Node> root, bool links_pass );
    string UniqueWalk( SR::SCREngine *e, string id, bool links_pass );
	string DoEngine( SR::SCREngine *e, string id );
	string DoEngineLinks( SR::SCREngine *e, string id );
    string Id( void *p );
    string SeqField( int i, int j=0 );
    string Sanitise( string s, bool remove_tp=false );
    string Name( TreePtr<Node> sp, bool *bold, string *shape );   // TODO put stringize capabilities into the Property nodes as virtual methods
    string Colour( TreePtr<Node> n );
    string HTMLLabel( string name, TreePtr<Node> n );
    string SimpleLabel( string name, TreePtr<Node> n );
    string DoNode( TreePtr<Node> n );
    string DoNodeLinks( TreePtr<Node> n );
    bool IsRecord( TreePtr<Node> n );
    string DoLink( TreePtr<Node> from, string field, TreePtr<Node> to, string atts=string(), const TreePtrInterface *ptr=NULL );
    UniqueFilter unique_filter;
    const string outfile; // empty means stdout
};

#endif
