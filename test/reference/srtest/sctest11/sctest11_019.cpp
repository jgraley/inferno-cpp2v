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
int y;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ void *f_link1;
private:
int i;
void *link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
(++ ::TopLevel::x)&&( ::TopLevel::y=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
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
goto ENTER_f;
}
}
LINK1:;
}
 ::TopLevel::f_return; }));
{
 ::TopLevel::x=(0);
if(  ::TopLevel::x<(2) )
do
{
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
}
}
LINK2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
}
}
LINK3:;
}
 ::TopLevel::f_return; }));
CONTINUE:;
;
}
while(  ::TopLevel::x<(2) );
}
cease(  ::TopLevel::y );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
{
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
