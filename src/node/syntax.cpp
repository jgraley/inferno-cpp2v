#include "syntax.hpp"
#include "common/common.hpp"

Syntax::Production operator+(Syntax::Production p, int i)
{
	return (Syntax::Production)((int)p + i);
}
