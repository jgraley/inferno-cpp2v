#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_return;
private:
void *link;
public:
int x;
/*temp*/ void *f_link;
void T();
/*temp*/ void *f_link_1;
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
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
{
auto int switch_value;
switch_value=(0);
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
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
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
auto int switch_value_1;
switch_value_1=(2);
{
if( switch_value_1==(2) )
goto CASE_4;
if( switch_value_1==(1) )
goto CASE_3;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_3;
CASE_4:;
{
if( !(( ::TopLevel::x==(0))||( ::TopLevel::x==(2))) )
goto THEN_1;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
if( !(false) )
goto THEN;
muxtemp=(88);
goto ELSE;
THEN:;
muxtemp=({ {
{
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
ELSE:;
}
muxtemp; });
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
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
