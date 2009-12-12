#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "helpers/transformation.hpp"
#include "helpers/search_replace.hpp"
#include "helpers/soft_patterns.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"


class Graph : public Transformation
{
public:
    void operator()( RootedSearchReplace *root )
    {
        string s;
        s += Header();
        s += Traverse( root, false );
        s += Traverse( root, true );
        FOREACH( const RootedSearchReplace::Coupling *pc, root->matches )
            s += DoCoupling( pc );
        s += Footer();
        Disburse( s );
    }

    shared_ptr<Node> operator()( shared_ptr<Node> context, shared_ptr<Node> root )
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

    string Header()
    {
        string s;
        s += "digraph Inferno {\n"; // g is name of graph
        s += "graph [\n";
        s += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
        s += "];\n";
        s += "node [\n";
        s += "];\n";
        return s;
    }

    string Footer()
    {
        string s;
        s += "}\n";
        return s;
    }

    void Disburse( string s )
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

    string Traverse( shared_ptr<Node> root, bool links_pass )
    {
    	string s;
        Walk w( root );
        while(!w.Done())
        {
            shared_ptr<Node> n = w.Get();
            if( n )
                s += links_pass ? DoNodeLinks(n) : DoNode(n);
            w.AdvanceInto();
        }
        return s;
    }
    
    string Traverse( RootedSearchReplace *sr, bool links_pass )
    {
    	string s;
    	s += links_pass ? DoSearchReplaceLinks(sr) : DoSearchReplace(sr);
		if( sr->search_pattern )
		    s += Traverse( sr->search_pattern, links_pass );
		if( sr->replace_pattern )
		    s += Traverse( sr->replace_pattern, links_pass );
        for( int j=0; j<sr->slaves.size(); j++ )
        	s += Traverse( sr->slaves[j], links_pass );
    	return s;
    }

	string DoSearchReplace( RootedSearchReplace *sr )
	{
		string s;
		s += Id( sr );
		s += " [\n";

		s += "label = \"<fixed> RootedSearchReplace | <search> Search | <replace> Replace";
        for( int j=0; j<sr->slaves.size(); j++ )
        {
            char c[20];
            sprintf(c, "%d", j+1);
            s += " | <slave" + string(c) + "> Slave " + string(c);
        }
		s += "\"\n";

        s += "shape = \"record\"\n"; // nodes can be split into fields
        s += "style = \"filled\"\n";
        s += "fontsize = \"16\"\n";
		s += "];\n";

		return s;
	}

	string DoSearchReplaceLinks( RootedSearchReplace *sr )
	{
		string s;
		if( sr->search_pattern )
		{
			s += Id(sr) + ":search -> " + Id(sr->search_pattern.get()) + ":fixed [\n";
			s += "];\n";
		}
		if( sr->replace_pattern )
		{
			s += Id(sr) + ":replace -> " + Id(sr->replace_pattern.get()) + ":fixed [\n";
			s += "];\n";
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

	string DoCoupling( const RootedSearchReplace::Coupling *pc )
    {
		string s;
		shared_ptr<Node> prev;
		FOREACH( shared_ptr<Node> n, *pc )
		{
			if( prev )
			{
		        s += Id(prev.get()) + ":fixed -> " + Id(n.get()) + ":fixed [\n";
		        s += "dir = \"none\"\n"; // no arrowhead
		        s += "style = \"dotted\"\n"; // dotted line
		        s += "constraint = false\n"; // Do not make dest higher rank than source - stops dest being moved to the right
		        s += "];\n";
		        // Graphviz 2.20 has a bug where you get "Lost edge" for some edges that have
		        // constraint = false (apparently ok in 2.18 and as-yet-unreleased 2.22)
		        // A workaround seems to be to supply another edge going in opposite direction.
		        s += Id(n.get()) + ":fixed -> " + Id(prev.get()) + ":fixed [\n";
		        s += "dir = \"none\"\n";
		        s += "style = \"invis\"\n";
		        s += "constraint = false\n";
		        s += "];\n";
			}
			prev = n;
		}
		return s;
    }


    string Id( void *p )
    {
        char s[20];
        sprintf(s, "\"%08X\"", (unsigned)(p) );
        return s;
    }    
    
    string SeqField( int i, int j=0 )
    {
        char s[20];
        sprintf( s, "%c%d", 'a'+i, j );
        return s;
    }
    
    string Sanitise( string s )
    {
        string o;
        for( int i=0; i<s.size(); i++ )
        {
            if( s[i] == '\"' )
            	o += '\\';
            o += s[i];
        }
        return o;
    }

    string Name( shared_ptr<Node> sp, bool *bold, bool *circle )   // TODO put stringize capabilities into the Property nodes as virtual methods
    {
        *bold=true;
        *circle=false;
        if( dynamic_pointer_cast<RootedSearchReplace::StarBase>(sp) )
        {
            *circle = true;
            return string("*");
        }
        else if( dynamic_pointer_cast<RootedSearchReplace::StuffBase>(sp) )
        {
            *circle = true;
            return string("#");
        }
        else if( dynamic_pointer_cast<NotMatchBase>(sp) )
        {
            *circle = true;
            return string("!");
        }
        else if( dynamic_pointer_cast<MatchAllBase>(sp) )
        {
            *circle = true;
            return string("&&"); // note & is a wildcard in dot but not handled prolperly, this becomes "& "
        }
//        else if( shared_ptr<SpecificString> ss = dynamic_pointer_cast< SpecificString >(sp) )
  //          return "\\\"" + ss->value + "\\\"";                     // TODO sanitise the string
//        else if( shared_ptr<SpecificInteger> ic = dynamic_pointer_cast< SpecificInteger >(sp) )
  //          return string(ic->value.toString(10));
        else if( shared_ptr<SpecificFloat> fc = dynamic_pointer_cast< SpecificFloat >(sp) )
        {
            char hs[256];
            ((llvm::APFloat)*fc).convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
            return string(hs); 
        }           
        else
        {
            *bold = false;
            return Sanitise( *sp );
        }
    }
    
    // Colours are GraphVis colours as listed at http://www.graphviz.org/doc/info/colors.html
    string Colour( shared_ptr<Node> n )
    {
        if( dynamic_pointer_cast<Declaration>(n) )
            return "chocolate1";                     
        else if( dynamic_pointer_cast<Type>(n) )
            return "plum";                     
        else if( dynamic_pointer_cast<Literal>(n) )
            return "cyan";                     
        else if( dynamic_pointer_cast<Expression>(n) )
            return "seagreen1";                     
        else if( dynamic_pointer_cast<Statement>(n) )
            return "gold";                     
        else if( dynamic_pointer_cast<Property>(n) )
            return "khaki";                     
        else if( dynamic_pointer_cast<Scope>(n) )
            return "lemonchiffon4";                     
        else
            return "white";                     
    }


    
    string DoNode( shared_ptr<Node> n )
    {
        string s;
        bool bold, circle;
        s += Id(n.get()) + " [\n";
        string name = Name(n, &bold, &circle);
        if(circle)
        {
            s += "label = \"" + name + "\"\n";
        }
        else
        {
            s += "label = \"<fixed> " + name;
            vector< Itemiser::Element * > members = n->Itemise();
            for( int i=0; i<members.size(); i++ )
            {
                if( GenericSequence *seq = dynamic_cast<GenericSequence *>(members[i]) )                
                {
                    for( int j=0; j<seq->size(); j++ )
                    {
                        char c[20];
                        sprintf(c, "%d", j+1);
                        s += " | <" + SeqField( i, j ) + "> " + string(c) + ".";
                    }
                }            
                else  if( GenericSharedPtr *ptr = dynamic_cast<GenericSharedPtr *>(members[i]) )
                {
                	s += " | <" + SeqField( i ) + "> ";
                	if( !*ptr )
                		s += "NULL";
                }
                else // Collection
                {
                	s += " | <" + SeqField( i ) + "> ";
                }
            }      
            s += "\"\n";
        }          

        s += "style = \"filled,rounded\"\n";
        if(circle)
        {
            s += "shape = \"circle\"\n";
            s += "fixedsize = true\n";
            s += "width = 0.65\n";
            s += "height = 0.65\n";
        }
        else
        {
            s += "shape = \"record\"\n"; // nodes can be split into fields
            s += "fillcolor = " + Colour(n) + "\n"; 
        }
        if( bold )
            s += "fontsize = \"24\"\n";
        else
            s += "fontsize = \"16\"\n";
    
        s += "];\n";
        return s;
    }
    
    string DoNodeLinks( shared_ptr<Node> n )
    {    
        string s;
        vector< Itemiser::Element * > members = n->Itemise();
        for( int i=0; i<members.size(); i++ )
        {
            TRACE("Size %d i=%d\n", members.size(), i );

        	if( GenericCollection *col = dynamic_cast<GenericCollection *>(members[i]) )
            {
        		FOREACH( const GenericSharedPtr &p, *col )
                    s += DoLink( n, SeqField(i), p );
            }
        	else if( GenericSequence *seq = dynamic_cast<GenericSequence *>(members[i]) )
            {
                int j=0;
        		FOREACH( const GenericSharedPtr &p, *seq )
                    s += DoLink( n, SeqField(i, j++), p );
            }            
            else if( GenericSharedPtr *ptr = dynamic_cast<GenericSharedPtr *>(members[i]) )         
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
    
    string DoLink( shared_ptr<Node> from, string field, shared_ptr<Node> to )
    {
        string s;
        s += Id(from.get()) + ":" + field + " -> " + Id(to.get()) + ":fixed [\n";
        s += "];\n";
        return s;
    }
};

#endif
