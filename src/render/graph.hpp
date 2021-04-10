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
 * SpecificIdentifier. All shared_ptr<>, Sequence<> and Collection<> members are listed below in a 
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
	void operator()( string region_id, const list<const Graphable *> &graphables ); // graph just the specified ojects
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node

private:
    struct MyBlock : Graphable::Block
    {
        string prerestriction_name;
        string colour;
        bool specify_ports;
        string base_id;
        bool italic_title;
    };

    void PopulateFromTransformation(Transformation *root);
    void PopulateFrom( const Graphable *g );
	void PopulateFromSubBlocks( const Graphable::Block &block );

	void GetMyBlocks();
    MyBlock PreProcessBlock( const Graphable::Block &block, 
                             const Graphable *g );
    
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

    string SeqField( int i );
    string EscapeForGraphviz( string s );
    void Disburse( string s );
    void Remember( string s );
    string LinkStyleAtt(Graphable::LinkStyle link_style);
    string GetFullId(const Graphable *g);

    const string outfile; // empty means stdout
    FILE *filep;
    list<const Graphable *> my_graphables;
    list<MyBlock> my_blocks;
    set<const Graphable *> reached;
    set<string> block_ids_show_prerestriction;
    static const LinkNamingFunction my_lnf;
    string region_id;
    string all_dot;
};

#endif
