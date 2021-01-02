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
    string s;
    s += PopulateFromTransformation(root);	
	Disburse( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

	string s;
    unique_filter.Reset();
	s += PopulateFromNode( root, false );
    unique_filter.Reset();
	s += PopulateFromNode( root, true );
	Disburse( s );

	return root; // no change
}


string Graph::PopulateFromTransformation(Transformation *root)
{
    string s;
    if( TransformationVector *tv = dynamic_cast<TransformationVector *>(root) )
    {
        FOREACH( shared_ptr<Transformation> t, *tv )
            s = PopulateFromTransformation( t.get() ) + s; // seem to have to pre-pend to get them appearing in the right order
    }
    else if( CompareReplace *cr = dynamic_cast<CompareReplace *>(root) )
    {
        unique_filter.Reset();
	    s += PopulateFromEngine( cr, nullptr, Id(root), Graphable::THROUGH, false );
        unique_filter.Reset();
	    s += PopulateFromEngine( cr, nullptr, Id(root), Graphable::THROUGH, true );
	}
	else
    {
        ASSERTFAIL("Unknown kind of transformation in graph plotter");
    }
	return s;
}


string Graph::PopulateFromEngine( const Graphable *g, TreePtr<Node> nbase, string base_id, Graphable::LinkStyle default_link_style, bool links_pass )
{
    Graphable::Block gblock = g->GetGraphBlockInfo();
    MyBlock block = PreProcessBlock( gblock, nbase, true );
    //MyBlock block; (Graphable::Block &)block = gblock;    

    PropagateLinkStyle( block, default_link_style );

	string s;
    s += links_pass ? DoLinks(block, base_id) : DoBlock(block, base_id);
        
    LambdaFilter block_filter( [&](TreePtr<Node> context,
                                   TreePtr<Node> root) -> bool
    {
        return !ShouldDoControlBlock(root); // Stop where we will do blocks        
    });
            
    for( const Graphable::SubBlock &sub_block : block.sub_blocks )
    {
        for( const Graphable::Link &link : sub_block.links )
        {
            if( link.child_node )
            {
                Walk w( (TreePtr<Node>)(link.child_node), &unique_filter, &block_filter ); // return each node only once; do not recurse through transformations
                FOREACH( const TreePtrInterface &ni, w )
                {              
                    TreePtr<Node> node = (TreePtr<Node>)ni;
                    Graphable *g = ShouldDoControlBlock(node);
                    if( g )
                    {
                        s += PopulateFromEngine( g, node, Id(node.get()), link.link_style, links_pass );
                    }
                    else
                    {
                        MyBlock child_block = PreProcessBlock( GetNodeBlockInfo( node ), node, false );
                        PropagateLinkStyle( child_block, link.link_style );
                        if( links_pass )
                            s += DoLinks(child_block, Id(node.get()));
                        else
                            s += DoBlock(child_block, Id(node.get()));
                    }
                }
            }
        }
    }
	return s;
}


string Graph::PopulateFromNode( TreePtr<Node> root, bool links_pass )
{
	string s;
    TRACE("Graph plotter traversing intermediate %s pass\n", links_pass ? "links" : "nodes");
	::UniqueWalk w( root );
	FOREACH( const TreePtrInterface &n, w )
	{
		if( !n )
            continue;
            
        MyBlock child_block = PreProcessBlock( GetNodeBlockInfo( (TreePtr<Node>)n ), (TreePtr<Node>)n, false );
        if( links_pass )
            s += DoLinks(child_block, Id(n.get()));
        else
            s += DoBlock(child_block, Id(n.get()));
	}
	return s;
}


Graph::MyBlock Graph::PreProcessBlock( const Graphable::Block &block, TreePtr<Node> node, bool for_engine_block )
{
    // Fill in everything in block 
    MyBlock my_block;
    (Graphable::Block &)my_block = block;
    
    // In graph trace mode, nodes get their serial number added in as an extra sub-block (with no links)
    if( ReadArgs::graph_trace && node )
    {
        my_block.sub_blocks.push_back( { node->GetAddr(), 
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
    // different policies for engine blocks vs node blocks.
    if( for_engine_block )
    {
        my_block.title = RemoveAllTemplateParam(my_block.title); 
        my_block.title = RemoveOneOuterScope(my_block.title); 
        my_block.shape = "record";
    }
    else
    {
        my_block.title = GetInnermostTemplateParam(my_block.title);
    }
    
    // These kinds of blocks require port names to be to be specified so links can tell them apart
    my_block.specify_ports = (my_block.shape=="record" || my_block.shape=="plaintext");  
    
    // Actions for sub-blocks
    for( Graphable::SubBlock &sub_block : my_block.sub_blocks )
    {
        // Actions for links
        for( Graphable::Link &link : sub_block.links )
        {
            // Detect pre-restrictions and add to link labels
            string pr = GetPreRestriction( link.ptr );
            if( !pr.empty() )
                link.labels.push_back( pr );
        }
    }

    return my_block;    
}


Graphable::Block Graph::GetNodeBlockInfo( TreePtr<Node> n )
{
    Graphable::Block block;
    if( dynamic_pointer_cast<SpecialBase>(n) )
    {
        (Graphable::Block &)block = Agent::AsAgent(n)->GetGraphBlockInfo();
        if( !block.title.empty() )
            return block;
    }
    
    return GetDefaultNodeBlockInfo(n);
}


Graphable::Block Graph::GetDefaultNodeBlockInfo( TreePtr<Node> n )
{    
	Graphable::Block block;
	block.title = n->GetRender();     
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
                link.child_node = (TreePtr<Node>)p;
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
                link.child_node = (TreePtr<Node>)p;
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
                link.child_node = (TreePtr<Node>)*ptr;
                link.ptr = ptr;
                link.link_style = Graphable::THROUGH;                
                link.trace_labels.push_back( PatternLink( n, ptr ).GetShortName() );          
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { EscapeForGraphviz(GetInnermostTemplateParam(ptr->GetName())), 
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


string Graph::GetInnermostTemplateParam( string s )
{
    while(true)
    {
        string::size_type iopen = s.find("<");
        string::size_type iclose = s.rfind(">");
        if( iopen == std::string::npos || iclose == std::string::npos )
            break; // done
        iopen++; // get past <
        s = s.substr( iopen, iclose-iopen );
    }
    return s;
}


string Graph::RemoveAllTemplateParam( string s )
{
    int n;
    for( n=0; n<s.size(); n++ )
    {
        if( s[n] == '<' )        
        {
            int nn;
            for( nn=n; nn<s.size(); nn++ )
            {            
                if( s[nn] == '>' )        
                {
                    s = s.substr( 0, n ) + s.substr( nn+1 );
                    break;
                }
            }
        }
    }
    return s;
}


string Graph::RemoveOneOuterScope( string s )
{
    int n = s.find("::");
	if( n != string::npos )
	    s = s.substr( n+2 );
    return s;
}


string Graph::DoBlock( const MyBlock &block, string base_id )
{
	string s;
	s += base_id;
	s += " [\n";
    
    // Can we hide sub-blocks?
    bool sub_blocks_hideable = ( block.sub_blocks.size() == 0 || 
                                 (block.sub_blocks.size() == 1 && block.sub_blocks.front().hideable) );

    // If not, make sure we're using a shape that allows for sub_blocks
    string shape = block.shape;
    if( !sub_blocks_hideable && !(shape == "plaintext" || shape == "record") )
        shape = "plaintext";

	s += "shape = \"" + shape + "\"\n";
	if(block.colour != "")
		s += "fillcolor = \"" + block.colour + "\"\n";

    // shape=plaintext triggers HTML label generation. From Graphviz docs:
    // "Adding HTML labels to record-based shapes (record and Mrecord) is 
    // discouraged and may lead to unexpected behavior because of their 
    // conflicting label schemas and overlapping functionality."
    // https://www.youtube.com/watch?v=Tv1kRqzg0AQ
	if( shape == "plaintext" )
	{
		s += "label = " + DoHTMLLabel( block.title, block.sub_blocks );
		s += "style = \"rounded,filled\"\n";
		s += "fontsize = \"" FS_SMALL "\"\n";
	}
	else if( shape == "record" )
    {
        s += "label = " + DoRecordLabel( block.title, block.sub_blocks );
        s += "style = \"filled\"\n";
        s += "fontsize = \"" FS_MIDDLE "\"\n";
    }
    else
	{
        // Ignoring sub-block (above check means there will only be one: it
        // is assumed that the title is sufficietly informative
		s += "label = \"" + block.title + "\"\n";// TODO causes errors because links go to targets meant for records
		s += "style = \"filled\"\n";
        s += "fontsize = \"" FS_LARGE "\"\n";
        s += "penwidth = 0.0\n";

		if( block.title.size() <= 3 ) // can fit about 3 chars in standard small shape
		{
			s += "fixedsize = true\n";
			s += "width = " NS_SMALL "\n";
			s += "height = " NS_SMALL "\n";
		}
	}

	s += "];\n";
	return s;
}


string Graph::DoRecordLabel( string title, const list<Graphable::SubBlock> &sub_blocks )
{
    string s;
    s += "\"<fixed> " + EscapeForGraphviz( title );
    int k=0;
    for( Graphable::SubBlock sub_block : sub_blocks )
    {        
        string label_text = EscapeForGraphviz(sub_block.item_name + sub_block.item_extra);
        s += " | <" +  SeqField(k) + "> " + label_text;
        k++;
    }
    s += "\"\n";
    return s;
}


string Graph::DoHTMLLabel( string title, const list<Graphable::SubBlock> &sub_blocks )
{
    
	string s = "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n";
	s += " <TR>\n";
	s += "  <TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + EscapeForGraphviz(title) + "</FONT></TD>\n";
	s += "  <TD></TD>\n";
	s += " </TR>\n";
    
    int porti=0;
    for( Graphable::SubBlock sub_block : sub_blocks )
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


string Graph::DoLinks( const MyBlock &block, string base_id )
{
    string s;
    
    int porti=0;    
    for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        for( Graphable::Link link : sub_block.links )
            s += DoLink( porti, block, sub_block, link, base_id );
        porti++;
    }

	return s;
}


string Graph::DoLink( int port_index, 
                      const MyBlock &block, 
                      const Graphable::SubBlock &sub_block, 
                      const Graphable::Link &link,
                      string base_id )
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
        atts += "label = \""+EscapeForGraphviz(Join(labels))+"\"\n"; 
    
    // GraphViz output
	string s;
	s += base_id;
    if( block.specify_ports )
        s += ":" + SeqField(port_index);
	s += " -> ";
	s += Id(link.child_node.get());
	s += " ["+atts+"];\n";
	return s;
}


string Graph::DoHeader()
{
	string s;
	s += "digraph Inferno {\n"; // g is name of graph
	s += "graph [\n";
	s += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
	s += "size = \"14,20\"\n"; // make it smaller
  //  s += "concentrate = \"true\"\n"; 
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
           
    if( g->GetGraphBlockInfo().block_type == Graphable::NODE )
        return nullptr;
        
    return g;
}


string Graph::Id( const void *p )
{
	char s[20];
	sprintf(s, "\"%p\"", p );
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


string Graph::GetPreRestriction(const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>((TreePtr<Node>)*ptr) )   // is to a special node
        {
            if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchitype()) ) )    // pre-restrictor is nontrivial
            {
                return (**(sbs->GetPreRestrictionArchitype())).GetRender();
            }
        }
    }
    return "";
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
