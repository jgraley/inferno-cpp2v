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
private:
void *link;
public:
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
/*temp*/ int f_return;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto int switch_value;
/*temp*/ int muxtemp;
/*temp*/ void *temp_link;
auto int switch_value_1;
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
switch_value=(0);
goto *(((0)==switch_value) ? (&&CASE_1) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value) ? (&&CASE) : (&&PROCEED_1));
PROCEED_1:;
goto *(((1)==switch_value) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE:;
 ::TopLevel::x=(44);
goto CASE_1;
CASE_1:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value_1=(2);
goto *(((2)==switch_value_1) ? (&&CASE_2) : (&&PROCEED_2));
PROCEED_2:;
goto *(((1)==switch_value_1) ? (&&PROCEED_CASE_1) : (&&PROCEED_CASE_1));
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
goto THEN_ELSE_BREAK;
CASE_2:;
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED_3));
PROCEED_3:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED_4));
PROCEED_4:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
goto ELSE;
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
goto THEN_ELSE_BREAK;
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
