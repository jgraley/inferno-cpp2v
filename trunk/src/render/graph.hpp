#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/tree.hpp"
#include "helpers/transformation.hpp"

//
// Generate a GraphViz compatible graph description from a subtree (when used as a
// Transformation) or a search/replace pattern set.
//
class CompareReplace;
class Graph : public OutOfPlaceTransformation
{
public:
	using Transformation::operator();
    void operator()( Transformation *root ); // Graph the search/replace pattern
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node
private:
    string Header();
    string Footer();
    void Disburse( string s );
    string Traverse( TreePtr<Node> root, bool links_pass );
    string Traverse( Transformation *sr, bool links_pass );
	string DoTransformation( Transformation *sr, string id );
	string DoTransformationLinks( Transformation *sr, string id );
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
};

#endif
