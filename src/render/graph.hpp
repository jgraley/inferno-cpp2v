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
    struct MyBlock : Graphable::Block
    {
        bool specify_ports;
    };

    string PopulateFromTransformation(Transformation *root);
    string PopulateFromEngine( const Graphable *g, TreePtr<Node> nbase, string id, bool links_pass );
    string PopulateFromNode( TreePtr<Node> root, bool links_pass );

    MyBlock PreProcessBlock( const Graphable::Block &block, TreePtr<Node> n );
    Graphable::Block GetNodeBlockInfo( TreePtr<Node> n );
    Graphable::Block GetDefaultNodeBlockInfo( TreePtr<Node> n );
    
    string DoEngine( const MyBlock &block, string base_id );
    string DoNode( const MyBlock &block, string base_id );
    string DoLinks( const MyBlock &block, string base_id );
    string DoLink( int port_index, 
                   const MyBlock &block, 
                   const Graphable::SubBlock &sub_block, 
                   const Graphable::Link &link, 
                   string base_id );
    string DoHTMLLabel( string name, const list<Graphable::SubBlock> &sub_blocks );
    string DoHeader();
    string DoFooter();

    string Name( TreePtr<Node> sp, bool *bold, string *shape );   // TODO put stringize capabilities into the Property nodes as virtual methods
    string Colour( TreePtr<Node> n );
    bool IsRecord( TreePtr<Node> n );
    Graphable *ShouldDoEngine( TreePtr<Node> node ); 
    string Id( const void *p );
    string SeqField( int i );
    string Sanitise( string s, bool remove_template=false );
    void Disburse( string s );
    string LinkStyleAtt(Graphable::LinkStyle link_style);
    string GetPreRestriction(TreePtr<Node> node, const TreePtrInterface *ptr);
    void OverrideLinkStyle( MyBlock &dest, Graphable::LinkStyle link_style );
    
    UniqueFilter unique_filter;
    const string outfile; // empty means stdout
};

#endif
