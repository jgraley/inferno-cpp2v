#ifndef INDENTER_HPP
#define INDENTER_HPP

#include "common/common.hpp"

namespace VN 
{
class Indenter
{
public:	
	void AddLinesFromString( string s );
	void DoIndent();
	string GetString() const;
	
private:
	int GetBracketBalance(string s) const;

	struct Line
	{
		int depth;
		string text;
	};
	
	list<Line> lines;
};
};

#endif

