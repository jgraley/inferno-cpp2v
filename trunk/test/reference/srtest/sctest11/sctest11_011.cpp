#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int y;
/*temp*/ int f_return;
/*temp*/ void *f_link;
void (f)();
void T();
/*temp*/ void *f_link_1;
/*temp*/ int f_i;
int x;
};
TopLevel top_level("top_level");

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::f_link;
auto int i =  ::TopLevel::f_i;
{
 ::TopLevel::f_return=((100)/i);
{
temp_link=link;
return ;
}
}
}

void TopLevel::T()
{
 ::TopLevel::x=(4);
(++ ::TopLevel::x)&&( ::TopLevel::y=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f();
}
}
LINK:;
}
 ::TopLevel::f_return; }));
(!(++ ::TopLevel::x))||( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_1);
 ::TopLevel::f();
}
}
LINK_1:;
}
 ::TopLevel::f_return; }));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_2);
 ::TopLevel::f();
}
}
LINK_2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_3);
 ::TopLevel::f();
}
}
LINK_3:;
}
 ::TopLevel::f_return; }));
cease(  ::TopLevel::y );
return ;
}
