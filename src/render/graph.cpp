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

// Graph Documantation
//
// The shapes and contents of the displayed nodes is explained in comments in the function 
// Graph::Name(), which may be found below. Ordinary tree nodes are always rectangles with
// soft corners, so this function mainly deals with special nodes as used by Search and Replace.
// The colours are defined in Graph::Colour, which categorises nodes (all nodes, including Special)
// by whether they are derived from particular intermediate nodes, which are easily seen in
// the code of the function.


// TODO indicate pre-restriction by putting class name over the link. Only when type is not that
// of the pointer, ie a non-trivial pre-estriction
// TODO indicate Stuff restrictor by making it come out of the top of the circle (note that it will
// be used on search, and usually search Stuff is coupled to replace Stuff, which will be below.
// TODO force ranking to space out graph as
// Primary: Stuff nodes, SearchReplace
// Secondary: Normal nodes and special nodes that occupy space
// Tertiary: CompareReplace and special nodes that do not occupy space

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
	    s += PopulateFromEngine( cr, nullptr, Id(root), false );
        unique_filter.Reset();
	    s += PopulateFromEngine( cr, nullptr, Id(root), true );
	}
	else
    {
        ASSERTFAIL("Unknown kind of transformation in graph plotter");
    }
	return s;
}


string Graph::PopulateFromEngine( const Graphable *g, TreePtr<Node> nbase, string base_id, bool links_pass )
{
    Graphable::Block gblock = g->GetGraphBlockInfo();
    MyBlock block = PreProcessBlock( gblock, nbase, true );
    //MyBlock block; (Graphable::Block &)block = gblock;    
    
	string s;
    s += links_pass ? DoLinks(block, base_id) : DoEngineBlock(block, base_id);
        
    LambdaFilter block_filter( [&](TreePtr<Node> context,
                                   TreePtr<Node> root) -> bool
    {
        return !ShouldDoEngine(root); // Stop where we will do blocks        
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
                    Graphable *g = ShouldDoEngine(node);
                    if( g )
                    {
                        s += PopulateFromEngine( g, node, Id(node.get()), links_pass );
                    }
                    else
                    {
                        MyBlock child_block = PreProcessBlock( GetNodeBlockInfo( node ), node, false );
                        OverrideLinkStyle( child_block, link.link_style );
                        if( links_pass )
                            s += DoLinks(child_block, Id(node.get()));
                        else
                            s += DoNodeBlock(child_block, Id(node.get()));
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
            s += DoNodeBlock(child_block, Id(n.get()));
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
                                         {} } );
    }
    
    // If there is more than one sub-block, use the rounded rectangle form for clarity
    if( my_block.sub_blocks.size() > 1 )
        my_block.shape = "plaintext";

    // These kinds of blocks require port names to be to be specified so links can tell them apart
    my_block.specify_ports = (my_block.shape=="record" || my_block.shape=="plaintext");  
    
    // Colour the block in accordance with the node if there is one otherwise leave it blank.
    // See #258: "block colour shall be dictated by the node type only"
    if( node )
        my_block.colour = Colour( node );

    // Make the titles more wieldy by removing template stuff - note:
    // different policies for engine blocks vs node blocks.
    if( for_engine_block )
    {
        my_block.title = RemoveAllTemplateParam(my_block.title); 
        my_block.title = RemoveOneOuterScope(my_block.title); 
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
    block.title = Name( n, &block.bold, &block.shape );  
        
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
                                                  {} };
                Graphable::Link link;
                if( ReadArgs::graph_trace )
                link.trace_labels.push_back( PatternLink( n, &p ).GetShortName() );
                link.child_node = (TreePtr<Node>)p;
                link.ptr = &p;
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
                                              {} };
			FOREACH( const TreePtrInterface &p, *col )
            {
                Graphable::Link link;
                link.trace_labels.push_back( PatternLink( n, &p ).GetShortName() );
                link.child_node = (TreePtr<Node>)p;
                link.ptr = &p;
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
                                                  {} };
                Graphable::Link link;
                link.trace_labels.push_back( PatternLink( n, ptr ).GetShortName() );          
                link.child_node = (TreePtr<Node>)*ptr;
                link.ptr = ptr;
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { EscapeForGraphviz(GetInnermostTemplateParam(ptr->GetName())), 
                                                  "NULL",
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


string Graph::DoEngineBlock( const MyBlock &block,
                        string base_id )
{            
    string s;
	s += base_id;
	s += " [\n";

    s += "label = \"<fixed> " + EscapeForGraphviz( block.title );
    int k=0;
	for( Graphable::SubBlock sub_block : block.sub_blocks )
    {        
        string label_text = EscapeForGraphviz(sub_block.item_name + sub_block.item_extra);
		s += " | <" +  SeqField(k) + "> " + label_text;
        k++;
    }
	s += "\"\n";

	s += "shape = \"record\"\n"; // nodes can be split into fields
	s += "style = \"filled\"\n";
	s += "fontsize = \"" FS_MIDDLE "\"\n";
	if(block.colour != "")
		s += "fillcolor = " + block.colour + "\n";
    s += "];\n";

	return s;
}


string Graph::DoNodeBlock( const MyBlock &block, string base_id )
{
	string s;
	s += base_id + " [\n";

	s += "shape = \"" + block.shape + "\"\n";
	if( block.shape == "record" || block.shape == "plaintext" )
	{
		s += "label = " + DoHTMLLabel( block.title, block.sub_blocks );
		s += "style = \"rounded,filled\"\n";
		s += "fontsize = \"" FS_SMALL "\"\n";
	}
	else
	{
		s += "label = \"" + block.title + "\"\n";// TODO causes errors because links go to targets meant for records
		s += "style = \"filled\"\n";

		if( block.shape == "circle" || block.shape=="triangle" )
		{
			s += "fixedsize = true\n";
			s += "width = " NS_SMALL "\n";
			s += "height = " NS_SMALL "\n";
			s += "fontsize = \"" FS_LARGE "\"\n";
		}
		else
		{
			s += "fontsize = \"" FS_MIDDLE "\"\n";
		}
	}

	if(block.colour != "")
		s += "fillcolor = " + block.colour + "\n";

	s += "];\n";
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
    bool trace_lables_to_head = !link.trace_labels.empty() && ReadArgs::graph_trace;
    if( !link.labels.empty() )
        atts += "label = \""+EscapeForGraphviz(Join(link.labels))+"\"\n"; 
    else if( trace_lables_to_head )
       atts += "label = \"     \"\n"; // Make a little room for head label

    if( trace_lables_to_head )
        atts += "headlabel = \""+EscapeForGraphviz(Join(link.trace_labels))+"\"\n";    

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


string Graph::DoHTMLLabel( string name, const list<Graphable::SubBlock> &sub_blocks )
{
    
	string s = "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n";
	s += " <TR>\n";
	s += "  <TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + EscapeForGraphviz(name) + "</FONT></TD>\n";
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


string Graph::Name( TreePtr<Node> sp, bool *bold, string *shape )   // TODO put stringize capabilities into the Property nodes as virtual methods
{
	// normal nodes and agents are represented as a rectangle with curved corners. At the top of the rectangle, 
	// in large font, is the name of the node's type OR the identifier name if the node is a kind of 
	// SpecificIdentifier. All TreePtr<>, Sequence<> and Collection<> members are listed below in a 
	// smaller font. The name of the pointed-to type is given (not the member's name, Inferno cannot deduce
	// this). 
	// Collections appear once and are followed by {...} where the number of dots equals the number of 
	// elements in the Collection.
	// Sequences appear once for each element in the sequence. Each appearance is followed by [i] where
	// i is the index, starting from 0.
	// All child pointers emerge from *approximately* the right of the corresponding member name. I cannot
	// for the life of me get GraphViz to make the lines begin *on* the right edge of the rectangle. They 
	// always come from some way in from the right edge, and if they are angled up or down, they can appear
	// to be coming from the wrong place.        
	string text = sp->GetRender();     
	*bold = false;
	*shape = "plaintext";//"record";

    return text;
}


// Colours are GraphVis colours as listed at http://www.graphviz.org/doc/info/colors.html
string Graph::Colour( TreePtr<Node> n )
{
	if( dynamic_pointer_cast<Identifier>(n) )
		return "gray60";
	else if( dynamic_pointer_cast<Declaration>(n) )
		return "plum";
	else if( dynamic_pointer_cast<Initialiser>(n) )
		return "plum";
	else if( dynamic_pointer_cast<MapOperand>(n) )
		return "goldenrod2";
	else if( dynamic_pointer_cast<Type>(n) )
		return "seagreen1";
	else if( dynamic_pointer_cast<Literal>(n) )
		return "goldenrod2";
	else if( dynamic_pointer_cast<Expression>(n) )
		return "chocolate1";
	else if( dynamic_pointer_cast<Property>(n) )
		return "olivedrab3";
	else if( dynamic_pointer_cast<Statement>(n) )
		return "brown1";
	else if( dynamic_pointer_cast<Scope>(n) )
		return "cyan";
	else
		return "";
}


bool Graph::IsRecord( TreePtr<Node> n )
{
	bool bold;
	string shape;
	bool fport, tport;
	Name( n, &bold, &shape );
	return shape=="record" || shape=="plaintext";
}


Graphable *Graph::ShouldDoEngine( TreePtr<Node> node )
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


void Graph::OverrideLinkStyle( MyBlock &dest, Graphable::LinkStyle link_style )
{
    for( Graphable::SubBlock &sub_block : dest.sub_blocks )    
        for( Graphable::Link &link : sub_block.links )
            link.link_style = link_style;    
}
