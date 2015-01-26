#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
void T();
void (recurser)(auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::recurser(1);
cease(  ::TopLevel::x );
}

void (TopLevel::recurser)(int i)
{
 ::TopLevel::x++;
if( i<(5) )
{
 ::TopLevel::recurser((1)+i);
 ::TopLevel::recurser((1)+i);
}
}
