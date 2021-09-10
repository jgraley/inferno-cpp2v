/*
 * graph.cpp
 *
 *  Created on: 18 Jul 2010
 *      Author: jgraley
 */

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/search_replace.hpp"
#include "sr/scr_engine.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "graph.hpp"
#include "steps/inferno_agents.hpp"
#include <inttypes.h>
#include "node/graphable.hpp"
#include "sr/link.hpp"
#include "sr/agents/standard_agent.hpp" // temporary, I hope

using namespace CPPTree;

// Graph Documentation
//
// The shapes and contents of the displayed nodes is explained in comments in the function 
// Graph::Name(), which may be found below. Ordinary tree nodes are always rectangles with
// soft corners, so this function mainly deals with special nodes as used by Search and Replace.
// The colours are defined by the nodes themselves and always reflect the node type of a
// block, if applicable.

#define FS_MIDDLE "12"
#define FS_LARGE "15"
#define FS_HUGE "20"
#define FS_TITLE "36"

#define NS_SMALL "0.4"
#define FONT "Arial"

#include <functional>

Graph::Graph( string of, string title ) :
    outfile(of),
    base_region { "",
                  "",
		          ReadArgs::graph_dark ? "black" : "antiquewhite1" },
	line_colour( ReadArgs::graph_dark ? "grey70" : "black" ),
    font_colour( ReadArgs::graph_dark ? "white" : "black" )
{
	if( !outfile.empty() )
	{
		filep = fopen( outfile.c_str(), "wt" );
		ASSERT( filep )( "Cannot open output file \"%s\"", outfile.c_str() );
    }
    
	Disburse( DoHeader(title) );
}


Graph::~Graph()
{
	Disburse( all_dot );
	Disburse( DoFooter() );

    if( !outfile.empty() )
    {
        fclose( filep );
    }
}


void Graph::operator()( Transformation *root )
{
    string s;
    s += "// -------------------- transformation figure --------------------\n";
    list<const Graphable *> my_graphables;
    list<MyBlock> my_blocks;

	reached.clear();
    PopulateFromTransformation(my_graphables, root);
    my_blocks = GetBlocks( my_graphables, "", {} );
    PostProcessBlocks(my_blocks);
    s += DoGraphBody(my_blocks, base_region);
    s += "\n";
	Remember(s);
}


void Graph::operator()( const Figure &figure )
{        
    // First we will get blocks, pre-and pos-process them and redirect links to subordinate engines
    list<const Graphable *> interior_gs, exterior_gs;
    map< const Figure::Subordinate *, list<MyBlock> > subordinate_blocks;

    for( const Figure::GraphableAndIncomingLinks &lb : figure.exteriors )
        exterior_gs.push_back( lb.graphable );
    for( const Figure::GraphableAndIncomingLinks &lb : figure.interiors )
        interior_gs.push_back( lb.graphable );

    list<MyBlock> exterior_blocks = GetBlocks( exterior_gs, figure.id, {Graphable::LINK_NORMAL, Graphable::LINK_ROOT, Graphable::LINK_ONLY_REPLACE} );
    list<MyBlock> interior_blocks = GetBlocks( interior_gs, figure.id, {Graphable::LINK_ONLY_REPLACE, Graphable::LINK_ROOT} );
    for( const Figure::Subordinate &sub : figure.subordinates )
    {
        string sub_figure_id = figure.id+" / "+sub.link_name;
        subordinate_blocks[&sub].push_back( CreateInvisibleNode( sub.root->GetGraphId(), {}, sub_figure_id ) );
    }
    
    if( interior_blocks.empty() )
        interior_blocks.push_back( CreateInvisibleNode( "engine", { exterior_gs.front()->GetGraphId() }, figure.id ) );
    
    // Note: ALL redirections apply to interior nodes, because these are the only ones with outgoing links.
    for( const Figure::GraphableAndIncomingLinks &lb : figure.interiors )
        for( pair<string, Graphable::LinkStyle> p : lb.link_styles )
            RedirectLinks( interior_blocks, lb.graphable, p.first, p.second );
    for( const Figure::GraphableAndIncomingLinks &lb : figure.exteriors )
        for( pair<string, Graphable::LinkStyle> p : lb.link_styles )
            RedirectLinks( interior_blocks, lb.graphable, p.first, p.second );
    for( const Figure::Subordinate &sub : figure.subordinates )
        RedirectLinks( interior_blocks, sub.root, sub.link_name, sub.link_style, &(subordinate_blocks[&sub].front()) );

    PostProcessBlocks(exterior_blocks);
    PostProcessBlocks(interior_blocks);
    for( const Figure::Subordinate &sub : figure.subordinates )
		PostProcessBlocks(subordinate_blocks[&sub]);	
    
    // Now generate all the dot code
    string s;
    s += "// -------------------- figure "+figure.id+" --------------------\n";
	s += DoGraphBody(exterior_blocks, base_region); // Exterior blocks
    RegionAppearance interior_region = base_region;
    interior_region.title = figure.title;
    interior_region.region_id = figure.id;
    interior_region.background_colour = ReadArgs::graph_dark ? "gray15" : "antiquewhite2";
    string s_interior = DoGraphBody(interior_blocks, interior_region); // Interior blocks

    for( const Figure::Subordinate &sub : figure.subordinates )
    {
		RegionAppearance subordinate_region = interior_region;
		subordinate_region.title = sub.id;
		subordinate_region.region_id += " / "+sub.id;
		subordinate_region.background_colour = ReadArgs::graph_dark ? "gray25" : "antiquewhite3";

        list<MyBlock> sub_blocks = subordinate_blocks.at(&sub);        
	    string s_subordinate = DoGraphBody(sub_blocks, subordinate_region); // Subordinate blocks
		s_interior += DoCluster(s_subordinate, subordinate_region);
	}

    s += DoCluster(s_interior, interior_region);
    s += "\n";

	Remember( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
    string s;
    s += "// -------------------- node figure --------------------\n";
    list<const Graphable *> my_graphables;
    list<MyBlock> my_blocks;
	(void)context; // Not needed!!

    reached.clear();
    Graphable *g = dynamic_cast<Graphable *>(root.get());
	PopulateFrom( my_graphables, g );
	my_blocks = GetBlocks( my_graphables, "", {} );
    PostProcessBlocks(my_blocks);
    s += DoGraphBody(my_blocks, base_region);
    s += "\n";
	Remember( s );

	return root; // no change
}


void Graph::PopulateFromTransformation( list<const Graphable *> &graphables, Transformation *root )
{    
    if( TransformationVector *tv = dynamic_cast<TransformationVector *>(root) )
    {
        FOREACH( shared_ptr<Transformation> t, *tv ) // TODO loop backwards so they come out in the right order in graph
            PopulateFromTransformation( graphables, t.get() );
    }
    else if( CompareReplace *cr = dynamic_cast<CompareReplace *>(root) )
    {
		reached.clear();
		PopulateFrom( graphables, cr );
	}
	else
    {
        ASSERTFAIL("Unknown kind of transformation in graph plotter");
    }
}

                   
void Graph::PopulateFrom( list<const Graphable *> &graphables, const Graphable *g )
{
	ASSERT(g);
    graphables.push_back(g);

    Graphable::Block block = g->GetGraphBlockInfo(my_lnf, nullptr);	        
    PopulateFromSubBlocks( graphables, block );
}


void Graph::PopulateFromSubBlocks( list<const Graphable *> &graphables, const Graphable::Block &block )
{
	for( const Graphable::SubBlock &sub_block : block.sub_blocks )
	{
		for( const Graphable::Link &link : sub_block.links )
		{
			if( link.child && reached.count(link.child)==0 )
			{
				PopulateFrom( graphables, link.child );
				reached.insert( link.child );
			}
		}
	}
}


void Graph::RedirectLinks( list<MyBlock> &blocks_to_redirect, 
                           const Graphable *child_g,
                           string trace_label,
                           Graphable::LinkStyle target_link_style,
                           const MyBlock *target_block )
{
    bool hit = false;
    TRACE("RedirectLinks()\n");
    // Loop over all the links in all the blocks that we might need to 
    // redirect (ones in the interior of the figure)
	for( MyBlock &block : blocks_to_redirect )
	{
        TRACEC("    Block: ")(block.base_id)("\n");
        auto sbidit = block.link_ids.begin();
        for( Graphable::SubBlock &sub_block : block.sub_blocks )
        {
            ASSERT(sbidit != block.link_ids.end() );
            auto lidit = sbidit->begin();
            for( Graphable::Link &link : sub_block.links )
            {
                ASSERT( lidit != sbidit->end() );
                string id = *lidit;
                TRACEC("        Link: ")(id)(": ")(link.labels)(link.trace_labels)("\n");
                // Two things must be true for us to redirect this link toward the target block:
                // - Link must point to the right agent - that being the root agent of the sub-engine
                // - The link label (sattelite serial number of the PatternLink) must match the one supplied to us for the sub-engine
                // AndRuleEngine knows link labels for sub-engines. These two criteria ensure we have got the right link. 
                if( link.child == child_g )
                {
                    ASSERT( link.trace_labels.size()==1 ); // brittle
                    if( link.trace_labels.front() == trace_label )
                    {
                        if( target_block )
                            *lidit = target_block->base_id;     
                        link.style = target_link_style;            
                        hit = true;
                    }
                }
                
                lidit++;
            }
            sbidit++;
        }
    }
    //ASSERT( hit );
}                           


list<Graph::MyBlock> Graph::GetBlocks( list< const Graphable *> graphables,
                                       string figure_id,
                                       const set<Graphable::LinkStyle> &discard_links )
{
	list<MyBlock> blocks;
	
	for( const Graphable *g : graphables )
	{
		Graphable::Block gblock = g->GetGraphBlockInfo(my_lnf, nullptr);
        MyBlock block = PreProcessBlock( gblock, g, figure_id, discard_links );
        blocks.push_back( block );
	}

	return blocks;
}


Graph::MyBlock Graph::CreateInvisibleNode( string id, list<string> child_ids, string figure_id )
{
	MyBlock block;
	block.title = "";     
	block.bold = false;
	block.shape = "none";
    block.block_type = Graphable::NODE;
    block.link_ids.push_back( {} );
    block.prerestriction_name = "";
    block.colour = "";
    block.specify_ports = false;
    block.base_id = GetFullId(id, figure_id);
    block.italic_title = false;
    
    Graphable::SubBlock sub_block = { "", 
                                      "",
                                      false,
                                      {} };
    for( string child_id : child_ids )
    {
        Graphable::Link link;
        link.child = nullptr;
        link.style = Graphable::LINK_NORMAL;                
        //link.trace_labels.push_back( lnf( ... ) ); TODO
        //link.is_ntpr = ntprf ? ntprf(&p) : false;
        sub_block.links.push_back( link );
        block.link_ids.back().push_back( GetFullId(child_id, figure_id) );
    }
    block.sub_blocks.push_back( sub_block );
		    
                
    return block;
}


Graph::MyBlock Graph::PreProcessBlock( const Graphable::Block &block, 
                                       const Graphable *g,
                                       string figure_id,
                                       const set<Graphable::LinkStyle> &discard_links )
{
	ASSERT(g);
	const Node *pnode = dynamic_cast<const Node *>(g);
    const SpecialBase *pspecial = pnode ? dynamic_cast<const SpecialBase *>(pnode) : nullptr;

    // Fill in everything in block 
    MyBlock my_block;
    (Graphable::Block &)my_block = block;
    
    // Fill in the GraphViz ID that the block will use
    my_block.base_id = GetFullId(g, figure_id);
    
    // Capture the node (if there is one: might be NULL)
	if( pspecial )
		my_block.prerestriction_name = (**(pspecial->GetPreRestrictionArchitype())).GetName();
    
    // In graph trace mode, nodes get their serial number added in as an extra sub-block (with no links)
    if( ReadArgs::graph_trace && pnode )
    {
        my_block.sub_blocks.push_back( { pnode->GetSerialString(), 
                                         "", 
                                         false, 
                                         {} } );
    }  

    // Make the titles more wieldy by removing template stuff - note:
    // different policies for control blocks vs node blocks.
    switch( block.block_type )
    {
	case Graphable::CONTROL:
        my_block.title = RemoveAllTemplateParam(my_block.title); 
        my_block.title = RemoveOneOuterScope(my_block.title); 
        my_block.shape = "record";
        my_block.colour = "transparent";
        break;

    case Graphable::NODE:
        my_block.title = GetInnermostTemplateParam(my_block.title);
        // See #258: "block colour shall be dictated by the node type only"
        my_block.colour = pnode->GetColour();
        break;

    default:
		ASSERTFAIL("Unknown block type");
		break;
	}
	
    // Actions for sub-blocks
    my_block.link_ids.clear();
    for( Graphable::SubBlock &sub_block : my_block.sub_blocks )
    {			
        // Actions for links
        list<Graphable::Link> new_links;
        list<string> new_link_ids;
        for( Graphable::Link &link : sub_block.links )
        {
			if( discard_links.count( link.style ) )
				continue;
			
			ASSERT( link.child )(block.title)(" ")(sub_block.item_name);
			string id = GetFullId(link.child, figure_id);			
			
            // Detect pre-restrictions and add to link labels
            if( link.is_ntpr )
            {
                block_ids_show_prerestriction.insert( id );
            }

			new_links.push_back( link );
            new_link_ids.push_back( id );
        }
        sub_block.links = new_links;
        my_block.link_ids.push_back( new_link_ids );
    }
    
    // Italic title OR symbol designates a special agent 
    my_block.italic_title = (bool)pspecial;
    if( !pspecial )
        ASSERT( my_block.symbol.empty() );

    return my_block;    
}


void Graph::PostProcessBlocks( list<MyBlock> &blocks )
{
    for( MyBlock &block : blocks )
    {
        if( block_ids_show_prerestriction.count( block.base_id ) > 0 )
        {          
            // Note: using push_front to get pre-restriction near the top (under title)
            block.sub_blocks.push_front( { "("+block.prerestriction_name+")", 
                                           "", 
                                           false, 
                                           {} } );
            block.link_ids.push_front( list<string>() );
        }
        
        // Can we hide sub-blocks?
        bool sub_blocks_hideable = ( block.sub_blocks.size() == 0 || 
                                     (block.sub_blocks.size() == 1 && block.sub_blocks.front().hideable) );

        // If not, make sure we're using a shape that allows for sub_blocks
        if( !sub_blocks_hideable && !(block.shape == "none" || block.shape == "plaintext" || block.shape == "record") )
            block.shape = "plaintext";      
        
        // These kinds of blocks require port names to be to be specified so links can tell them apart
        block.specify_ports = (block.shape=="record" || block.shape=="plaintext");              
    }
}


string Graph::DoGraphBody( const list<MyBlock> &blocks, const RegionAppearance &region )
{
    string s;
    
    for( const MyBlock &block : blocks )
        s += DoBlock(block, region);
    
    return s;
}


string Graph::DoBlock( const MyBlock &block, const RegionAppearance &region )
{
    blocks_for_links.push_back(block);

	string s;
	s += "shape = \"" + block.shape + "\"\n";
	if( block.colour=="transparent" )
		s += "fillcolor = " + region.background_colour + "\n";
	else if(block.colour != "")
		s += "fillcolor = \"" + block.colour + "\"\n";

    // shape=plaintext triggers HTML label generation. From Graphviz docs:
    // "Adding HTML labels to record-based shapes (record and Mrecord) is 
    // discouraged and may lead to unexpected behavior because of their 
    // conflicting label schemas and overlapping functionality."
    // https://www.youtube.com/watch?v=Tv1kRqzg0AQ
	if( block.shape == "plaintext" )
	{
		s += "label = " + DoHTMLLabel( block );
		s += "style = \"rounded,filled\"\n";
		s += "fontsize = \"" FS_MIDDLE "\"\n";
	}
	else if( block.shape == "record" )
    {
        s += "label = " + DoRecordLabel( block );
        s += "style = \"filled\"\n";
        s += "fontsize = \"" FS_MIDDLE "\"\n";
        s += "color = " + line_colour + "\n";
        s += "fontcolor = " + font_colour + "\n";
    }
	else if( block.shape == "none" )
    {
        s += "style = \"invisible\"\n";
    }
    else
	{
        string lt;
        if( !block.symbol.empty() )
            lt = EscapeForGraphviz( block.symbol );
        else if( block.italic_title )        
            lt = "<I>" + EscapeForGraphviz( block.title ) + "</I>";
        else
            lt = EscapeForGraphviz( block.title );        // Ignoring sub-block (above check means there will only be one: it
        // is assumed that the title is sufficietly informative
		s += "label = <" + lt + ">\n";// TODO causes errors because links go to targets meant for records
		s += "style = \"filled\"\n";
        if( !block.symbol.empty() )
            s += "fontsize = \"" FS_HUGE "\"\n";
        else
            s += "fontsize = \"" FS_LARGE "\"\n";
        s += "penwidth = 0.0\n";

		if( !block.symbol.empty() )
		{
			s += "fixedsize = true\n";
			s += "width = " NS_SMALL "\n";
			s += "height = " NS_SMALL "\n";
		}
	}
	string sc;
    sc += "\""+block.base_id+"\"";
	sc += " [\n";
    sc += Indent(s);
	sc += "];\n";
    sc += "\n";
	return sc;
}


string Graph::DoRecordLabel( const MyBlock &block )
{
    string lt = EscapeForGraphviz( block.symbol.empty() ? block.title : block.symbol );
    string s;
    s += "\"<fixed> " + lt;
    int k=0;
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {        
        string label_text = EscapeForGraphviz(sub_block.item_name + sub_block.item_extra);
        s += " | <" +  SeqField(k) + "> " + label_text;
        k++;
    }
    s += "\"\n";
    return s;
}


string Graph::DoHTMLLabel( const MyBlock &block )
{
    string lt;
    if( !block.symbol.empty() )
        lt = EscapeForGraphviz( block.symbol );
    else if( block.italic_title )        
        lt = "<I>" + EscapeForGraphviz( block.title ) + "</I>";
    else
        lt = EscapeForGraphviz( block.title );
        
    
	string s = "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n";
	s += " <TR>";
	s += "<TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + lt + "</FONT></TD>";
	s += "<TD></TD>";
	s += "</TR>\n";
    
    int porti=0;
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        s += " <TR>";
        s += "<TD>" + EscapeForGraphviz(sub_block.item_name) + "</TD>";
        s += "<TD PORT=\"" + SeqField( porti ) + "\">" + EscapeForGraphviz(sub_block.item_extra) + "</TD>";
        s += "</TR>\n";
        porti++;
    }
    
	s += "</TABLE>>\n";
	return s;
}


string Graph::DoLinks( const MyBlock &block )
{
    string s;
    
    int porti=0;
    s += "// links for block "+block.base_id+"\n";
    auto sbidit = block.link_ids.begin();
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
		ASSERT(sbidit != block.link_ids.end() );
		auto lidit = sbidit->begin();
        for( Graphable::Link link : sub_block.links )
        {
			ASSERT( lidit != sbidit->end() );
			s += DoLink( porti, block, sub_block, link, *lidit );
			lidit++;
		}
        porti++;
        sbidit++;
    }

	return s;
}


string Graph::DoLink( int port_index, 
                      const MyBlock &block, 
                      const Graphable::SubBlock &sub_block, 
                      const Graphable::Link &link,
                      string id )
{          
    // Atts
    string atts;
    atts += LinkStyleAtt(link.style);

    // Labels
    list<string> labels;
    if( ReadArgs::graph_trace )
        labels = link.labels + link.trace_labels;
    else
        labels = link.labels;
    if( !labels.empty() )
    {
        atts += "label = \""+EscapeForGraphviz(Join(labels))+"\"\n"; 
        atts += "decorate = true\n";
    }

    atts += "color = " + line_colour + "\n";
    atts += "fontcolor = " + font_colour + "\n";

    // GraphViz output
	string s;
	s += "\""+block.base_id+"\"";
    if( block.specify_ports )
        s += ":" + SeqField(port_index);
	s += " -> ";
	s += "\""+id+"\"";
	s += " [\n"+Indent(atts)+"];\n";
    s += "\n";
	return s;
}


string Graph::DoHeader(string title)
{
	string sg, sn, se;
	sg += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
	sg += "ranksep = 0.3\n"; // separation parent-child (default 0.5)
	//sg += "size = \"14,20\"\n"; // make it smaller
  //  sg += "concentrate = \"true\"\n"; 
    sg += "bgcolor = " + base_region.background_colour + "\n";
    sg += "color = " + line_colour + "\n";
    sg += "fontcolor = " + font_colour + "\n";    
#ifdef FONT
    sg += "fontname = \"" FONT "\"\n";
    sn += "fontname = \"" FONT "\"\n";
    se += "fontname = \"" FONT "\"\n";
#endif    
    
    string s;
	s += "digraph \""+title+"\" {\n"; 
    s += "label = \""+title+"\"\n";
    s += "labelloc = t\n";
    s += "fontsize = \"" FS_TITLE "\"\n";
	s += "graph [\n";
    s += Indent(sg);
    s += "];\n";
	s += "node [\n";
    s += Indent(sn);
	s += "];\n";
	s += "edge [\n";
    s += Indent(se);
	s += "];\n";
    s += "\n";
	return s;
}


string Graph::DoFooter()
{
	string s;

    s += "// -------------------- links --------------------\n";
    
    for( const MyBlock &block : blocks_for_links )
        s += DoLinks(block);
        	
	s += "}\n";
	return s;
}


string Graph::DoCluster(string ss, const RegionAppearance &region)
{
    string s;
    s += "label = \"" + region.title + "\"\n";
    s += "style = \"filled\"\n";
	s += "color = " + region.background_colour + "\n";
    s += "fontsize = \"" FS_LARGE "\"\n";
	s += ss;
 
    string sc;
    sc += "subgraph \"cluster" + region.region_id + "\" {\n";
    sc += Indent(s);
    sc += "}\n";
    sc += "\n";
    return sc;
}


string Graph::SeqField( int i )
{
	char s[20];
	sprintf( s, "port%d", i );
	return s;
}


string Graph::EscapeForGraphviz( string s )
{
    string o;
	for( int i=0; i<s.size(); i++ )
	{
		if( s[i] == '\"' )
		{
			o += "\\\"";
		}
		else if( s[i] == '<' )
		{
			o += "&lt;";
		}
		else if( s[i] == '>' )
		{
			o += "&gt;";
		}
		else if( s[i] == '&' )
		{
			o += "&amp;";
		}
		else
		{
			o += s[i];
		}
	}
	return o;
}


void Graph::Disburse( string s )
{
	if( outfile.empty() )
	{
		puts( s.c_str() );
	}
	else
	{
		fputs( s.c_str(), filep );
	}
}


void Graph::Remember( string s )
{
	all_dot = s + all_dot; // dot seems to reverse the graphs
}


string Graph::LinkStyleAtt(Graphable::LinkStyle link_style)
{
    string atts;
    switch(link_style)
    {
    case Graphable::LINK_NORMAL:
    case Graphable::LINK_ROOT:
        break;
    case Graphable::LINK_KEYER:
        atts += "arrowhead=\"normalnonebox\"\n";
        break;
    case Graphable::LINK_RESIDUAL:
        atts += "arrowhead=\"normalnonetee\"\n";
        break;
    case Graphable::LINK_ONLY_REPLACE:
        atts += "style=\"dashed\"\n";
        break;
    case Graphable::LINK_ABNORMAL:
        atts += "arrowhead=\"normalnoneodot\"\n";
        break;
    case Graphable::LINK_EVALUATOR:
        atts += "arrowhead=\"normalnoneodiamond\"\n";
        break;
    case Graphable::LINK_MULTIPLICITY:
        atts += "arrowhead=\"normalonormalonormalonormal\"\n";
        break;
    }
    return atts;
}


string Graph::GetFullId(const Graphable *g, string figure_id)
{
	return GetFullId(g->GetGraphId(), figure_id);
}


string Graph::GetFullId(string id, string figure_id)
{
	if(figure_id.empty() )
		return id;
	else
		return figure_id+" / "+id;
}


const Graph::LinkNamingFunction Graph::my_lnf = []( const TreePtr<Node> *parent_pattern,
										   		    const TreePtrInterface *ppattern )
{
	return PatternLink( *parent_pattern, ppattern ).GetShortName();
};		


string Graph::Indent(string s)
{
    stringstream ss(s);
    string sl, sout;

    while(getline(ss, sl, '\n'))
        sout += "    " + sl + "\n";

    return sout;
}