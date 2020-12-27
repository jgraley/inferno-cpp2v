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
}


void Graph::operator()( Transformation *root )
{    
    string s;
    s += DoHeader();
    s += PopulateFromTransformation(root);	
	s += DoFooter();
	Disburse( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

	string s;
	s += DoHeader();
    unique_filter.Reset();
	s += PopulateFromNode( root, false );
    unique_filter.Reset();
	s += PopulateFromNode( root, true );
	s += DoFooter();
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
	    s += PopulateFromEngine( cr, Id(root), false );
        unique_filter.Reset();
	    s += PopulateFromEngine( cr, Id(root), true );
	}
	else
    {
        ASSERTFAIL("Unknown kind of transformation in graph plotter");
    }
	return s;
}


string Graph::PopulateFromEngine( const Graphable *g, string id, bool links_pass )
{
	string s;
    s += links_pass ? DoEngineLinks(g, id) : DoEngine(g, id);
        
    LambdaFilter block_filter( [&](TreePtr<Node> context,
                                   TreePtr<Node> root) -> bool
    {
        return !ShouldDoBlock(root); // Stop where we will do blocks        
    });
        
    Graphable::Block block = g->GetGraphBlockInfo();
    
    for( const Graphable::SubBlock &sub_block : block.sub_blocks )
    {
        const Graphable::Link &link = sub_block.links.front(); // TODO loop over these
        if( link.child_node )
        {
            Walk w( (TreePtr<Node>)(link.child_node), &unique_filter, &block_filter ); // return each node only once; do not recurse through transformations
            FOREACH( const TreePtrInterface &n, w )
            {              
                Graphable *g = ShouldDoBlock((TreePtr<Node>)n);
                if( g )
                    s += PopulateFromEngine( g, Id( n.get() ), links_pass );
                else
                    s += links_pass ? DoNodeLinks((TreePtr<Node>)n, link.link_style) : DoNode((TreePtr<Node>)n);
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
		if( n )
			s += links_pass ? DoNodeLinks((TreePtr<Node>)n, Graphable::SOLID) : DoNode((TreePtr<Node>)n);
	}
	return s;
}


string Graph::DoEngine( const Graphable *g,
                        string id )
{    
    vector<string> labels;
    vector< TreePtr<Node> > links;
    Graphable::Block block = g->GetGraphBlockInfo();
        
    string name = block.title; 
    int n;
    for( n=0; n<name.size(); n++ )
    {
        if( name[n] == '<' )        
        {
            int nn;
            for( nn=n; nn<name.size(); nn++ )
            {            
                if( name[nn] == '>' )        
                {
                    name = name.substr( 0, n ) + name.substr( nn+1 );
                    break;
                }
            }
        }
    }
        
    string s;
	s += id;
	s += " [\n";

    s += "label = \"<fixed> " + Sanitise( name );
    int k=0;
	for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        string label_text = sub_block.item_name + sub_block.item_extra;
		s += " | <" +  SeqField(k) + "> " + label_text;
        k++;
    }
	s += "\"\n";

	s += "shape = \"record\"\n"; // nodes can be split into fields
	s += "style = \"filled\"\n";
	s += "fontsize = \"" FS_MIDDLE "\"\n";
	s += "];\n";

	return s;
}


string Graph::DoEngineLinks( const Graphable *g, string id )
{
    vector<string> labels;
    vector< TreePtr<Node> > links;
    Graphable::Block block = g->GetGraphBlockInfo();

    string s;
    int k=0;
	for( Graphable::SubBlock sub_block : block.sub_blocks )
    {
        const Graphable::Link &link = sub_block.links.front(); // TODO loop over these

        // Atts
        string atts;
        atts += LinkStyleAtt(link.link_style);                        
        if( !link.link_labels.empty() )
            atts += "label = \""+Sanitise(Join(link.link_labels))+"\"\n";             
   
        // GraphViz output
        s += id;
        if( block.port_type==Graphable::ENUMERATED )
            s += ":" +  SeqField(k);
        s += " -> " + Id(link.child);
        s += " [" + atts + "];\n";
        k++;
    }

    return s;
}


string Graph::DoNode( TreePtr<Node> n )
{
	string s;
	bool bold;
	string shape;
	s += Id(n.get()) + " [\n";

    Graphable::Block block = GetDefaultNodeBlockInfo( n );

	s += "shape = \"" + block.shape + "\"\n";
	if(shape == "record" || block.shape == "plaintext")
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


string Graph::DoNodeLinks( TreePtr<Node> n, Graphable::LinkStyle link_style )
{
    Graphable::Block block = GetDefaultNodeBlockInfo( n );
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
                      const Graphable::Block &block, 
                      const Graphable::SubBlock &sub_block, 
                      const Graphable::Link &link )
{
    // Atts
    string atts;
    atts += LinkStyleAtt(link.link_style);
    if( !link.link_labels.empty() )
        atts += "label = \""+Sanitise(Join(link.link_labels))+"\"\n";    

    // GraphViz output
	string s;
	s += Id(block.base);
	if( block.port_type==Graphable::ENUMERATED )
        s += ":" + SeqField(port_index);
	s += " -> ";
	s += Id(link.child);
	s += " ["+atts+"];\n";
	return s;
}


Graphable::Block Graph::GetDefaultNodeBlockInfo( TreePtr<Node> n )
{
	Graphable::Block block;
    block.title = Name( n, &block.bold, &block.shape );
    if( IsRecord(n) )
        block.port_type = Graphable::ENUMERATED;
    else
        block.port_type = Graphable::SHARED;   
    block.colour = Colour( n );
    block.base = n.get();

    vector< Itemiser::Element * > members = n->Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
            int j=0;
			FOREACH( const TreePtrInterface &p, *seq )
			{
                Graphable::SubBlock sub_block = { Sanitise(seq->GetName(), true), 
                                                  SSPrintf("[%d]", j++),
                                                  {} };
                Graphable::Link link;
                link.link_labels.push_back( GetPreRestriction( (TreePtr<Node>)p, &p ) );                    
                if( ReadArgs::graph_trace )
                    link.link_labels.push_back( PatternLink( n, &p ).GetName() );
                link.child = p.get();
                link.child_node = (TreePtr<Node>)p;
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
			}
		}
		else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
            string dots;
            for( int j=0; j<col->size(); j++ )
                dots += ".";
            
            Graphable::SubBlock sub_block = { Sanitise(col->GetName(), true), 
                                              "{" + dots + "}",
                                              {} };
			FOREACH( const TreePtrInterface &p, *col )
            {
                Graphable::Link link;
                link.link_labels.push_back( GetPreRestriction( (TreePtr<Node>)p, &p ) );                                    
                if( ReadArgs::graph_trace )
                    link.link_labels.push_back( PatternLink( n, &p ).GetName() );
                link.child = p.get();
                link.child_node = (TreePtr<Node>)p;
                sub_block.links.push_back( link );
            }
            block.sub_blocks.push_back( sub_block );
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			if( *ptr )
			{
                Graphable::SubBlock sub_block = { Sanitise(ptr->GetName(), true), 
                                                  "",
                                                  {} };
                Graphable::Link link;
                link.link_labels.push_back( GetPreRestriction( (TreePtr<Node>)*ptr, ptr ) );                    
                if( ReadArgs::graph_trace )
                    link.link_labels.push_back( PatternLink( n, ptr ).GetName() );          
                link.child = ptr->get();
                link.child_node = (TreePtr<Node>)*ptr;
                sub_block.links.push_back( link );
                block.sub_blocks.push_back( sub_block );
   		    }
            else if( ReadArgs::graph_trace )
			{
                Graphable::SubBlock sub_block = { Sanitise(ptr->GetName(), true), 
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


string Graph::DoHTMLLabel( string name, const list<Graphable::SubBlock> &sub_blocks )
{
    
	string s = "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n";
	s += " <TR>\n";
	s += "  <TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + Sanitise(name, true) + "</FONT></TD>\n";
	s += "  <TD></TD>\n";
	s += " </TR>\n";
    
    int porti=0;
    for( Graphable::SubBlock sub_block : sub_blocks )
    {
        s += " <TR>\n";
        s += "  <TD>" + sub_block.item_name + "</TD>\n";
        s += "  <TD PORT=\"" + SeqField( porti ) + "\">" + sub_block.item_extra + "</TD>\n";
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
    
    // Permit agents to set their own appearance
    if( Graphable *graphable = dynamic_cast<Graphable *>(sp.get()) )
		graphable->GetGraphNodeAppearance( bold, &text, shape );    

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


Graphable *Graph::ShouldDoBlock( TreePtr<Node> node )
{
    Graphable *g = dynamic_cast<Graphable *>(node.get());
    if( !g )
        return nullptr; // Need Graphable to do a block
           
    if( g->GetGraphBlockInfo().sub_blocks.empty() ) // Don't do block if it would be devoid of sub-blocks    
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


string Graph::Sanitise( string s, bool remove_template )
{
	if( remove_template ) // get rid of templates
    {
        while(true)
        {
            string::size_type iopen = s.find("<");
            string::size_type iclose = s.find(">");
            if( iopen == std::string::npos || iclose == std::string::npos )
                break; // done
            iopen++; // get past <
            s = s.substr( iopen, iclose-iopen );
        }
    }

	string o;
	int n = s.find("::");
	if( n != string::npos )
	    s = s.substr( n+2 );
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
		FILE *fp = fopen( outfile.c_str(), "wt" );
		ASSERT( fp )( "Cannot open output file \"%s\"", outfile.c_str() );
		fputs( s.c_str(), fp );
		fclose( fp );
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


string Graph::GetPreRestriction(TreePtr<Node> node, const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>(node) )   // is to a special node
        {
            if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchitype()) ) )          // pre-restrictor is nontrivial
            {
                return (**(sbs->GetPreRestrictionArchitype())).GetRender();
            }
        }
    }
    return "";
}