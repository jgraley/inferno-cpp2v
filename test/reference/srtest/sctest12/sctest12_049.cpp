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
/*temp*/ int f_return;
private:
void *link;
public:
/*temp*/ void *f_link_1;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ void *temp_link;
auto int switch_value;
auto void *state;
/*temp*/ int result_1;
auto int switch_value_1;
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
wait(SC_ZERO_TIME);
{
state=((switch_value==(0)) ? (&&CASE_1) : (&&PROCEED));
goto *(state);
}
PROCEED:;
{
state=((switch_value==(4)) ? (&&CASE) : (&&PROCEED_1));
goto *(state);
}
PROCEED_1:;
{
state=((switch_value==(1)) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(&&BREAK);
goto *(state);
}
CASE:;
 ::TopLevel::x=(44);
{
state=(&&CASE_1);
goto *(state);
}
CASE_1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(&&BREAK);
goto *(state);
}
BREAK:;
switch_value_1=(2);
{
state=((switch_value_1==(2)) ? (&&CASE_2) : (&&PROCEED_2));
goto *(state);
}
PROCEED_2:;
{
state=((switch_value_1==(1)) ? (&&PROCEED_CASE_1) : (&&PROCEED_CASE_1));
goto *(state);
}
PROCEED_CASE_1:;
 ::TopLevel::x=(99);
{
state=(&&THEN_ELSE_BREAK);
goto *(state);
}
CASE_2:;
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED_3));
goto *(state);
}
PROCEED_3:;
{
state=((!(false)) ? (&&THEN) : (&&PROCEED_4));
goto *(state);
}
PROCEED_4:;
muxtemp=(88);
{
state=(&&ELSE);
goto *(state);
}
THEN:;
 ::TopLevel::f_link_1=(&&LINK);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result_1= ::TopLevel::f_return;
muxtemp=result_1;
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
{
state=(&&THEN_ELSE_BREAK);
goto *(state);
}
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
