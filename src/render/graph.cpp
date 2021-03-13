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
    outfile(of)
{
	if( !outfile.empty() )
	{
		filep = fopen( outfile.c_str(), "wt" );
		ASSERT( filep )( "Cannot open output file \"%s\"", outfile.c_str() );
    }
    
    string s;
    s += DoHeader();
	Disburse( s );
}


Graph::~Graph()
{
    string s;
  	s += DoFooter();
	Disburse( s );

    if( !outfile.empty() )
    {
        fclose( filep );
    }
}


void Graph::operator()( Transformation *root )
{    
    reached.clear();
    PopulateFromTransformation(root);	
    PostProcessBlocks();
    string s = DoGraphBody();
	Disburse( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

    reached.clear();
	PopulateFrom( root, Graphable::SOLID );
    PostProcessBlocks();
    string s = DoGraphBody();
	Disburse( s );

	return root; // no change
}


void Graph::PopulateFromTransformation(Transformation *root)
{    
    if( TransformationVector *tv = dynamic_cast<TransformationVector *>(root) )
    {
        FOREACH( shared_ptr<Transformation> t, *tv ) // TODO loop backwards so they come out in the right order in graph
            PopulateFromTransformation( t.get() );
    }
    else if( CompareReplace *cr = dynamic_cast<CompareReplace *>(root) )
    {
		reached.clear();
	    PopulateFromControl( cr, nullptr, Graphable::THROUGH );
	}
	else
    {
        ASSERTFAIL("Unknown kind of transformation in graph plotter");
    }
}


void Graph::PopulateFromControl( const Graphable *g,
                                 TreePtr<Node> nbase, 
                                 Graphable::LinkStyle default_link_style )
{
    Graphable::Block gblock = g->GetGraphBlockInfo(my_lnf);
    gblock.default_link_style = default_link_style;
    MyBlock block = PreProcessBlock( gblock, g, nbase, true );
    my_blocks.push_back( block );
            
    PopulateFromSubBlocks( block );
}


void Graph::PopulateFromNode( TreePtr<Node> node,
                              Graphable::LinkStyle default_link_style )
{
	Graphable::Block nblock = GetNodeBlockInfo( node );
	nblock.default_link_style = default_link_style;
	MyBlock block = PreProcessBlock( nblock, nullptr, node, false );
	my_blocks.push_back( block );

	PopulateFromSubBlocks( block );
}

                              
void Graph::PopulateFrom( TreePtr<Node> node,
                          Graphable::LinkStyle default_link_style )
{

	Graphable *g = ShouldDoControlBlock(node);
	
	if( g )
		PopulateFromControl( g, node, default_link_style );
	else
		PopulateFromNode( node, default_link_style );
}


void Graph::PopulateFromSubBlocks( const MyBlock &block )
{
	for( const Graphable::SubBlock &sub_block : block.sub_blocks )
	{
		for( const Graphable::Link &link : sub_block.links )
		{
			if( GetChildNode(link) && reached.count(GetChildNode(link))==0 )
			{
				PopulateFrom( GetChildNode(link), link.link_style );
				reached.insert( GetChildNode(link) );
			}
		}
	}
}


Graph::MyBlock Graph::PreProcessBlock( const Graphable::Block &block, 
                                       const Graphable *g,
                                       TreePtr<Node> node, 
                                       bool for_control_block )
{
    // Fill in everything in block 
    MyBlock my_block;
    (Graphable::Block &)my_block = block;
    
    // Fill in the GraphViz ID that the block will use
    my_block.base_id = Id(g, node);
    
    // Capture the node (if there is one: might be NULL)
    my_block.as_node = node;
    
    // In graph trace mode, nodes get their serial number added in as an extra sub-block (with no links)
    if( ReadArgs::graph_trace && node )
    {
        my_block.sub_blocks.push_back( { node->GetSerialString(), 
                                         "", 
                                         false, 
                                         {} } );
    }
    
    // Colour the block in accordance with the node if there is one otherwise leave the colour blank.
    // See #258: "block colour shall be dictated by the node type only"
    if( node )
        my_block.colour = node->GetColour();
    else
        my_block.colour = "transparent";


    // Make the titles more wieldy by removing template stuff - note:
    // different policies for control blocks vs node blocks.
    if( for_control_block )
    {
        my_block.title = RemoveAllTemplateParam(my_block.title); 
        my_block.title = RemoveOneOuterScope(my_block.title); 
        my_block.shape = "record";
    }
    else
    {
        my_block.title = GetInnermostTemplateParam(my_block.title);
    }
    
    // Actions for sub-blocks
    for( Graphable::SubBlock &sub_block : my_block.sub_blocks )
    {
        // Actions for links
        for( Graphable::Link &link : sub_block.links )
        {
			ASSERT( GetChildNode(link) )(block.title)(" ")(sub_block.item_name);
			ASSERT( link.ptr )(block.title)(" ")(sub_block.item_name);
			ASSERT( GetChildNode(link) == GetChildNode(link) )(block.title)(" ")(sub_block.item_name);

            // Detect pre-restrictions and add to link labels
            if( IsNonTrivialPreRestriction( link.ptr ) )
            {
                block_ids_show_prerestriction.insert( Id(nullptr, GetChildNode(link)) );
            }
        }
    }
    
    // Italic title OR symbol designates a special agent 
    bool special = node && dynamic_pointer_cast<SpecialBase>(node);
    my_block.italic_title = special;
    if( !special )
        ASSERT( my_block.symbol.empty() );

    // Apply current link style to links as a default
    PropagateLinkStyle( my_block, my_block.default_link_style );

    return my_block;    
}


void Graph::PropagateLinkStyle( MyBlock &dest, Graphable::LinkStyle link_style )
{
    for( Graphable::SubBlock &sub_block : dest.sub_blocks ) 
    {  
        for( Graphable::Link &link : sub_block.links )
        {
            if( link.link_style == Graphable::THROUGH )
                link.link_style = link_style;    
        }
    }
}


Graphable::Block Graph::GetNodeBlockInfo( TreePtr<Node> node )
{    
    ASSERT(node);
    Graphable::Block block;
	const Graphable *g = (const Graphable*)Agent::TryAsAgentConst(node);
        
    // Temporary: StandardAgent is Graphable (because Agent is) but 
    // does not implement GetGraphBlockInfo() and the default gets used.
    auto sa = dynamic_cast<const StandardAgent *>(Agent::TryAsAgentConst(node));
        
    if( g && !sa )
    {
        (Graphable::Block &)block = Agent::AsAgent(node)->GetGraphBlockInfo(my_lnf);
        ASSERT(!block.title.empty());
        return block;
    }
    else // not Graphable or StandardAgent
    {
        return GetDefaultNodeBlockInfo(node, my_lnf);
    }
}


Graphable::Block Graph::GetDefaultNodeBlockInfo( TreePtr<Node> n, const LinkNamingFunction &lnf )
{    
	Graphable::Block block;
	block.title = n->GetGraphName();     
	block.bold = false;
	block.shape = "plaintext";
        
    vector< Itemiser::Element * > members = n->Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
            int j=0;
			FOREACH( const TreePtrInterface &p, *seq )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(seq->GetName()), 
                                                  SSPrintf("[%d]", j++),
                                                  false,
                                                  {} };
                Graphable::Link link;
                link.ptr = &p;
                link.link_style = Graphable::THROUGH;
                link.trace_labels.push_back( PatternLink( n, &p ).GetShortName() );
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
			}
		}
		else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
            string dots;
            for( int j=0; j<col->size(); j++ )
                dots += ".";
            
            Graphable::SubBlock sub_block = { GetInnermostTemplateParam(col->GetName()), 
                                              "{" + dots + "}",
                                              false,
                                              {} };
			FOREACH( const TreePtrInterface &p, *col )
            {
                Graphable::Link link;
                link.ptr = &p;
                link.link_style = Graphable::THROUGH;                
                link.trace_labels.push_back( PatternLink( n, &p ).GetShortName() );
                sub_block.links.push_back( link );
            }
            block.sub_blocks.push_back( sub_block );
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			if( *ptr )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(ptr->GetName()), 
                                                  "",
                                                  false,
                                                  {} };
                Graphable::Link link;
                link.ptr = ptr;
                link.link_style = Graphable::THROUGH;                
                link.trace_labels.push_back( PatternLink( n, ptr ).GetShortName() );          
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { GetInnermostTemplateParam(ptr->GetName()), 
                                                  "NULL",
                                                  false, 
                                                  {} };
                block.sub_blocks.push_back( sub_block );
            }
		}
		else
		{
			ASSERT(0);
		}
	}
    
    return block;
}


void Graph::PostProcessBlocks()
{
    for( MyBlock &block : my_blocks )
    {
        if( block_ids_show_prerestriction.count( block.base_id ) > 0 )
        {
            string prs = GetPreRestrictionName( block.as_node );
            
            // Note: using push_front to get pre-restriction near the top (under title)
            block.sub_blocks.push_front( { "("+prs+")", 
                                           "", 
                                           false, 
                                           {} } );
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


string Graph::DoGraphBody()
{
    string s;
    
    for( const MyBlock &block : my_blocks )
        s += DoBlock(block);

    for( const MyBlock &block : my_blocks )
        s += DoLinks(block);
    
    return s;
}


string Graph::DoBlock( const MyBlock &block )
{
	string s;
	s += "\""+block.base_id+"\"";
	s += " [\n";
    
	s += "shape = \"" + block.shape + "\"\n";
	if(block.colour != "")
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
        if( ReadArgs::graph_dark )
        {
            s += "fillcolor = gray15\n";
            s += "fontcolor = white\n";
            s += "color = gray70\n";
        }
        else
        {
            s += "fillcolor = antiquewhite1\n";
        }
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
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        for( Graphable::Link link : sub_block.links )
            s += DoLink( porti, block, sub_block, link );
        porti++;
    }

	return s;
}


string Graph::DoLink( int port_index, 
                      const MyBlock &block, 
                      const Graphable::SubBlock &sub_block, 
                      const Graphable::Link &link )
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

    if( ReadArgs::graph_dark )
    {
        atts += "color = gray70\n";
        atts += "fontcolor = white\n";
    }

    // GraphViz output
	string s;
	s += "\""+block.base_id+"\"";
    if( block.specify_ports )
        s += ":" + SeqField(port_index);
	s += " -> ";
	s += "\""+Id(nullptr, GetChildNode(link))+"\"";
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
    if( ReadArgs::graph_dark )
    {
        s += "bgcolor = gray15\n";
        s += "color = gray70\n";
        s += "fontcolor = white\n";    
    }
    else
    {
        s += "bgcolor = antiquewhite1\n";
    }
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
	s += "}\n";
	return s;
}


Graphable *Graph::ShouldDoControlBlock( TreePtr<Node> node )
{
    Graphable *g = dynamic_cast<Graphable *>(node.get());
    if( !g )
        return nullptr; // Need Graphable to do a block
           
    switch( g->GetGraphBlockInfo(my_lnf).block_type )
    {
        case Graphable::NODE:
            return nullptr;
            break;
        
        case Graphable::CONTROL:
            return g;
            break;
            
        default:
            ASSERTFAIL("Unknown block type");
    }
}


string Graph::Id( const Graphable *g, TreePtr<Node> node )
{
	if( node )
		g = (const Graphable*)Agent::TryAsAgentConst(node);
	
	return g ? g->GetGraphId() : node->GetSerialString();
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
    case Graphable::THROUGH:
        ASSERT(false);
        break;
    }
    return atts;
}


bool Graph::IsNonTrivialPreRestriction(const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>((TreePtr<Node>)*ptr) )   // is to a special node
        {
            if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchitype()) ) )    // pre-restrictor is nontrivial
            {
                return true;
            }
        }
    }
    return false;
}


string Graph::GetPreRestrictionName(TreePtr<Node> node)
{
    if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>(node) )   // is to a special node
    {
        return (**(sbs->GetPreRestrictionArchitype())).GetName();
    }
    return "";
}


const Graph::LinkNamingFunction Graph::my_lnf = []( TreePtr<Node> parent_pattern,
										   		    const TreePtrInterface *ppattern )
{
	return PatternLink( parent_pattern, ppattern ).GetShortName();
};		


TreePtr<Node> Graph::GetChildNode( const Graphable::Link &link )
{
	return (TreePtr<Node>)*(link.ptr);
}
