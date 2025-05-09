#include "common.hpp"

#include "standard.hpp"
#include "trace.hpp"

#include <string>

string GetInnermostTemplateParam( string s )
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


string RemoveAllTemplateParam( string s )
{
    string::size_type n;
    for( n=0; n<s.size(); n++ )
    {
        if( s[n] == '<' )        
        {
            string::size_type nn;
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


string RemoveOneOuterScope( string s )
{
    string::size_type n = s.find("::");
    if( n != string::npos )
        s = s.substr( n+2 );
    return s;
}


void RemoveCommonPrefix( string &s1, string &s2 )
{
    while( !s1.empty() && !s2.empty() && s1[0] == s2[0] )
    {
        s1 = s1.substr(1);
        s2 = s2.substr(1);
    }
}


string RemoveOuterTemplate( string s )
{
    string::size_type open_pos = s.find('<');
    string::size_type close_pos = s.rfind('>');
    if( open_pos != string::npos && close_pos != string::npos )
        s = s.substr(open_pos+1, close_pos-(open_pos+1));
    return s;
}


const vector<bool> index_range_bool = {false, true};

