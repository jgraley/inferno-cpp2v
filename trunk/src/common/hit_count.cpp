#include "hit_count.hpp"
#include "read_args.hpp"

HitCount HitCount::instance; 

bool HitCount::enable = false; ///< call HitCount::Enable(true) to begin counting hits

bool operator<( const HitCount::Category &l, const HitCount::Category &r )
{
    // prioritise the comparisons in a way that makes for a nice dump when dumped in order
    for( int i=0; i<ReadArgs::hits_format.size(); i++ )
    {
        switch( ReadArgs::hits_format[i] )
        {
            case 'S':
            if( l.step != r.step )
                return l.step < r.step;
            break;
            
            case 'I':
            case 'N':
            if( l.instance != r.instance )
                return l.instance < r.instance;
            break;
            
            case 'F':
            if( l.file != r.file )
                return l.file < r.file;
            break;
            
            case 'L':
            if( l.line != r.line )
                return l.line < r.line;
            break;
            
            case 'M':
            if( l.function != r.function )
                return l.function < r.function;
            break;
            
            default: // lower case letter have no effect
                break;
        }
    }
    return false;
}

void HitCount::Usage()
{
    fprintf(stderr,
            "Hit count format string:\n\n"
            "Letters after -th correspond to fields displayed in the dump.\n"
            "Dump is sorted according to the capital letters.\n"
            "left-most fields take precidence. Letters are:\n\n"
            "S Step number\n"
            "I Instance address\n"
            "F source file name\n"
            "L line number in source\n"
            "M function/method name\n"
            "n number of hits in this category\n\n");
    exit(1);
}

void HitCount::Check()
{
    if( ReadArgs::hits_format == string("?") )
    {
        Usage();
    }
}

void HitCount::Dump()
{
    FOREACH( pc p, counter )
    {
        for( int i=0; i<ReadArgs::hits_format.size(); i++ )
        {
            switch( ReadArgs::hits_format[i] )
            {
                case 'S':
                if( p.first.step>=0 )
                    printf("step %d", p.first.step );                   
                else
                    printf("pre/post");
                break;
                
                case 'I':
                printf("%s", p.first.instance->GetAddr().c_str() );                   
                break;
                
                case 'N':
                printf("%s", p.first.instance->GetName().c_str() );                   
                break;

                case 'F':
                printf("%s", p.first.file.c_str() );
                break;
                
                case 'L':
                printf(":%d", p.first.line );         
                break;
                
                case 'M':
                printf("in %s", p.first.function.c_str() );    
                break;
                
                case 'n':
                printf("%u hits", p.second );  
                break;
                
                default:
                Usage();
                break;
            }
            if( i+1<ReadArgs::hits_format.size() )            
                printf(" ");            
        }
        printf("\n");               
    }                 
}

void HitCount::Enable( bool e )
{
    enable = e;
}


