#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (f)();
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
void T();
/*temp*/ int f_i;
int x;
/*temp*/ int f_return;
int y;
};
TopLevel top_level("top_level");

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto int i;
auto void *link;
link= ::TopLevel::f_link1;
i= ::TopLevel::f_i;
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
 ::TopLevel::f_link1=(&&LINK);
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
 ::TopLevel::f_link1=(&&LINK1);
 ::TopLevel::f();
}
}
LINK1:;
}
 ::TopLevel::f_return; }));
for(  ::TopLevel::x=(0);  ::TopLevel::x<(2);  )
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link1=(&&LINK2);
 ::TopLevel::f();
}
}
LINK2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link1=(&&LINK3);
 ::TopLevel::f();
}
}
LINK3:;
}
 ::TopLevel::f_return; }));
cease(  ::TopLevel::y );
return ;
}
