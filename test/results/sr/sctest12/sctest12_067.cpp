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
auto void *state;
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
wait(SC_ZERO_TIME);
{
state=(((1)==switch_value) ? (&&PROCEED_CASE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
{
state=(((0)==switch_value) ? (&&CASE) : (&&PROCEED_1));
goto *(state);
}
PROCEED_1:;
{
state=(((4)==switch_value) ? (&&CASE_1) : (&&PROCEED_CASE));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(&&BREAK);
goto *(state);
}
CASE_1:;
 ::TopLevel::x=(44);
{
state=(&&CASE);
goto *(state);
}
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(&&BREAK);
goto *(state);
}
BREAK:;
switch_value_1=(2);
{
state=(((1)==switch_value_1) ? (&&PROCEED_CASE_1) : (&&PROCEED_2));
goto *(state);
}
PROCEED_2:;
{
state=(((2)==switch_value_1) ? (&&CASE_2) : (&&PROCEED_CASE_1));
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
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED_3));
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
 ::TopLevel::f_link=(&&LINK);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result_1=muxtemp;
 ::TopLevel::x=result_1;
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
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
