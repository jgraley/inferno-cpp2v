#include "hit_count.hpp"
#include "read_args.hpp"

bool operator<( const HitCount::Category &l, const HitCount::Category &r )
{
    // prioritise the comparisons in a way that makes for a nice dump when dumped in order
    for( string::size_type i=0; i<ReadArgs::hits_format.size(); i++ )
    {
        switch( ReadArgs::hits_format[i] )
        {
            case 'S':
            if( l.progress != r.progress )
                return l.progress < r.progress;
            break;
            
            case 'F':
            if( l.file != r.file )
                return l.file < r.file;
            break;
            
            case 'L':
            if( l.line != r.line )
                return l.line < r.line;
            break;
            
            case 'I':
            if( l.instance != r.instance )
                return l.instance < r.instance;
            break;
            
            case 'M':
            if( l.function != r.function )
                return l.function < r.function;
            break;
            
            case 'P':
            if( l.prefix != r.prefix )
                return l.prefix < r.prefix;
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
            "Fields specified first take precidence. Letters are:\n\n"
            "S step number\n"
            "I instance name\n"
            "F source file name\n"
            "L line number in source\n"
            "M function/method name\n"
            "P trace prefix string (=mini-stacktrace)\n");
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
    string fmt = ReadArgs::hits_format;
    int fs = fmt.size();
    for( pc p : counter )
    {
        for( int i=0; i<fs; i++ )
        {
            if( fmt.substr(i, 2)=="FL" )
            {
                printf("%s:%d", p.first.file.c_str(), p.first.line );
                i++;
            }
            else if( fmt.substr(i, 2)=="IM" )
            {
                string i_f = JoinInstanceFunction(p.first.instance, p.first.function);
                printf("%s", i_f.c_str());
                i++;
            }
            else
            {
                switch( fmt[i] )
                {
                    case 'S':
                    printf("At %s", p.first.progress.GetPrefix().c_str() );                   
                    break;
                    
                    case 'F':
                    printf("%s", p.first.file.c_str() );
                    break;
                    
                    case 'L':
                    printf("%d", p.first.line );         
                    break;
                    
                    case 'I':
                    printf("%s", p.first.instance.c_str() );                   
                    break;

                    case 'M':
                    printf("in %s", p.first.function.c_str() );    
                    break;
                    
                    case 'P':
                    printf("%s", p.first.prefix.c_str() );                   
                    break;
                                    
                    default:
                    Usage();
                    break;
                }
            }
            printf(" ");            
        }
        printf("%u hits", p.second );  
        printf("\n");               
    }                 
}

void HitCount::Enable( bool e )
{
    enable = e;
}


HitCount HitCount::instance; 

bool HitCount::enable = false; ///< call HitCount::Enable(true) to begin counting hits

