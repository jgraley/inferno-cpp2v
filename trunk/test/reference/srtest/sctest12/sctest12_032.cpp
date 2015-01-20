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
/*temp*/ void *f_link;
int x;
/*temp*/ void *f_link_1;
void T();
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result_1;
auto int switch_value;
auto int switch_value_1;
/*temp*/ void *temp_link;
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
switch_value_1=(0);
goto *((switch_value_1==(0)) ? (&&CASE_2) : (&&PROCEED));
PROCEED:;
goto *((switch_value_1==(4)) ? (&&CASE_1) : (&&PROCEED_1));
PROCEED_1:;
goto *((switch_value_1==(1)) ? (&&CASE) : (&&PROCEED_2));
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
BREAK:;
BREAK_1:;
switch_value=(2);
goto *((switch_value==(2)) ? (&&CASE_4) : (&&PROCEED_3));
PROCEED_3:;
goto *((switch_value==(1)) ? (&&CASE_3) : (&&PROCEED_4));
PROCEED_4:;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_3;
CASE_4:;
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_1) : (&&PROCEED_5));
PROCEED_5:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED_6));
PROCEED_6:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
goto BREAK_2;
BREAK_2:;
BREAK_3:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
