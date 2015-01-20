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
void T();
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
int x;
private:
void *link;
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
goto *(((0)==switch_value) ? (&&CASE_2) : (&&PROCEED));
PROCEED:;
goto *((switch_value==(4)) ? (&&CASE_1) : (&&PROCEED_1));
PROCEED_1:;
goto *((switch_value==(1)) ? (&&CASE) : (&&PROCEED_2));
PROCEED_2:;
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
auto int switch_value_1;
switch_value_1=(2);
{
goto *((switch_value_1==(2)) ? (&&CASE_4) : (&&PROCEED_3));
PROCEED_3:;
goto *(((1)==switch_value_1) ? (&&CASE_3) : (&&PROCEED_4));
PROCEED_4:;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_3;
CASE_4:;
{
goto *((!(((0)== ::TopLevel::x)||( ::TopLevel::x==(2)))) ? (&&THEN_1) : (&&PROCEED_5));
PROCEED_5:;
{
/*temp*/ int result;
/*temp*/ int muxtemp;
{
goto *((!(false)) ? (&&THEN) : (&&PROCEED_6));
PROCEED_6:;
muxtemp=(88);
goto ELSE;
THEN:;
{
/*temp*/ int result_1;
{
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
result_1= ::TopLevel::f_return;
muxtemp=result_1;
}
ELSE:;
}
result=muxtemp;
 ::TopLevel::x=result;
}
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
