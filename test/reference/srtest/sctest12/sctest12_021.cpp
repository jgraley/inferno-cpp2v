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
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
private:
void *link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
goto BREAK1;
case 4:;
 ::TopLevel::x=(44);
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
BREAK:;
}
BREAK1:;
}
{
{
switch( 2 )
{
case 1:;
 ::TopLevel::x=(99);
goto BREAK3;
case 2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : ({ {
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; }));
goto BREAK2;
}
BREAK2:;
}
BREAK3:;
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
