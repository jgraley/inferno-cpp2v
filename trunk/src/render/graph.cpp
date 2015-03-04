/*
 * graph.cpp
 *
 *  Created on: 18 Jul 2010
 *      Author: jgraley
 */

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/search_replace.hpp"
#include "sr/soft_patterns.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "graph.hpp"
#include "steps/inferno_patterns.hpp"
#include <inttypes.h>

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

#define FS_SMALL "12"
#define FS_MIDDLE "14"
#define FS_LARGE "16"
#define NS_SMALL "0.4"
//#define FONT "Arial"

Graph::Graph( string of ) :
    outfile(of)
{
}

void Graph::operator()( Transformation *root )
{    
    string s;
    s += Header();
    s += MakeGraphTx(root);	
	s += Footer();
	Disburse( s );
}

string Graph::MakeGraphTx(Transformation *root)
{
    string s;
    if( TransformationVector *tv = dynamic_cast<TransformationVector *>(root) )
    {
        FOREACH( shared_ptr<Transformation> t, *tv )
            s = MakeGraphTx( t.get() ) + s; // seem to have to pre-pend to get them appearing in the right order
    }
    else
    {
        unique_filter.Reset();
	    s += UniqueWalk( root, Id(root), false );
        unique_filter.Reset();
	    s += UniqueWalk( root, Id(root), true );
	}
	return s;
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

	string s;
	s += Header();
    unique_filter.Reset();
	s += UniqueWalk( root, false );
    unique_filter.Reset();
	s += UniqueWalk( root, true );
	s += Footer();
	Disburse( s );

	return root; // no change
}


string Graph::Header()
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


string Graph::Footer()
{
	string s;
	s += "}\n";
	return s;
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


string Graph::UniqueWalk( TreePtr<Node> root, bool links_pass )
{
	string s;
    TRACE("Graph plotter traversing intermediate %s pass\n", links_pass ? "links" : "nodes");
	::UniqueWalk w( root );
	FOREACH( TreePtr<Node> n, w )
	{
		if( n )
			s += links_pass ? DoNodeLinks(n) : DoNode(n);
	}
	return s;
}


string Graph::UniqueWalk( Transformation *sr, string id, bool links_pass )
{
	string s;
    s += links_pass ? DoTransformationLinks(sr, id) : DoTransformation(sr, id);
    
    struct : public Filter
    {
        virtual bool IsMatch( TreePtr<Node> context,
                              TreePtr<Node> root )
        {
            return !( !dynamic_pointer_cast<TransformOfBase>(root) && 
                      !dynamic_pointer_cast<BuildIdentifierBase>(root) && 
                      dynamic_cast<Transformation*>(root.get()) );
        }
    } no_tx_filter;
    
    vector<string> labels;
    vector< TreePtr<Node> > links;
    (void)sr->GetGraphInfo( &labels, &links );
    
    FOREACH( TreePtr<Node> pattern, links )
    {
        if( pattern )
        {
            TRACE("Walking transform pattern ")(*pattern)("\n");
            Walk w( pattern, &unique_filter, &no_tx_filter ); // return each node only once; do not recurse through transformations
            FOREACH( TreePtr<Node> n, w )
            {              
                s += links_pass ? DoNodeLinks(n) : DoNode(n);
            }
        }
    }
	return s;
}


string Graph::DoTransformation( Transformation *sr,
		                        string id )
{    
    vector<string> labels;
    vector< TreePtr<Node> > links;
    sr->GetGraphInfo( &labels, &links );
        
    string name = sr->GetName(); 
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
	for( int i=0; i<labels.size(); i++ )        
		s += " | <" + labels[i] + "> " + labels[i];
	s += "\"\n";

	s += "shape = \"record\"\n"; // nodes can be split into fields
	s += "style = \"filled\"\n";
	s += "fontsize = \"" FS_MIDDLE "\"\n";
	s += "];\n";

	return s;
}


string Graph::DoTransformationLinks( Transformation *sr, string id )
{
    vector<string> labels;
    vector< TreePtr<Node> > links;
    sr->GetGraphInfo( &labels, &links );

    string s;
    for( int i=0; i<labels.size(); i++ )        
        s += id + ":" + labels[i] + " -> " + Id(links[i].get()) + " [];\n";

    return s;
}


string Graph::Id( void *p )
{
	char s[20];
	sprintf(s, "\"%p\"", p );
	return s;
}


string Graph::SeqField( int i, int j )
{
	char s[20];
	sprintf( s, "port%c%d", 'a'+i, j );
	return s;
}


string Graph::Sanitise( string s, bool remove_tp )
{
	string o;
	if( remove_tp ) // get rid of TreePtr<>
	    s = s.substr( 8, s.size()-9 );
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


string Graph::Name( TreePtr<Node> sp, bool *bold, string *shape )   // TODO put stringize capabilities into the Property nodes as virtual methods
{
    // This function does not deal directly with SearchReplace, CompareReplace, SlaveSearchReplace or 
    // SlaveCompareReplace. These appear in sharp-cornered rectangles, with the name at the top and the
    // member TreePtr names below. These may be some combination of search, compare, replace and through
    // and their links are approximately to the right.
	*bold=true;
	if( dynamic_pointer_cast<StarAgent>(sp) )
	{
	    // The Star node appears as a small circle with a * character inside it. * is chosen for its role in 
	    // filename wildcarding, which is semantically equiviant only when used in a Sequence.
		*shape = "circle";
		return string("*");
	}
	else if( TreePtr<StuffAgent> stuff = dynamic_pointer_cast<StuffAgent>(sp) )
	{
	    // The Stuff node appears as a small circle with a # character inside it. The terminus link emerges from the
	    // right of the circle. If there is a recurse restriction the circle is egg-shaped and the restriction link 
	    // emerges from the top of the egg shape. # is chosen (as is the name Stuff) for its similarity to * because
	    // the nodes are both able to wildcard multiple nodes in the tree.
		if( stuff->recurse_restriction )
		    *shape = "egg";
		else
     	    *shape = "circle";
		return string("#"); 
	}
	else if( dynamic_pointer_cast<AnyNodeAgent>(sp) )
	{
	    // The AnyNode node appears as a small circle with a ? sign in it. The terminus link emerges from the
	    // right of the circle. ? implies the tendancy to match exactly one thing.
   	    *shape = "circle";
		return string("?"); 
	}
	else if( dynamic_pointer_cast<NotMatchAgent>(sp) )
	{
	    // The NotMatch node appears as a small circle with an ! character inside it. The affected subtree is 
	    // on the right.
	    // NOTE this and the next few special nodes are the nodes that control the action of the search engine in 
	    // Inferno search/replace. They are not the nodes that represent the operations in the program being processed.
	    // Those nodes would appear as rounded rectangles with the name at the top. The nmes may be found in
	    // src/tree/operator_db.txt  
		*shape = "circle";
		return string("!");
	}
	else if( dynamic_pointer_cast<MatchAllAgent>(sp) )
	{
	    // The MatchAll node appears as a small circle with an & character inside it. The affected subtrees are 
	    // on the right.
		*shape = "circle";
		return string("&"); // note & is a wildcard in dot but not handled properly, this becomes "& ". At least some of the time.
	}
	else if( dynamic_pointer_cast<MatchAnyAgent>(sp) )
	{
	    // The MatchAny node appears as a small circle with an | character inside it. The affected subtrees are 
	    // on the right.
		*shape = "circle";
		return string("|");
	}
    else if( shared_ptr<TransformOfBase> tob = dynamic_pointer_cast<TransformOfBase>(sp) )
    {
        // The TransformOf node appears as a slightly flattened hexagon, with the name of the specified 
        // kind of Transformation class inside it.
        *shape = "hexagon";
        return *(tob->transformation);
    }
    else if( dynamic_pointer_cast<PointerIsBase>(sp) )
    {
        // The TransformOf node appears as a slightly flattened hexagon, with the name of the specified 
        // kind of Transformation class inside it.
        *shape = "pentagon";
        return string("pointer is"); 
    }
	else if( shared_ptr<BuildIdentifierBase> smi = dynamic_pointer_cast<BuildIdentifierBase>(sp) )
	{
	    // The BuildIdentifier node appears as a parallelogram (rectangle pushed to the side) with
	    // the printf format string that controls the name of the generated identifier inside it.
	    // TODO indicate whether it's building instance, label or type identifier
		*shape = "parallelogram";
		return smi->format;
	}
	else if( shared_ptr<IdentifierByNameBase> ibnb = dynamic_pointer_cast<IdentifierByNameBase>(sp) )
	{
	    // The IdentifierByNameBase node appears as a trapezium (rectangle narrower at the top) with
	    // the string that must be matched inside it.
	    // TODO indicate whether it's matching instance, label or type identifier
		*shape = "trapezium";
		return ibnb->name;
	}
	else if( dynamic_pointer_cast<GreenGrassAgent>(sp) )
	{
	    // The GreenGrass node appears as a small circle containing four vertical line characters,
	    // like this: ||||. These are meant to represent the blades of grass. It was late and I was
	    // tired.
		*shape = "circle";
		return string("||||");
	}
    else if( dynamic_pointer_cast<OverlayAgent>(sp) )
    {
        // The Overlay node is shown as a small triangle, with the through link on the right and the overlay link
        // coming out of the bottom.
        *shape = "triangle";
        return string(""); 
    }
    else if( dynamic_pointer_cast<SoftAgent>(sp) )
    {
        *bold = false;
        *shape = "plaintext";//"record";
        return sp->GetName();
    }
    else if( dynamic_pointer_cast<NormalAgent>(sp) )
    {
        // All the other nodes are represented as a rectangle with curved corners. At the top of the rectangle, 
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
        *bold = false;
        *shape = "plaintext";//"record";
        return sp->GetName();
    }
	else
    {
        ASSERT(false)(sp)(" ")(*sp)(" ")(sp->GetName());
        ASSERTFAIL("Unknown node in graph plotter");
    }
}


// Colours are GraphVis colours as listed at http://www.graphviz.org/doc/info/colors.html
string Graph::Colour( TreePtr<Node> n )
{
	if( dynamic_pointer_cast<Identifier>(n) )
		return "gray60";
	else if( dynamic_pointer_cast<Declaration>(n) )
		return "seagreen1";
	else if( dynamic_pointer_cast<MapOperand>(n) )
		return "goldenrod2";
	else if( dynamic_pointer_cast<Type>(n) )
		return "plum";
	else if( dynamic_pointer_cast<Literal>(n) )
		return "goldenrod2";
	else if( dynamic_pointer_cast<Expression>(n) )
		return "chocolate1";
	else if( dynamic_pointer_cast<Property>(n) )
		return "olivedrab3";
	else if( dynamic_pointer_cast<Statement>(n) )
		return "brown1";
	else if( dynamic_pointer_cast<Initialiser>(n) )
		return "olivedrab3";
	else if( dynamic_pointer_cast<Scope>(n) )
		return "cyan";
	else
		return "";
}


string Graph::HTMLLabel( string name, TreePtr<Node> n )
{
	string s = "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n";
	s += " <TR>\n";
	s += "  <TD><FONT POINT-SIZE=\"" FS_LARGE ".0\">" + Sanitise(name) + "</FONT></TD>\n";
	s += "  <TD></TD>\n";
	s += " </TR>\n";
	vector< Itemiser::Element * > members = n->Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
			for( int j=0; j<seq->size(); j++ )
			{
				char c[20];
				sprintf(c, "%d", j);
				s += " <TR>\n";
				s += "  <TD>" + Sanitise(*seq, true) + "[" + string(c) + "]</TD>\n";
				s += "  <TD PORT=\"" + SeqField( i, j ) + "\"></TD>\n";
				s += " </TR>\n";
			}
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			if( *ptr )
			{
				s += " <TR>\n";
				s += "  <TD>" + Sanitise(ptr->GetName(), true) + "</TD>\n";
				s += "  <TD PORT=\"" + SeqField( i ) + "\"></TD>\n";
				s += " </TR>\n";
		   }
		}
		else if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
			s += " <TR>\n";
			s += "  <TD>" + Sanitise(*col, true) + "{";
            for( int j=0; j<col->size(); j++ )
                s += ".";
            s += "}</TD>\n";
			s += "  <TD PORT=\"" + SeqField( i ) + "\"></TD>\n";
			s += " </TR>\n";
		}
		else
		{
			ASSERT(0);
		}
	}
	s += "</TABLE>>\n";
	return s;
}


string Graph::SimpleLabel( string name, TreePtr<Node> n )
{
	string s = "\"<fixed> " + Sanitise(name);
	vector< Itemiser::Element * > members = n->Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
			for( int j=0; j<seq->size(); j++ )
			{
				s += " | <" + SeqField( i, j ) + "> ";
			}
		}
		else  if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			s += " | <" + SeqField( i ) + "> ";
		}
		else // Collection
		{
			s += " | <" + SeqField( i ) + "> ";// \\{\\}";
		}
	}
	s += "\"\n";
	return s;
}


string Graph::DoNode( TreePtr<Node> n )
{
    if( !dynamic_pointer_cast<TransformOfBase>(n) && !dynamic_pointer_cast<BuildIdentifierBase>(n) ) // ignire the fact that these also derive from Transformation
  	    if( Transformation *rsb = dynamic_cast<Transformation *>(n.get()) )
		    return UniqueWalk( rsb, Id( n.get() ), false );

	string s;
	bool bold;
	string shape;
	s += Id(n.get()) + " [\n";
	string name = Name(n, &bold, &shape);

	s += "shape = \"" + shape + "\"\n";
	if(shape == "record" || shape == "plaintext")
	{
		s += "label = " + HTMLLabel( name, n );
		s += "style = \"rounded,filled\"\n";
		s += "fontsize = \"" FS_SMALL "\"\n";
	}
	else
	{
		s += "label = \"" + name + "\"\n";// TODO causes errors because links go to targets meant for records
		s += "style = \"filled\"\n";

		if( shape == "circle" || shape=="triangle" )
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

	string c = Colour(n);
	if(c!="")
		s += "fillcolor = " + c + "\n";

	s += "];\n";
	return s;
}


string Graph::DoNodeLinks( TreePtr<Node> n )
{
    if( !dynamic_pointer_cast<TransformOfBase>(n) && !dynamic_pointer_cast<BuildIdentifierBase>(n) ) // ignire the fact that these also derive from Transformation
        if( Transformation *rsb = dynamic_cast<Transformation *>(n.get()) )
            return UniqueWalk( rsb, Id( n.get() ), true );

    string s;
    TRACE("Itemising\n");
	vector< Itemiser::Element * > members = n->Itemise();
    TRACE("Doing links for ")(*n)(" size is %d\n", members.size() );
    for( int i=0; i<members.size(); i++ )
	{
		if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
			FOREACH( const TreePtrInterface &p, *col )
				s += DoLink( n, SeqField(i), p, string(), &p );
		}
		else if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
			int j=0;
			FOREACH( const TreePtrInterface &p, *seq )
				s += DoLink( n, SeqField(i, j++), p, string(), &p );
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
            //TRACE("TreePtr %d is @%p\n", i, ptr );
			if( *ptr )
				s += DoLink( n, SeqField(i), *ptr, string(), ptr );
		}
		else
		{
			ASSERT(0)("got something from itemise that isnt a sequence, collection or a shared pointer");
		}
	}
	return s;
}


bool Graph::IsRecord( TreePtr<Node> n )
{
	bool bold;
	string shape;
	bool fport, tport;
	Name( n, &bold, &shape );
	return shape=="record" || shape=="plaintext";
}

string Graph::DoLink( TreePtr<Node> from, string field, TreePtr<Node> to, string atts, const TreePtrInterface *ptr )
{
	string s;
	s += Id(from.get());
	if( field != "" && IsRecord(from) )
	{
		s += ":" + field;
		atts = "dir = \"both\"\n";
        atts = "arrowtail = \"dot\"\n";
      //  atts = "sametail = \"" + field + "\"\n";
		if( ptr )									// is normal tree link
		    if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>(to) )   // is to a special node
		        if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchitype()) ) )          // pre-restrictor is nontrivial
			{
			    atts += "label = \"" + (**(sbs->GetPreRestrictionArchitype())).GetName() + "\"\n";
			}
	}
    else if( dynamic_pointer_cast<StuffAgent>(from) )
    {
        if( field == "porta0" )
            s+= ":n";
        else
            s+= ":e";
    }
    else if( dynamic_pointer_cast<OverlayAgent>(from) )
    {
        if( field == "portb0" )
            s+= ":s";
        else
            s+= ":e";
    }

	s += " -> ";
	s += Id(to.get());
	s += " ["+atts+"];\n";
	return s;
}

