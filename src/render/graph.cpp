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

#define FS_TINY "8"
#define FS_SMALL "12"
#define FS_MIDDLE "14"
#define FS_LARGE "16"
#define FS_HUGE "18"
#define NS_SMALL "0.4"
//#define FONT "Arial"

#include <functional>

Graph::Graph( string of ) :
    outfile(of),
    base_region { "",
		          ReadArgs::graph_dark ? "black" : "antiquewhite1" },
	line_colour( ReadArgs::graph_dark ? "grey70" : "black" ),
    font_colour( ReadArgs::graph_dark ? "white" : "black" )
{
	if( !outfile.empty() )
	{
		filep = fopen( outfile.c_str(), "wt" );
		ASSERT( filep )( "Cannot open output file \"%s\"", outfile.c_str() );
    }
    
	Disburse( DoHeader() );
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
    Figure my_graphables;
    list<MyBlock> my_blocks;

	reached.clear();
    PopulateFromTransformation(my_graphables.interior, root);
    my_blocks = GetBlocks(my_graphables.interior, "");
    PostProcessBlocks(my_blocks);
    string s = DoGraphBody(my_blocks, base_region);
	Remember(s);
}


void Graph::operator()( string figure_id, const Figure &figure )
{    
    RegionAppearance my_region = base_region;
    my_region.region_id += figure_id;
    my_region.background_colour = ReadArgs::graph_dark ? "gray15" : "antiquewhite2";

    list<MyBlock> my_blocks = GetBlocks(figure.interior, figure_id);
    PostProcessBlocks(my_blocks);
    list<MyBlock> ex_blocks = GetBlocks(figure.exterior, figure_id);
    PostProcessBlocks(ex_blocks);

	string s = DoGraphBody(ex_blocks, base_region);

    string sc = DoGraphBody(my_blocks, my_region);
    s += DoCluster(sc, my_region);

	Remember( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
    Figure my_graphables;
    list<MyBlock> my_blocks;
	(void)context; // Not needed!!

    reached.clear();
    Graphable *g = dynamic_cast<Graphable *>(root.get());
	PopulateFrom( my_graphables.interior, g );
	my_blocks = GetBlocks(my_graphables.interior, "");
    PostProcessBlocks(my_blocks);
    string s = DoGraphBody(my_blocks, base_region);
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


list<Graph::MyBlock> Graph::GetBlocks( list< const Graphable *> graphables, string figure_id )
{
	list<MyBlock> blocks;
	
	for( const Graphable *g : graphables )
	{
		Graphable::Block gblock = g->GetGraphBlockInfo(my_lnf, nullptr);
        MyBlock block = PreProcessBlock( gblock, g, figure_id );
        blocks.push_back( block );
	}

	return blocks;
}


Graph::MyBlock Graph::PreProcessBlock( const Graphable::Block &block, 
                                       const Graphable *g,
                                       string figure_id )
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
		my_block.link_ids.push_back( list<string>() );
		
        // Actions for links
        for( Graphable::Link &link : sub_block.links )
        {
			ASSERT( link.child )(block.title)(" ")(sub_block.item_name);
			
			string id = GetFullId(link.child, figure_id);
            my_block.link_ids.back().push_back( id );

            // Detect pre-restrictions and add to link labels
            if( link.is_ntpr )
            {
                block_ids_show_prerestriction.insert( id );
            }
        }
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
        if( !sub_blocks_hideable && !(block.shape == "plaintext" || block.shape == "record") )
            block.shape = "plaintext";      
        
        // These kinds of blocks require port names to be to be specified so links can tell them apart
        block.specify_ports = (block.shape=="record" || block.shape=="plaintext");              
    }
}


string Graph::DoGraphBody( const list<MyBlock> &blocks, const RegionAppearance &region )
{
    string s;
    
    for( const MyBlock &block : blocks )
    {
        s += DoBlock(block, region);
        blocks_for_links.push_back(block);
    }
    
    return s;
}


string Graph::DoBlock( const MyBlock &block, const RegionAppearance &region )
{
	string s;
	s += "\""+block.base_id+"\"";
	s += " [\n";
    
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
		s += "fontsize = \"" FS_SMALL "\"\n";
	}
	else if( block.shape == "record" )
    {
        s += "label = " + DoRecordLabel( block );
        s += "style = \"filled\"\n";
        s += "fontsize = \"" FS_MIDDLE "\"\n";
        s += "color = " + line_colour + "\n";
        s += "fontcolor = " + font_colour + "\n";
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

	s += "];\n";
	return s;
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
	s += " <TR>\n";
	s += "  <TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + lt + "</FONT></TD>\n";
	s += "  <TD></TD>\n";
	s += " </TR>\n";
    
    int porti=0;
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        s += " <TR>\n";
        s += "  <TD>" + EscapeForGraphviz(sub_block.item_name) + "</TD>\n";
        s += "  <TD PORT=\"" + SeqField( porti ) + "\">" + EscapeForGraphviz(sub_block.item_extra) + "</TD>\n";
        s += " </TR>\n";
        porti++;
    }
    
	s += "</TABLE>>\n";
	return s;
}


string Graph::DoLinks( const MyBlock &block )
{
    string s;
    
    int porti=0;
    FTRACE( "DoLinks()\n" );
    auto sbidit = block.link_ids.begin();
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
		ASSERT(sbidit != block.link_ids.end() );
		FTRACE( "OK\n" );
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
    atts += LinkStyleAtt(link.link_style);

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
	s += " ["+atts+"];\n";
	return s;
}


string Graph::DoHeader()
{
	string s;
	s += "digraph Inferno {\n"; // g is name of graph
	s += "graph [\n";
	s += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
	s += "ranksep = 1.0\n"; // 1-inch separation parent-child (default 0.5)
	s += "size = \"14,20\"\n"; // make it smaller
  //  s += "concentrate = \"true\"\n"; 
    s += "bgcolor = " + base_region.background_colour + "\n";
    s += "color = " + line_colour + "\n";
    s += "fontcolor = " + font_colour + "\n";
    s += "];\n";
	s += "node [\n";
#ifdef FONT
    s += "fontname = \"" FONT "\"\n";
#endif    
	s += "];\n";
	return s;
}


string Graph::DoFooter()
{
	string s;

    for( const MyBlock &block : blocks_for_links )
        s += DoLinks(block);
        	
	s += "}\n";
	return s;
}


string Graph::DoCluster(string ss, const RegionAppearance &region)
{
    string s;
    s += "subgraph \"cluster" + region.region_id + "\" {\n";
    s += "label = \"" + region.region_id + "\"\n";
    s += "style = \"filled\"\n";
	s += "color = " + region.background_colour + "\n";
	s += ss;
    s += "}\n";
    return s;
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
    case Graphable::SOLID:
        atts += "style=\"solid\"\n";
        break;
    case Graphable::DASHED:
        atts += "style=\"dashed\"\n";
        break;
    }
    return atts;
}


string Graph::GetFullId(const Graphable *g, string figure_id)
{
	if(figure_id.empty() )
		return g->GetGraphId();
	else
		return figure_id+"/"+g->GetGraphId();
}


const Graph::LinkNamingFunction Graph::my_lnf = []( const TreePtr<Node> *parent_pattern,
										   		    const TreePtrInterface *ppattern )
{
	return PatternLink( *parent_pattern, ppattern ).GetShortName();
};		
