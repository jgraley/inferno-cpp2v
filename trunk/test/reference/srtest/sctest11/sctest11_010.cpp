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
/*temp*/ void *f_link;
int y;
/*temp*/ int f_return;
void (f)(auto int i, auto void *link);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
(++ ::TopLevel::x)&&( ::TopLevel::y=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK);
LINK:;
}
 ::TopLevel::f_return; }));
(!(++ ::TopLevel::x))||( ::TopLevel::y+=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK1);
LINK1:;
}
 ::TopLevel::f_return; }));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK2);
LINK2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK3);
LINK3:;
}
 ::TopLevel::f_return; }));
cease(  ::TopLevel::y );
return ;
}

void (TopLevel::f)(int i, void *link)
{
/*temp*/ void *temp_link;
{
 ::TopLevel::f_return=((100)/i);
{
temp_link=link;
return ;
}
}
}