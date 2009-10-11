#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "helpers/pass.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"


class Graph : public Pass
{
public:
    void operator()( shared_ptr<Program> program )
    {
    	operator()( (shared_ptr<Node>)program );
    }

    void operator()( shared_ptr<Node> program )
    {
        string s;
        s += "digraph g {\n"; // g is name of graph
        s += "graph [\n";
        s += "rankdir = \"LR\"\n"; // left-to-right looks more like source code
        s += "];\n";
        s += "node [\n";
        s += "shape = \"record\"\n"; // nodes can be split into fields
        s += "style=\"rounded,filled\"\n"; 
        s += "];\n";
        
        {
            Walk w( program );
            while(!w.Done())
            {
                shared_ptr<Node> n = w.Get();
                if( n )
                    s += DoNode(n);                            
                w.AdvanceInto(); 
            }
        }
        
        {
            Walk w( program );
            while(!w.Done())
            {
                shared_ptr<Node> n = w.Get();
                if( n )
                    s += DoNodeLinks(n);                            
                w.AdvanceInto(); 
            }
        }
        
        s += "}\n";
        
        if( ReadArgs::outfile.empty() )
        {
            puts( s.c_str() );
        }
        else
        {
            FILE *fp = fopen( ReadArgs::outfile.c_str(), "wt" );
            ASSERT( fp && "Cannot open output file" );
            fputs( s.c_str(), fp );
            fclose( fp );
        }    
    }
    
    string Id( shared_ptr<Node> n )
    {
        char s[20];
        sprintf(s, "\"%08X\"", (unsigned)(n.get()) );
        return s;
    }    
    
    string SeqField( int i, int j=0 )
    {
        char s[20];
        sprintf( s, "%c%d", 'a'+i, j );
        return s;
    }
    
    string Name( shared_ptr<Node> sp, bool *bold )    
    {
        *bold=true;
        if( shared_ptr<Named> ii = dynamic_pointer_cast<Named>(sp) )
            return ii->name;                     
        else if( shared_ptr<String> ss = dynamic_pointer_cast< String >(sp) )
            return "\\\"" + ss->value + "\\\"";                     // todo sanitise this
        else if( shared_ptr<Integer> ic = dynamic_pointer_cast< Integer >(sp) )
            return string(ic->value.toString(10)); 
        else if( shared_ptr<Float> fc = dynamic_pointer_cast< Float >(sp) )
        {
            char hs[256];
            fc->value.convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
            return string(hs); 
        }           
        else if( shared_ptr<FloatSemantics> fs = dynamic_pointer_cast< FloatSemantics >(sp) )
        {
            return SSPrintf("@%p", fs->value );
        }           
        else
        {
            *bold = false;
            return TypeInfo(sp).name();
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
        else if( dynamic_pointer_cast<Program>(n) )
            return "lemonchiffon4";                     
        else
            return "white";                     
    }
    
    string DoNode( shared_ptr<Node> n )
    {
        string s;
        bool bold;
        s += Id(n) + " [\n";
        s += "label = \"<fixed> " + Name(n, &bold);

        vector< Itemiser::Element * > members = Itemiser::Itemise(n);
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
            else
            	s += " | <" + SeqField( i ) + "> ";
        }
                
        s += "\"\n";
        s += "fillcolor=" + Colour(n) + "\n"; 
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
        vector< Itemiser::Element * > members = Itemiser::Itemise(n);
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
                ASSERT(!"got something from itemise that isnt a sequence, collection or a shared pointer");
            }
        }        
        return s;
    }    
    
    string DoLink( shared_ptr<Node> from, string field, shared_ptr<Node> to )
    {
        string s;
        s += Id(from) + ":" + field + " -> " + Id(to) + ":fixed [\n";
        s += "];\n";
        return s;
    }
};

#endif
