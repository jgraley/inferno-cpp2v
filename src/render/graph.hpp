#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/scr_engine.hpp"

/**
 * Generate a GraphViz compatible graph description from a subtree (when used as a
 * Transformation) or a search/replace pattern set.
 *
 * Normal nodes and agents are represented as a rectangle with curved corners. At the top of the rectangle, 
 * in large font, is the name of the node's type OR the identifier name if the node is a kind of 
 * SpecificIdentifier. All TreePtr<>, Sequence<> and Collection<> members are listed below in a 
 * smaller font. The name of the pointed-to type is given (not the member's name, Inferno cannot deduce
 * this). 
 * 
 * Collections appear once and are followed by {...} where the number of dots equals the number of 
 * elements in the Collection.
 * 
 * Sequences appear once for each element in the sequence. Each appearance is followed by [i] where
 * i is the index, starting from 0.
 * 
 * All child pointers emerge from *approximately* the right of the corresponding member name. I cannot
 * for the life of me get GraphViz to make the lines begin *on* the right edge of the rectangle. They 
 * always come from some way in from the right edge, and if they are angled up or down, they can appear
 * to be coming from the wrong place.    
 * 
 * TODO put stringize capabilities into the Property nodes as virtual methods    
*/
class Graph : public OutOfPlaceTransformation
{
public:
	using Transformation::operator();

    Graph( string of = string() );
    ~Graph();
    void operator()( Transformation *root ); // Graph the search/replace pattern
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node

private:
    struct MyBlock : Graphable::Block
    {
        TreePtr<Node> as_node;
        string colour;
        bool specify_ports;
        string base_id;
        bool italic_title;
    };

    void PopulateFromTransformation(Transformation *root);
    void PopulateFromControl( const Graphable *g,
                              TreePtr<Node> nbase, 
                              Graphable::LinkStyle default_link_style );
    void PopulateFromNode( TreePtr<Node> root, 
                           Graphable::LinkStyle default_link_style );
    void PopulateFrom( TreePtr<Node> root, 
                       Graphable::LinkStyle default_link_style );
	void PopulateFromSubBlocks( const MyBlock &block, bool uniquify );

    MyBlock PreProcessBlock( const Graphable::Block &block, 
                             const Graphable *g,
                             TreePtr<Node> n, 
                             bool for_control_block );
    void PropagateLinkStyle( MyBlock &dest, Graphable::LinkStyle link_style );
    Graphable::Block GetNodeBlockInfo( TreePtr<Node> n );
    Graphable::Block GetDefaultNodeBlockInfo( TreePtr<Node> n );
    string GetInnermostTemplateParam( string s );
    string RemoveAllTemplateParam( string s );
    string RemoveOneOuterScope( string s );
    
    void PostProcessBlocks();

    string DoGraphBody();
    string DoBlock( const MyBlock &block );
    string DoRecordLabel( const MyBlock &block );
    string DoHTMLLabel( const MyBlock &block );
    string DoLinks( const MyBlock &block );
    string DoLink( int port_index, 
                   const MyBlock &block, 
                   const Graphable::SubBlock &sub_block, 
                   const Graphable::Link &link );
    string DoHeader();
    string DoFooter();

    Graphable *ShouldDoControlBlock( TreePtr<Node> node ); 
    string Id( const Graphable *g, TreePtr<Node> node );
    string SeqField( int i );
    string EscapeForGraphviz( string s );
    void Disburse( string s );
    string LinkStyleAtt(Graphable::LinkStyle link_style);
    bool IsNonTrivialPreRestriction(const TreePtrInterface *ptr);
    string GetPreRestrictionName(TreePtr<Node> node);

    const string outfile; // empty means stdout
    FILE *filep;
    
    list<MyBlock> my_blocks;
    set<TreePtr<Node>> reached;
    set<string> block_ids_show_prerestriction;
};

#endif
