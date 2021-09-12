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

	struct Figure
	{        
        struct Subordinate
        {
            string id;
            Graphable::LinkStyle link_style;
            string link_name;
        };
        struct Agent
        {
            map<string, Graphable::LinkStyle> link_styles;
        };
        string id;
        string title;
		map<Graphable *, Agent> interior_agents;
		map<Graphable *, Agent> exterior_agents;
		map<Graphable *, Subordinate> subordinates;
	};

    Graph( string of, string title );
    ~Graph();
    void operator()( Transformation *root ); // Graph the search/replace pattern
	void operator()( const Figure &figure ); // graph just the specified ojects
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root ); // graph the subtree under root node

private:
    struct MyBlock : Graphable::Block
    {
        string prerestriction_name;
        string colour;
        bool specify_ports;
        string base_id;
        bool italic_title;
        list< list<string> > link_ids; 
    };

    struct RegionAppearance
    {
		string region_id;
		string title;
		string background_colour;
	};

    void PopulateFromTransformation( list<const Graphable *> &graphables, Transformation *root );
    void PopulateFrom( list<const Graphable *> &graphables, const Graphable *g );
	void PopulateFromSubBlocks( list<const Graphable *> &graphables, const Graphable::Block &block );

    void RedirectLinks( list<MyBlock> &blocks_to_redirect, 
                        const Graphable *child_g,
                        string trace_label,
                        Graphable::LinkStyle target_link_style,
                        const MyBlock *target_block = nullptr );
	list<MyBlock> GetBlocks( list< const Graphable *> graphables,
	                         string figure_id,
                             const set<Graphable::LinkStyle> &link_styles_to_discard );
    MyBlock CreateInvisibleNode( string base_id, list<string> child_ids, string figure_id );
    MyBlock PreProcessBlock( Graphable::Block &block, 
                             const Graphable *g,
                             string figure_id,
                             const set<Graphable::LinkStyle> &link_styles_to_discard );
    
    void PostProcessBlocks( list<MyBlock> &blocks );

    string DoGraphBody( const list<MyBlock> &blocks,
                       const RegionAppearance &region );
    string DoBlock( const MyBlock &block,
                    const RegionAppearance &region );
    string DoRecordLabel( const MyBlock &block );
    string DoHTMLLabel( const MyBlock &block );
    string DoLinks( const MyBlock &block );
    string DoLink( int port_index, 
                   const MyBlock &block, 
                   const Graphable::SubBlock &sub_block, 
                   const Graphable::Link &link,
                   string id );
    string DoHeader( string title );
    string DoFooter();
    string DoCluster(string s, const RegionAppearance &region);

    string SeqField( int i );
    string EscapeForGraphviz( string s );
    void Disburse( string s );
    void Remember( string s );
    string LinkStyleAtt(Graphable::LinkStyle link_style);
    string GetFullId(const Graphable *g, string figure_id);
    string GetFullId(string id, string figure_id);
    string Indent(string s);

    const string outfile; // empty means stdout
    FILE *filep;
    set<const Graphable *> reached;
    set<string> block_ids_show_prerestriction;
    static const LinkNamingFunction my_lnf;
    const RegionAppearance base_region;
	const string line_colour;
	const string font_colour;
	list<MyBlock> blocks_for_links;
    string all_dot;
};

#endif
