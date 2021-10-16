/*
 * graph.cpp
 *
 *  Created on: 18 Jul 2010
 *      Author: jgraley
 */

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/vn_transformation.hpp"
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
    base_region( ReadArgs::graph_dark ? "black" : "antiquewhite1" ),
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
    my_blocks = GetBlocks( my_graphables, nullptr );
    TrimLinksByChild( my_blocks, my_graphables );

    PostProcessBlocks(my_blocks);
    s += DoBlocks(my_blocks, base_region);
    s += DoLinks(my_blocks);
    s += "\n";
	Remember(s);
    
    CheckLinks(my_blocks);    
}


void Graph::operator()( const Figure &figure )
{        
    // Compile handy "all"s: Figure::Agents and Graphables
    list<const Graphable *> interior_gs, exterior_gs, all_gs;
    list<Figure::Agent> all_agents = figure.interior_agents + figure.exterior_agents;
    for( auto engine_agent : figure.subordinate_engines_and_root_agents )
        all_agents.push_back(engine_agent.second);      
    for( const Figure::Agent &agent : all_agents )
        all_gs.push_back(agent.g);
    
    // Exterior agents (shown outside of region): get blocks for them and 
    // remove links to graphables not in this figure
    list<MyBlock> exterior_blocks;
    {
        list<const Graphable *> exterior_gs;
        for( const Figure::Agent &agent : figure.exterior_agents )
            exterior_gs.push_back( agent.g );
        exterior_blocks = GetBlocks( exterior_gs, &figure );
        TrimLinksByChild( exterior_blocks, all_gs );
    }
    
    // Interior agents (shown inside region): get blocks for them and 
    // remove links to graphables not in this figure
    list<MyBlock> interior_blocks;
    {
        list<const Graphable *> interior_gs;
        for( const Figure::Agent &agent : figure.interior_agents )
            interior_gs.push_back(agent.g );
        interior_blocks = GetBlocks( interior_gs, &figure );
        TrimLinksByChild( interior_blocks, all_gs );
    }
    
    // Subordinate engines (shown as shaded sub-regions) and their 
    // agents (invisible): get blocks for them, remove links to graphables 
    // not in this SUB-REGION, make them invisible.    
    map< const GraphIdable *, list<MyBlock> > subordinate_blocks;
    for( auto engine_agent : figure.subordinate_engines_and_root_agents )
    {
        list<const Graphable *> sub_gs = { engine_agent.second.g };

        Region sub_region;
        sub_region.id = GetRegionGraphId(&figure, engine_agent.first); 
        // Note: the same root agent can be used in multiple engines, but because we've got the
        // current subordinate's id in the sub_region.id, the new node will be unique enough
        list<MyBlock> sub_blocks = GetBlocks( sub_gs, &sub_region );
        
        // Snip off all links that leave this subordinate region
        TrimLinksByChild( sub_blocks, sub_gs );
        
        // Make them all invisible
        for( MyBlock &sub_block : sub_blocks )
            sub_block.shape = "invisible";
                            
        // Set the child id correctly on all the links from all the interior nodes to our nodes. 
        // Note: ALL redirections/updates apply to interior nodes, because 
        // these are the only ones with outgoing links that we care about.
        for( auto p : Zip( sub_gs, sub_blocks ) )     
        {   
            for( const Figure::Link &figure_link : engine_agent.second.incoming_links )
            {
                shared_ptr<MyLink> link = FindLink( interior_blocks, 
                                                    p.first, 
                                                    figure_link.short_name );
                if( link )
                    link->child_id = p.second.base_id;       
            }
        }
                            
        subordinate_blocks[engine_agent.first] = sub_blocks;    
    }
    
    // Special case for trivial engines (aka no normal agents): a new invisible 
    // node goes into the INTERNAL region and points to all externals.
    if( figure.interior_agents.empty() )
    {
        list< tuple<const Graphable *, string, Graphable::Phase> > links_info;
        for( const Figure::Agent &agent : figure.exterior_agents )
        {
            for( const Figure::Link &figure_link : agent.incoming_links )
            {
                // Note: we don't actually know the phase, could be IN_COMPARE_ONLY or IN_COMPARE_AND_REPLACE
                links_info.push_back( make_tuple(agent.g, figure_link.short_name, IN_COMPARE_ONLY) );
            }
        }
        interior_blocks.push_back( CreateInvisibleBlock( "IRIP", links_info, &figure ) ); 
        // IRIP short for InvisibleRootInteriorPlaceholder, but the length of the string sets the width of the region!    
    }
    
    // Set the planned_as on all the links from all the interior nodes.
    for( const Figure::Agent &agent : all_agents )
    {
        for( const Figure::Link &figure_link : agent.incoming_links )
        {
            shared_ptr<MyLink> link = FindLink( interior_blocks, 
                                                agent.g, 
                                                figure_link.short_name );
            if( link )
                link->planned_as = figure_link.details.planned_as;     
        }     
    };
     
    // Trim off replace-only links
    set<Graphable::Phase> phases_to_keep = { Graphable::IN_COMPARE_ONLY, 
                                             Graphable::IN_COMPARE_AND_REPLACE };
    TrimLinksByPhase( exterior_blocks, phases_to_keep );
    TrimLinksByPhase( interior_blocks, phases_to_keep );
    for( auto p : figure.subordinate_engines_and_root_agents )
        TrimLinksByPhase( subordinate_blocks[p.first], phases_to_keep );

    // Post-process blocks
    PostProcessBlocks(exterior_blocks);
    PostProcessBlocks(interior_blocks);
    for( auto p : figure.subordinate_engines_and_root_agents ) 
		PostProcessBlocks( subordinate_blocks[p.first] );
    
    // Check for broken links (no block with matching child id)
    list<MyBlock> all_blocks = interior_blocks + exterior_blocks;
    for( auto p : figure.subordinate_engines_and_root_agents )
        all_blocks = all_blocks + subordinate_blocks.at(p.first);
    // Links must be checked for a whole figure because figures dont link to each other
    CheckLinks(all_blocks);    
    
    // Now generate all the dot code for the current figure
    string s;
    s += "// -------------------- figure "+figure.id+" --------------------\n";   
    
    RegionAppearance interior_region = base_region;
    interior_region.title = figure.title;
    interior_region.id = figure.id;
    interior_region.background_colour = ReadArgs::graph_dark ? "gray15" : "antiquewhite2";
    string s_interior = DoBlocks(interior_blocks, interior_region); // Interior blocks

    for( auto p : figure.subordinate_engines_and_root_agents )
    {
		RegionAppearance subordinate_region = interior_region;
		subordinate_region.title = p.first->GetGraphId();
		subordinate_region.id = GetRegionGraphId(&figure, p.first);
		subordinate_region.background_colour = ReadArgs::graph_dark ? "gray25" : "antiquewhite3";

        list<MyBlock> sub_blocks = subordinate_blocks.at(p.first);        
	    string s_subordinate = DoBlocks(sub_blocks, subordinate_region); 
		s_interior += DoRegion(s_subordinate, subordinate_region);
	}

    s += DoRegion(s_interior, interior_region);

    s += DoBlocks(exterior_blocks, base_region); // Exterior blocks

    // Links must be done at figure-level otherwise they "pull" blocks into regions
    s += DoLinks(all_blocks); 
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
	my_blocks = GetBlocks( my_graphables, nullptr );
    TrimLinksByChild( my_blocks, my_graphables );
    PostProcessBlocks(my_blocks);
    s += DoBlocks(my_blocks, base_region);
    s += DoLinks(my_blocks);
    s += "\n";
	Remember( s );
    
    CheckLinks(my_blocks);

	return root; // no change
}


void Graph::PopulateFromTransformation( list<const Graphable *> &graphables, Transformation *root )
{    
    if( TransformationVector *tv = dynamic_cast<TransformationVector *>(root) )
    {
        FOREACH( shared_ptr<Transformation> t, *tv ) // TODO loop backwards so they come out in the right order in graph
            PopulateFromTransformation( graphables, t.get() );
    }
    else if( VNTransformation *vnt = dynamic_cast<VNTransformation *>(root) )
    {
		reached.clear();
		PopulateFrom( graphables, vnt );
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
		for( shared_ptr<const Graphable::Link> link : sub_block.links )
		{
			if( link->child && reached.count(link->child)==0 )
			{
				PopulateFrom( graphables, link->child );
				reached.insert( link->child );
			}
		}
	}
}

  
shared_ptr<Graph::MyLink> Graph::FindLink( list<MyBlock> &blocks_to_act_on, 
                                           const Graphable *target_child_g,
                                           string target_trace_label )
{
    TRACE("UpdateLinksDetails( target_child_g=")(target_child_g)(" target_trace_label=")(target_trace_label)(" )\n");         
    // Loop over all the links in all the blocks that we might need to 
    // redirect (ones in the interior of the figure)
	for( MyBlock &block_to_act_on : blocks_to_act_on )
	{
        TRACEC("    Block: ")(block_to_act_on.base_id)("\n");
        for( Graphable::SubBlock &sub_block_to_act_on : block_to_act_on.sub_blocks )
        {
            for( shared_ptr<Graphable::Link> link_to_act_on : sub_block_to_act_on.links )
            {
                auto my_link_to_act_on = dynamic_pointer_cast<MyLink>(link_to_act_on);
                ASSERT( my_link_to_act_on );
                TRACEC("        To act on: child_id=")(my_link_to_act_on->child_id)
                      (" child=")(link_to_act_on->child)
                      (" labels=")(link_to_act_on->labels)(link_to_act_on->trace_labels)("\n");
                // Two things must be true for us to update this link's planned_as field:
                // - Link must point to the right agent - that being the root agent of the sub-engine
                // - The link label (satellite serial number of the PatternLink) must match the one supplied to us for the sub-engine
                // AndRuleEngine knows link labels for sub-engines. These two criteria ensure we have got the right link. 
                if( link_to_act_on->child == target_child_g )
                {
                    ASSERT( link_to_act_on->trace_labels.size()==1 ); // brittle
                    if( link_to_act_on->trace_labels.front() == target_trace_label )
                    {
                        return my_link_to_act_on;
                    }
                }
            }
        }
    }
    return nullptr;
}                           


void Graph::CheckLinks( list<MyBlock> blocks )
{
    set<string> base_ids;

  	for( MyBlock &block : blocks )
        base_ids.insert( block.base_id );
    
  	for( MyBlock &block : blocks )
	{
        for( Graphable::SubBlock &sub_block : block.sub_blocks )
        {
            for( shared_ptr<const Graphable::Link> link : sub_block.links )
            {                
                auto my_link = dynamic_pointer_cast<const MyLink>(link);
                ASSERT( my_link );

                ASSERT( base_ids.count( my_link->child_id ) > 0 )
                      ("Link to child id ")(my_link->child_id)(" but no such block\n")
                      (DoLink(0, block, link));
            }
        }
    }
}


list<Graph::MyBlock> Graph::GetBlocks( list<const Graphable *> graphables,
                                       const Region *region )
{
	list<MyBlock> blocks;

	for( const Graphable *g : graphables )	
        blocks.push_back( GetBlock( g, region ) );	

	return blocks;
}


Graph::MyBlock Graph::GetBlock( const Graphable *g,
                                const Region *region )
{
    Graphable::Block gblock = g->GetGraphBlockInfo(my_lnf, nullptr);       
    MyBlock block = PreProcessBlock( gblock, g, region );
    return block;
}
                  

void Graph::TrimLinksByChild( list<MyBlock> &blocks,
                              list<const Graphable *> to_keep )
{
    set<const Graphable *> to_keep_set;
    for( const Graphable *g : to_keep )
        to_keep_set.insert( g );
        
    TrimLinksByChild( blocks, to_keep_set );
}                              

                              
void Graph::TrimLinksByChild( list<MyBlock> &blocks,
                              set<const Graphable *> to_keep )
{
	for( MyBlock &block : blocks )
	{       
        for( Graphable::SubBlock &sub_block : block.sub_blocks )
        {			
            list< shared_ptr<Graphable::Link> > new_links;
            for( shared_ptr<Graphable::Link> link : sub_block.links )
            {
                if( to_keep.count(link->child) > 0 )
                {
                    ASSERT( link->child )(block.title)(" ")(sub_block.item_name);
                    new_links.push_back( link );
                }
            }
            sub_block.links = new_links;
        }
	}
}


void Graph::TrimLinksByPhase( list<MyBlock> &blocks,
                              set<Graphable::Phase> to_keep )
{
	for( MyBlock &block : blocks )
	{       
        for( Graphable::SubBlock &sub_block : block.sub_blocks )
        {			
            list< shared_ptr<Graphable::Link> > new_links;
            for( shared_ptr<Graphable::Link> link : sub_block.links )
            {
                if( to_keep.count(link->phase) > 0 )
                    new_links.push_back( link );
            }
            sub_block.links = new_links;
        }
	}
}


Graph::MyBlock Graph::CreateInvisibleBlock( string id, 
                                           list< tuple<const Graphable *, string, Graphable::Phase> > links_info,  
                                           const Region *region )
{
	MyBlock block;
	block.title = "";     
	block.bold = false;
	block.shape = "invisible";
    block.block_type = Graphable::NODE;
    block.prerestriction_name = "";
    block.colour = "";
    block.specify_ports = false;
    block.base_id = GetRegionGraphId(region, id);
    block.italic_title = false;
    
    Graphable::SubBlock sub_block = { "", 
                                      "",
                                      false,
                                      {} };
    for( auto link_info : links_info )
    {        
        auto link = make_shared<Graphable::Link>( nullptr, 
                                                  list<string>{},
                                                  list<string>{get<1>(link_info)},
                                                  get<2>(link_info),
                                                  false );
        auto my_link = make_shared<MyLink>( link,
                                            GetRegionGraphId(region, get<0>(link_info)),
                                            LINK_NORMAL );
        //link.trace_labels.push_back( lnf( ... ) ); TODO
        //link.is_nontrivial_prerestriction = ntprf ? ntprf(&p) : false;
        sub_block.links.push_back( my_link );
    }
    block.sub_blocks.push_back( sub_block );
		    
                
    return block;
}


Graph::MyBlock Graph::PreProcessBlock( const Graphable::Block &block, 
                                       const Graphable *g,
                                       const Region *region )
{
	ASSERT(g);
	const Node *pnode = dynamic_cast<const Node *>(g);
    const SpecialBase *pspecial = pnode ? dynamic_cast<const SpecialBase *>(pnode) : nullptr;

    // Fill in everything in my block 
    MyBlock my_block;
    (Graphable::Block &)my_block = block;
    
    // Fill in the GraphViz ID that the block will use
    my_block.base_id = GetRegionGraphId(region, g);
    
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
    for( Graphable::SubBlock &sub_block : my_block.sub_blocks )
    {			
        // Actions for links
        for( shared_ptr<Graphable::Link> &link : sub_block.links )
        {							
            // Fill in everything in my link 
            auto my_link = make_shared<MyLink>( link, 
                                                GetRegionGraphId(region, link->child), 
                                                LINK_NORMAL );		
            
            // Detect pre-restrictions and add to link labels
            if( link->is_nontrivial_prerestriction )
            {
                block_ids_show_prerestriction.insert( my_link->child_id );
            }
            
            link = my_link;
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
        PostProcessBlock(block);
    }
}


void Graph::PostProcessBlock( MyBlock &block )
{
    if( block_ids_show_prerestriction.count( block.base_id ) > 0 )
    {          
        // Note: using push_front to get pre-restriction near the top (under title)
        block.sub_blocks.push_front( { "("+block.prerestriction_name+")", 
                                       "", 
                                       false, 
                                       {} } );
    }
    
    // Can we hide sub-blocks?
    bool sub_blocks_hideable = ( block.sub_blocks.size() == 0 || 
                                 (block.sub_blocks.size() == 1 && block.sub_blocks.front().hideable) );

    // If not, make sure we're using a shape that allows for sub_blocks
    if( !sub_blocks_hideable && !(block.shape == "invisible" || block.shape == "plaintext" || block.shape == "record") )
        block.shape = "plaintext";      
    
    // These kinds of blocks require port names to be to be specified so links can tell them apart
    block.specify_ports = (block.shape=="record" || block.shape=="plaintext");              
}


string Graph::DoBlocks( const list<MyBlock> &blocks, 
                        const RegionAppearance &region )
{
    string s;
    
    for( const MyBlock &block : blocks )
        s += DoBlock(block, region);
    
    return s;
}


string Graph::DoBlock( const MyBlock &block, 
                       const RegionAppearance &region )
{
	string s;
    
    s += "// -------- block " + block.base_id + " ----------\n";
    if( block.shape == "invisible" )
        s += "shape = none\n";
    else
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
	else if( block.shape == "invisible" )
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


string Graph::DoLinks( const list<MyBlock> &blocks )
{
	string s;
    
    for( const MyBlock &block : blocks )
        s += DoLinks(block);

    return s;
}


string Graph::DoLinks( const MyBlock &block )
{
    string s;
    
    int porti=0;
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        for( shared_ptr<const Graphable::Link> link : sub_block.links )
        {
			s += DoLink( porti, block, link );
		}
        porti++;
    }

	return s;
}


string Graph::DoLink( int port_index, 
                      const MyBlock &block, 
                      shared_ptr<const Graphable::Link> link )
{          
    auto my_link = dynamic_pointer_cast<const MyLink>(link);
    ASSERT( my_link );
    
    // Atts
    string atts;
    atts += LinkStyleAtt(my_link->planned_as, link->phase);

    // Labels
    list<string> labels;
    if( ReadArgs::graph_trace )
        labels = link->labels + link->trace_labels;
    else
        labels = link->labels;
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
	s += "\""+my_link->child_id+"\"";
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
	s += "}\n";
	return s;
}


string Graph::DoRegion(string ss, const RegionAppearance &region)
{
    string s;
    s += "label = \"" + region.title + "\"\n";
    s += "style = \"filled\"\n";
	s += "color = " + region.background_colour + "\n";
    s += "fontsize = \"" FS_LARGE "\"\n";
	s += ss;
 
    string sc;
    sc += "// -------- region " + region.id + " ----------\n";
    sc += "subgraph \"cluster" + region.id + "\" {\n";
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


string Graph::LinkStyleAtt(LinkPlannedAs incoming_link_planned_as, Graphable::Phase phase)
{
    string atts;
    switch(incoming_link_planned_as)
    {
    case LINK_NORMAL:
        break;
    case LINK_KEYER:
        atts += "arrowhead=\"normalnonebox\"\n";
        break;
    case LINK_RESIDUAL:
        atts += "arrowhead=\"normalnonetee\"\n";
        break;
    case LINK_ABNORMAL:
        atts += "arrowhead=\"normalnoneodot\"\n";
        break;
    case LINK_EVALUATOR:
        atts += "arrowhead=\"normalnoneodiamond\"\n";
        break;
    case LINK_MULTIPLICITY:
        atts += "arrowhead=\"normalonormalonormalonormal\"\n";
        break;
    }
    
    switch( phase )
    {
    case IN_COMPARE_ONLY:
    case IN_COMPARE_AND_REPLACE:
        //atts += "style=\"solid\"\n";
        break;
    case IN_REPLACE_ONLY:
        atts += "style=\"dashed\"\n";
        break;
    }
    return atts;
}


string Graph::GetRegionGraphId(const Region *region, const GraphIdable *g)
{
	return GetRegionGraphId(region, g->GetGraphId());
}


string Graph::GetRegionGraphId(const Region *region, string id)
{
	if( !region )
		return id;
	else
		return region->id+"/"+id;
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