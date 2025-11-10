
#include "vn_lang.location.hpp"
#include "vn_lang.lpp.hpp"
#include "vn_lang.ypp.hpp"

void YY::VNLangParser::error( const location& loc, const string& msg )
{
    cerr << loc << ": " << msg << endl;
    if( loc.begin.line == loc.end.line && loc.begin.line == lexer.lineno() )
    {
        cerr << lexer.matcher().line() << endl;
        for( size_t i = 1; i < loc.begin.column; ++i )
        {
            cerr << " ";
        }
        for( size_t i = loc.begin.column; i < loc.end.column; ++i )
        {
            cerr << "~";
        }
        cerr << endl;
    }
    else
    {
        FILE* file = lexer.in().file(); // the current file being scanned
        if( file != NULL )
        {
            YY::VNLangScanner::Matcher* m = lexer.new_matcher( file ); // new matcher
            lexer.push_matcher( m ); // save the current matcher
            off_t pos = ftell( file ); // save current position in the file
            fseek( file, 0, SEEK_SET ); // go to the start of the file
            for( size_t i = 1; i < loc.begin.line; ++i )
            {
                m->skip( '\n' ); // skip to the next line
            }
            for( size_t i = loc.begin.line; i <= loc.end.line; ++i )
            {
                cerr << m->line() << endl; // display the offending line
                m->skip( '\n' ); // next line
            }
            fseek( file, pos, SEEK_SET ); // restore position in the file to continue scanning
            lexer.pop_matcher(); // restore matcher
        }
    }
    if( lexer.size() == 0 ) // if token is unknown (no match)
    {
        lexer.matcher().winput(); // skip character
    }
}
