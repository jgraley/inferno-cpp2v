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
/*temp*/ void *f_link;
void T();
/*temp*/ int f_return;
/*temp*/ void *f_link_1;
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
auto int switch_value;
auto int switch_value_1;
/*temp*/ void *temp_link;
/*temp*/ int result;
/*temp*/ int result_1;
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
wait(SC_ZERO_TIME);
goto *(((0)==switch_value_1) ? (&&CASE_1) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value_1) ? (&&CASE) : (&&PROCEED_1));
PROCEED_1:;
goto *(((1)==switch_value_1) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE:;
 ::TopLevel::x=(44);
goto CASE_1;
CASE_1:;
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value=(2);
goto *(((2)==switch_value) ? (&&CASE_2) : (&&PROCEED_2));
PROCEED_2:;
goto *(((1)==switch_value) ? (&&PROCEED_CASE_1) : (&&PROCEED_CASE_1));
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
goto THEN_ELSE_BREAK;
CASE_2:;
goto *((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED_3));
PROCEED_3:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED_4));
PROCEED_4:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result_1= ::TopLevel::f_return;
muxtemp=result_1;
goto ELSE;
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
goto THEN_ELSE_BREAK;
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
