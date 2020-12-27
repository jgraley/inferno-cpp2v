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
	using Transformation::operator();

    Graph( string of = string() );
    void operator()( Transformation *root ); // Graph the search/replace pattern
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node
    
private:
    string PopulateFromTransformation(Transformation *root);
    string PopulateFromEngine( const Graphable *g, string id, bool links_pass );
    string PopulateFromNode( TreePtr<Node> root, bool links_pass );

	string DoEngine( const Graphable *g, string id );
	string DoEngineLinks( const Graphable *e, string id );
    string DoNode( TreePtr<Node> n );
    string DoNodeLinks( TreePtr<Node> n, Graphable::LinkStyle link_style );
    string DoLink( int port_index, 
                   const Graphable::Block &block, 
                   const Graphable::SubBlock &sub_block, 
                   const Graphable::Link &link );
    Graphable::Block GetDefaultNodeBlockInfo( TreePtr<Node> n );
    string DoHTMLLabel( string name, const list<Graphable::SubBlock> &sub_blocks );
    string DoHeader();
    string DoFooter();

    string Name( TreePtr<Node> sp, bool *bold, string *shape );   // TODO put stringize capabilities into the Property nodes as virtual methods
    string Colour( TreePtr<Node> n );
    bool IsRecord( TreePtr<Node> n );
    Graphable *ShouldDoBlock( TreePtr<Node> node ); 
    string Id( const void *p );
    string SeqField( int i );
    string Sanitise( string s, bool remove_template=false );
    void Disburse( string s );
    string LinkStyleAtt(Graphable::LinkStyle link_style);
    string GetPreRestriction(TreePtr<Node> node, const TreePtrInterface *ptr);
    
    UniqueFilter unique_filter;
    const string outfile; // empty means stdout
};

#endif
