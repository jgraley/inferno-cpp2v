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
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
private:
void *link;
public:
/*temp*/ int f_return;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ void *temp_link;
/*temp*/ int result_1;
auto int switch_value;
auto int switch_value_1;
/*temp*/ int muxtemp;
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
switch_value=(0);
goto *((switch_value==(0)) ? (&&CASE_1) : (&&PROCEED));
PROCEED:;
goto *((switch_value==(4)) ? (&&CASE) : (&&PROCEED_1));
PROCEED_1:;
goto *((switch_value==(1)) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE:;
 ::TopLevel::x=(44);
CASE_1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value_1=(2);
goto *((switch_value_1==(2)) ? (&&CASE_2) : (&&PROCEED_2));
PROCEED_2:;
goto *((switch_value_1==(1)) ? (&&PROCEED_CASE_1) : (&&PROCEED_CASE_1));
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
goto BREAK_1;
CASE_2:;
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE) : (&&PROCEED_3));
PROCEED_3:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED_4));
PROCEED_4:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
goto THEN_ELSE;
THEN_ELSE:;
goto BREAK_1;
BREAK_1:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
