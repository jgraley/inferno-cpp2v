/*
 * graph.cpp
 *
 *  Created on: 18 Jul 2010
 *      Author: jgraley
 */

#include "tree/tree.hpp"
#include "helpers/transformation.hpp"
#include "helpers/search_replace.hpp"
#include "helpers/soft_patterns.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "graph.hpp"

// TODO indicate pre-restriction by putting class name over the link. Only when type is not that
// of the pointer, ie a non-trivial pre-estriction
// TODO indicate Stuff restrictor by making it come out of the top of the circle (note that it will
// be used on search, and usually search Stuff is coupled to replace Stuff, which will be below.
// TODO force ranking to space out graph as
// Primary: Stuff nodes, SearchReplace
// Secondary: Normal nodes and special nodes that occupy space
// Tertiary: CompareReplace and special nodes that do not occupy space

void Graph::operator()( Transformation *root )
{    
    string s;
	s += Header();
    unique_filter.Reset();
	s += Traverse( root, Id(root), false );
    unique_filter.Reset();
	s += Traverse( root, Id(root), true );
	s += Footer();
	Disburse( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

	string s;
	s += Header();
    unique_filter.Reset();
	s += Traverse( root, false );
    unique_filter.Reset();
	s += Traverse( root, true );
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
    s += "fontname = \"arial\"\n"; // get with the 21st century
  //  s += "concentrate = \"true\"\n"; 
	s += "];\n";
	s += "node [\n";
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
	if( ReadArgs::outfile.empty() )
	{
		puts( s.c_str() );
	}
	else
	{
		FILE *fp = fopen( ReadArgs::outfile.c_str(), "wt" );
		ASSERT( fp )( "Cannot open output file " )(ReadArgs::outfile);
		fputs( s.c_str(), fp );
		fclose( fp );
	}
}


string Graph::Traverse( TreePtr<Node> root, bool links_pass )
{
	string s;
    TRACE("Graph plotter traversing intermediate %s pass\n", links_pass ? "links" : "nodes");
	::Traverse w( root );
	FOREACH( TreePtr<Node> n, w )
	{
		if( n )
			s += links_pass ? DoNodeLinks(n) : DoNode(n);
	}
	return s;
}


string Graph::Traverse( Transformation *sr, string id, bool links_pass )
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
            TRACE("Expanding transform pattern ")(*pattern)("\n");
            Expand w( pattern, &unique_filter, &no_tx_filter ); // return each node only once; do not recurse through transformations
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
        
    string name = *sr; 
    int n;
    for( n=0; n<name.size(); n++ )
    {
        if( name[n] == '<' )        
            name = name.substr( 0, n );
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
	s += "fontsize = \"16\"\n";
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
	sprintf(s, "\"%08X\"", (unsigned)(p) );
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
	*bold=true;
	if( dynamic_pointer_cast<StarBase>(sp) )
	{
		*shape = "circle";
		return string("*");
	}
	else if( dynamic_pointer_cast<StuffBase>(sp) )
	{
		*shape = "circle";
		return string("#"); // TODO what if there's a restriction: egg?
	}
	else if( dynamic_pointer_cast<NotMatchBase>(sp) )
	{
		*shape = "circle";
		return string("!");
	}
	else if( dynamic_pointer_cast<MatchAllBase>(sp) )
	{
		*shape = "circle";
		return string("&"); // note & is a wildcard in dot but not handled properly, this becomes "& ". At least some of the time.
	}
	else if( dynamic_pointer_cast<MatchAnyBase>(sp) )
	{
		*shape = "circle";
		return string("|");
	}
	else if( dynamic_pointer_cast<MatchOddBase>(sp) )
	{
		*shape = "circle";
		return string("^");
	}
	else if( dynamic_pointer_cast<TransformOfBase>(sp) )
	{
		*shape = "hexagon";
		return *sp;
	}
	else if( shared_ptr<BuildIdentifierBase> smi = dynamic_pointer_cast<BuildIdentifierBase>(sp) )
	{
	    // TODO indicate whether it's building instance, label or type identifier
		*shape = "parallelogram";
		return smi->format;
	}
	else if( dynamic_pointer_cast<GreenGrassBase>(sp) )
	{
		*shape = "circle";
		return string("||||");
	}
    else if( dynamic_pointer_cast<OverlayBase>(sp) )
    {
        *shape = "circle";
        return string("+-"); // TODO want greek delta symbol
    }
	else
	{
		*bold = false;
		*shape = "plaintext";//"record";
		return *sp;
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
	s += "  <TD><FONT POINT-SIZE=\"24.0\">" + Sanitise(name) + "</FONT></TD>\n";
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
				s += "  <TD>" + Sanitise(*ptr) + "</TD>\n";
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
		    return Traverse( rsb, Id( n.get() ), false );

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
		s += "fontsize = \"16\"\n";
	}
	else
	{
		s += "label = \"" + name + "\"\n";// TODO causes errors because links go to targets meant for records
		s += "style = \"filled\"\n";

		if( shape == "circle" )
		{
			s += "fixedsize = true\n";
			s += "width = 0.55\n";
			s += "height = 0.55\n";
			s += "fontsize = \"24\"\n";
		}
		else
		{
			s += "fontsize = \"20\"\n";
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
            return Traverse( rsb, Id( n.get() ), true );

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
        TRACE("TreePtr %d is @%p\n", i, ptr );
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
			    atts += "label = \"" + (string)**(sbs->GetPreRestrictionArchitype()) + "\"\n";
			}
	}
    else if( dynamic_pointer_cast<StuffBase>(from) )
    {
        if( field == "porta0" )
            s+= ":n";
        else
            s+= ":e";
    }
    else if( dynamic_pointer_cast<OverlayBase>(from) )
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

