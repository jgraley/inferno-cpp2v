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
int x;
void (recurser)(auto int i);
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);

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

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);
