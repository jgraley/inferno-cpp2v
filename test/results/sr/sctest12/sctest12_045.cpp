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
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
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
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
{
auto int switch_value;
switch_value=(0);
{
if( (1)==switch_value )
goto CASE;
goto PROCEED;
PROCEED:;
if( (0)==switch_value )
goto CASE_1;
goto PROCEED_1;
PROCEED_1:;
if( (4)==switch_value )
goto CASE_2;
goto PROCEED_2;
PROCEED_2:;
CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE_2:;
 ::TopLevel::x=(44);
CASE_1:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK_1;
}
}
BREAK_1:;
}
BREAK:;
}
{
{
{
auto int switch_value_1;
switch_value_1=(2);
{
if( (1)==switch_value_1 )
goto CASE_3;
goto PROCEED_3;
PROCEED_3:;
if( (2)==switch_value_1 )
goto CASE_4;
goto PROCEED_4;
PROCEED_4:;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_2;
CASE_4:;
{
if( !(((0)== ::TopLevel::x)||((2)== ::TopLevel::x)) )
goto THEN;
goto PROCEED_5;
PROCEED_5:;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
if( !(false) )
goto THEN_1;
goto PROCEED_6;
PROCEED_6:;
muxtemp=(88);
goto ELSE;
THEN_1:;
muxtemp=({ {
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
ELSE:;
}
muxtemp; });
goto ELSE_1;
THEN:;
;
ELSE_1:;
}
goto BREAK_3;
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
 ::TopLevel::link= ::TopLevel::f_link;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}