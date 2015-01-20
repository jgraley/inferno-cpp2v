#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
private:
void *link;
public:
void T();
/*temp*/ int f_return;
int x;
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
goto CASE_2;
if( switch_value==(4) )
goto CASE_1;
if( switch_value==(1) )
goto CASE;
CASE:;
 ::TopLevel::x=(99);
goto BREAK_1;
CASE_1:;
 ::TopLevel::x=(44);
CASE_2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
}
BREAK:;
}
BREAK_1:;
}
{
{
{
auto int switch_value_1 = 2;
{
if( switch_value_1==(2) )
goto CASE_4;
if( (1)==switch_value_1 )
goto CASE_3;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_3;
CASE_4:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=({ /*temp*/ int muxtemp; if( false )
muxtemp=(88);
else
muxtemp=({ {
{
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
muxtemp; });
goto BREAK_2;
}
}
BREAK_2:;
}
BREAK_3:;
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link_1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
