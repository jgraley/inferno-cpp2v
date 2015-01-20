#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
int i;
public:
int y;
/*temp*/ void *f_link;
int x;
void (f)();
/*temp*/ int f_return;
private:
void *link;
public:
/*temp*/ void *f_link_1;
/*temp*/ int f_i;
void T();
};
TopLevel top_level("top_level");

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
{
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
{
temp_link= ::TopLevel::link;
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
 ::TopLevel::f_link_1=(&&LINK);
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
 ::TopLevel::f_link_1=(&&LINK_1);
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
 ::TopLevel::f_link_1=(&&LINK_2);
 ::TopLevel::f();
}
}
LINK_2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link_1=(&&LINK_3);
 ::TopLevel::f();
}
}
LINK_3:;
}
 ::TopLevel::f_return; }));
cease(  ::TopLevel::y );
return ;
}
