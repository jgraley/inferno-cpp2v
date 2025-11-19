#include "indenter.hpp"

using namespace VN;

void Indenter::AddLinesFromString( string s )
{	
	string::size_type next_start = 0;
	string::size_type found_pos;
    while ((found_pos = s.find("\n", next_start)) != string::npos)
    {
		lines.push_back( {0, s.substr(next_start, found_pos-next_start)} );
		next_start = found_pos+1;		
	}
	
	// Be flexible on newline at EOF
	if( next_start < s.size() )
		lines.push_back( {0, s.substr(next_start)} );	
}


void Indenter::DoIndent()
{
	int current_depth = 0;
	for( Line &line : lines )
	{
		int bal = GetBracketBalance(line.text);
		
		// Inclusive detections 		   
		if( bal < 0 )
			current_depth += bal;
					
		// Update current line
		ASSERT( current_depth >= 0 );
		line.depth = current_depth;
		
		// Exclusive detections
		if( bal > 0 )
			current_depth += bal;
	}
	ASSERT( current_depth == 0 );
}


string Indenter::GetString() const
{
	// Tabs give more cosistent indentation in Geany when using 
	// Unicode (which makes spaces vary for come ewason)
	string s;
	for( const Line &line : lines )
		s += string(line.depth, '\t') + line.text + "\n";
	return s;
}


int Indenter::GetBracketBalance(string s) const
{
	// Note: <> for templates is not so easy
	int opens = Count("(", s) + Count("[", s) + Count("{", s) + Count("【", s);
	int closes = Count(")", s) + Count("[", s) + Count("}", s) + Count("】", s);
	return opens - closes;
}

