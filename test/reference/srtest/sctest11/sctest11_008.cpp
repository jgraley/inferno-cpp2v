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
int (f)(auto int i);
int x;
int y;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
(++ ::TopLevel::x)&&( ::TopLevel::y=( ::TopLevel::f( ::TopLevel::x)));
(!(++ ::TopLevel::x))||( ::TopLevel::y+=( ::TopLevel::f( ::TopLevel::x)));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=( ::TopLevel::f( ::TopLevel::x))) : ( ::TopLevel::y-=( ::TopLevel::f( ::TopLevel::x)));
cease(  ::TopLevel::y );
return ;
}

int (TopLevel::f)(int i)
{
return (100)/i;
}
