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
{
auto int switch_value = 0;
{
if( switch_value==(0) )
goto CASE2;
if( switch_value==(4) )
goto CASE1;
if( switch_value==(1) )
goto CASE;
CASE:;
 ::TopLevel::x=(99);
goto BREAK1;
CASE1:;
 ::TopLevel::x=(44);
CASE2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
}
BREAK:;
}
BREAK1:;
}
{
{
{
auto int switch_value1 = 2;
{
if( switch_value1==(2) )
goto CASE4;
if( switch_value1==(1) )
goto CASE3;
CASE3:;
 ::TopLevel::x=(99);
goto BREAK3;
CASE4:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=({ /*temp*/ int muxtemp; if( false )
muxtemp=(88);
else
muxtemp=({ {
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
muxtemp; });
goto BREAK2;
}
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
