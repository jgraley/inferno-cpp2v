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
int i;
void *link;
public:
int x;
int y;
/*temp*/ int f_i;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result4;
/*temp*/ bool result1;
/*temp*/ int result5;
/*temp*/ int result;
/*temp*/ int result6;
/*temp*/ int result3;
/*temp*/ int result2;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result5);
{
state=(&&THEN_ELSE);
goto *(state);
}
THEN_ELSE:;
result1=andtemp;
result1;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
goto *(state);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
{
state=(&&ENTER_f);
goto *(state);
}
LINK1:;
result6= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result6);
{
state=(&&PROCEED_ELSE);
goto *(state);
}
PROCEED_ELSE:;
result4=ortemp;
result4;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED1));
goto *(state);
}
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK2);
{
state=(&&ENTER_f);
goto *(state);
}
LINK2:;
result2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result2);
{
state=(&&ELSE);
goto *(state);
}
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
{
state=(&&ENTER_f);
goto *(state);
}
LINK3:;
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result3);
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result=muxtemp;
result;
{
state=(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
