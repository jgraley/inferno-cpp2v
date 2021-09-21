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

    enum LinkPlannedAs
    {
        LINK_NORMAL,
        LINK_KEYER,
        LINK_RESIDUAL,
        LINK_ABNORMAL,
        LINK_EVALUATOR,
        LINK_MULTIPLICITY        
    };

    struct Region
    {
        string id;
        string title;        
    };
    
	struct Figure : Region
	{        
        struct Subordinate
        {
            const Graphable *root_g;
            string root_link_short_name;
            LinkPlannedAs root_link_planned_as;
        };
        struct Agent
        {
            map<string, LinkPlannedAs> links_planned_as;
        };
		map<Graphable *, Agent> interior_agents;
		map<Graphable *, Agent> exterior_agents;
        map<GraphIdable *, Subordinate> subordinates;
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
    };

    struct MyLink : Graphable::Link
    {
        MyLink( shared_ptr<const Graphable::Link> link,
                string child_id_,
                LinkPlannedAs planned_as_ ) : 
            Graphable::Link( *link ),
            child_id( child_id_ ),
            planned_as( planned_as_ )
        {
        }

        virtual ~MyLink() = default;
        
        string child_id;
        LinkPlannedAs planned_as;
    };

    struct RegionAppearance : Region
    {
        RegionAppearance( string bg ) : background_colour(bg) {}
		string background_colour;
	};

    void PopulateFromTransformation( list<const Graphable *> &graphables, Transformation *root );
    void PopulateFrom( list<const Graphable *> &graphables, const Graphable *g );
	void PopulateFromSubBlocks( list<const Graphable *> &graphables, const Graphable::Block &block );

    void RedirectLinks( list<MyBlock> &blocks_to_act_on, 
                        const Graphable *target_child_g,
                        string trace_label,
                        const MyBlock *target_block = nullptr );
    void UpdateLinksPlannedAs( list<MyBlock> &blocks_to_act_on, 
                               const Graphable *target_child_g,
                               string trace_label,
                               LinkPlannedAs target_link_planned_as );
    void CheckLinks( list<MyBlock> blocks );
	list<MyBlock> GetBlocks( list< const Graphable *> graphables,
	                         list< const Graphable *> all_graphables,
	                         const Region *region,
                             bool hide_replace_only );
    MyBlock CreateInvisibleNode( string base_id, 
                                 list<const Graphable *> children, 
                                 const Region *region );
    MyBlock PreProcessBlock( const Graphable::Block &block, 
                             const Graphable *g,
                             const Region *region );
    
    void PostProcessBlocks( list<MyBlock> &blocks );
    void PostProcessBlock( MyBlock &block );

    string DoBlocks( const list<MyBlock> &blocks,
                     const RegionAppearance &region );
    string DoBlock( const MyBlock &block,
                    const RegionAppearance &region );
    string DoRecordLabel( const MyBlock &block );
    string DoHTMLLabel( const MyBlock &block );
    string DoLinks( const list<MyBlock> &blocks );
    string DoLinks( const MyBlock &block );
    string DoLink( int port_index, 
                   const MyBlock &block, 
                   shared_ptr<const Graphable::Link> link );
    string DoHeader( string title );
    string DoFooter();
    string DoRegion(string s, const RegionAppearance &region);

    string SeqField( int i );
    string EscapeForGraphviz( string s );
    void Disburse( string s );
    void Remember( string s );
    string LinkStyleAtt(LinkPlannedAs root_link_planned_as, Graphable::Phase phase);
    string GetRegionGraphId(const Region *region, const GraphIdable *g);
    string GetRegionGraphId(const Region *region, string id);
    string Indent(string s);

    const string outfile; // empty means stdout
    FILE *filep;
    set<const Graphable *> reached;
    set<string> block_ids_show_prerestriction;
    static const LinkNamingFunction my_lnf;
    const RegionAppearance base_region;
	const string line_colour;
	const string font_colour;
    string all_dot;
};

#endif
