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
void (f)(auto int i);
int x;
int y;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
(++ ::TopLevel::x)&&( ::TopLevel::y=({  ::TopLevel::f( ::TopLevel::x);  ::TopLevel::f_return; }));
(!(++ ::TopLevel::x))||( ::TopLevel::y+=({  ::TopLevel::f( ::TopLevel::x);  ::TopLevel::f_return; }));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=({  ::TopLevel::f( ::TopLevel::x);  ::TopLevel::f_return; })) : ( ::TopLevel::y-=({  ::TopLevel::f( ::TopLevel::x);  ::TopLevel::f_return; }));
cease(  ::TopLevel::y );
return ;
}

void (TopLevel::f)(int i)
{
{
 ::TopLevel::f_return=((100)/i);
return ;
}
}
