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
/*temp*/ void *temp_link;
auto int switch_value;
auto int switch_value_1;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result_1;
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
switch_value=(0);
goto *(((1)==switch_value) ? (&&PROCEED_CASE) : (&&PROCEED));
PROCEED:;
goto *(((0)==switch_value) ? (&&CASE) : (&&PROCEED_1));
PROCEED_1:;
goto *(((4)==switch_value) ? (&&CASE_1) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE_1:;
 ::TopLevel::x=(44);
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value_1=(2);
goto *(((1)==switch_value_1) ? (&&PROCEED_CASE_1) : (&&PROCEED_2));
PROCEED_2:;
goto *(((2)==switch_value_1) ? (&&CASE_2) : (&&PROCEED_CASE_1));
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
goto BREAK_1;
CASE_2:;
goto *((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN_ELSE) : (&&PROCEED_3));
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
