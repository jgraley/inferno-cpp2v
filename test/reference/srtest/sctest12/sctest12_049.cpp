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
int x;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto void *state;
auto int switch_value;
auto int switch_value1;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result1;
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
state=(((0)==switch_value) ? (&&CASE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
{
state=(((4)==switch_value) ? (&&CASE1) : (&&PROCEED1));
goto *(state);
}
PROCEED1:;
{
state=(((1)==switch_value) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(&&BREAK);
goto *(state);
}
CASE1:;
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
switch_value1=(2);
{
state=(((2)==switch_value1) ? (&&CASE2) : (&&PROCEED2));
goto *(state);
}
PROCEED2:;
{
state=(((1)==switch_value1) ? (&&PROCEED_CASE1) : (&&PROCEED_CASE1));
goto *(state);
}
PROCEED_CASE1:;
 ::TopLevel::x=(99);
{
state=(&&THEN_ELSE_BREAK);
goto *(state);
}
CASE2:;
{
state=((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED3));
goto *(state);
}
PROCEED3:;
{
state=((!(false)) ? (&&THEN) : (&&PROCEED4));
goto *(state);
}
PROCEED4:;
muxtemp=(88);
{
state=(&&ELSE);
goto *(state);
}
THEN:;
 ::TopLevel::f_link1=(&&LINK);
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
result1=muxtemp;
 ::TopLevel::x=result1;
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
