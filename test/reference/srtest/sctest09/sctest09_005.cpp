#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
void (recurser)(auto int i);
int x;
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
 ::TopLevel::recurser(i+(1));
 ::TopLevel::recurser(i+(1));
}
}
