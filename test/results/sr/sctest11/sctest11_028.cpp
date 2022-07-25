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
int y;
void (f)(auto int i, auto void *link);
/*temp*/ int f_return;
/*temp*/ void *f_link;
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
 ::TopLevel::f( ::TopLevel::x, &&LINK_1);
LINK_1:;
}
 ::TopLevel::f_return; }));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK_2);
LINK_2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
 ::TopLevel::f( ::TopLevel::x, &&LINK_3);
LINK_3:;
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
