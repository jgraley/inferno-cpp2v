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


void Graph::operator()( RootedSearchReplace *root )
{
	string s;
	s += Header();
	s += Traverse( root, false );
	s += Traverse( root, true );
	FOREACH( const Coupling &pc, root->couplings )
		s += DoCoupling( pc );
	s += Footer();
	Disburse( s );
}


TreePtr<Node> Graph::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	(void)context; // Not needed!!

	string s;
	s += Header();
	s += Traverse( root, false );
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
	Walk w( root );
	FOREACH( TreePtr<Node> n, w )
	{
		if( n )
			s += links_pass ? DoNodeLinks(n) : DoNode(n);
	}
	return s;
}


string Graph::Traverse( RootedSearchReplace *sr, bool links_pass )
{
	string s;
	s += links_pass ? DoSearchReplaceLinks(sr) : DoSearchReplace(sr, Id(sr));
	if( sr->search_pattern )
		s += Traverse( sr->search_pattern, links_pass );
	if( sr->replace_pattern )
		s += Traverse( sr->replace_pattern, links_pass );
	for( int j=0; j<sr->slaves.size(); j++ )
		s += Traverse( sr->slaves[j], links_pass );
	return s;
}


string Graph::DoSearchReplace( RootedSearchReplace *sr,
		                       string id,
		                       bool slave,
		                       TreePtr<Node> through )
{
	string s;
	s += id;
	s += " [\n";

	s += "label = \"<fixed> RootedSearchReplace";
	if( slave )
		s += " | <" + SeqField(2) + "> through";
	s += " | <" + (slave ? SeqField(0) : string("search")) + "> search";
	s += " | <" + (slave ? SeqField(1) : string("replace")) + "> replace";
	for( int j=0; j<sr->slaves.size(); j++ ) // TODO obsolete
	{
		char c[20];
		sprintf(c, "%d", j+1);
		s += " | <slave" + string(c) + "> slave " + string(c);
	}
	s += "\"\n";

	s += "shape = \"record\"\n"; // nodes can be split into fields
	s += "style = \"filled\"\n";
	s += "fontsize = \"16\"\n";
	s += "];\n";

	return s;
}


string Graph::DoSearchReplaceLinks( RootedSearchReplace *sr )
{
	string s;
	if( sr->search_pattern )
	{
		s += Id(sr) + ":search -> " + Id(sr->search_pattern.get());
		s += " [];\n";
	}
	if( sr->replace_pattern )
	{
		s += Id(sr) + ":replace -> " + Id(sr->replace_pattern.get());
		s += " [];\n";
	}
	for( int j=0; j<sr->slaves.size(); j++ )
	{
		char c[20];
		sprintf(c, "%d", j+1);
		s += Id(sr) + ":slave" + string(c) + " -> " + Id(sr->slaves[j]) + ":fixed [\n";
		s += "];\n";
	}
	return s;
}


string Graph::DoCoupling( Coupling pc )
{
	string s;
	TreePtr<Node> prev;
	FOREACH( TreePtr<Node> n, pc )
	{
		if( prev )
		{
			string common_atts;
			common_atts = "dir = \"none\"\n"; // no arrowhead
			common_atts += "constraint = false\n"; // Do not make dest higher rank than source - stops dest being moved to the right
			string atts;
			atts = common_atts;
			string c = Colour(prev);
			if(c=="")
				c = Colour(n);
			if(c=="")
				c = "grey60";// make it pale
			atts += "color = " + c + "\n";
			s += DoLink( prev, "", n, atts );
			if( ReadArgs::hack_graph )
			{
				// Graphviz 2.20 has a bug where you get "Lost edge" for some edges that have
				// constraint = false (apparently ok in 2.18 and as-yet-unreleased 2.22)
				// A workaround seems to be to supply another edge going in opposite direction.
				// HOWEVER: using this can make dot crash, on graphs over a certain size. So
				// the hack is only enabled when -g<x>h is specified
				atts = common_atts;
				atts += "style = \"invis\"\n";
				s += DoLink( n, "", prev, atts );
			}
		}
		prev = n;
	}
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
	sprintf( s, "%c%d", 'a'+i, j );
	return s;
}


string Graph::Sanitise( string s )
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
	else if( dynamic_pointer_cast<TransformToBase>(sp) )
	{
		*shape = "hexagon";
		return *sp;
	}
	else if( TreePtr<SoftMakeIdentifier> smi = dynamic_pointer_cast<SoftMakeIdentifier>(sp) )
	{
		*shape = "parallelogram";
		return smi->format;
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
				s += "  <TD>" + Sanitise(*seq) + "[" + string(c) + "]</TD>\n";
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
			s += "  <TD>" + Sanitise(*col) + "{}</TD>\n";
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
	if( TreePtr<RootedSlaveBase> rsb = dynamic_pointer_cast<RootedSlaveBase>(n) )
	{
		TreePtr<RootedSlaveBase> rsr = (TreePtr<RootedSlaveBase>)rsb;
		return DoSearchReplace( rsr.get(), Id( n.get() ), true, rsb->GetThrough() );
	}

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
	string s;
	vector< Itemiser::Element * > members = n->Itemise();
	for( int i=0; i<members.size(); i++ )
	{
		TRACE("Size %d i=%d\n", members.size(), i );

		if( CollectionInterface *col = dynamic_cast<CollectionInterface *>(members[i]) )
		{
			FOREACH( const TreePtrInterface &p, *col )
				s += DoLink( n, SeqField(i), p );
		}
		else if( SequenceInterface *seq = dynamic_cast<SequenceInterface *>(members[i]) )
		{
			int j=0;
			FOREACH( const TreePtrInterface &p, *seq )
				s += DoLink( n, SeqField(i, j++), p );
		}
		else if( TreePtrInterface *ptr = dynamic_cast<TreePtrInterface *>(members[i]) )
		{
			if( *ptr )
				s += DoLink( n, SeqField(i), *ptr );
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

string Graph::DoLink( TreePtr<Node> from, string field, TreePtr<Node> to, string atts )
{
	string s;
	s += Id(from.get());
	if( field != "" && IsRecord(from) )
	{
		s += ":" + field;
		atts = "dir = \"both\"\n";
		atts = "arrowtail = \"dot\"\n";
	}

	s += " -> ";
	s += Id(to.get());
	s += " ["+atts+"];\n";
	return s;
}

