#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
private:
void *link;
public:
/*temp*/ void *f_link1;
void T();
int y;
/*temp*/ int f_return;
/*temp*/ int f_i;
int x;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool ortemp;
/*temp*/ int result;
auto void *state;
/*temp*/ bool result1;
/*temp*/ void *temp_link;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ bool andtemp;
/*temp*/ bool result4;
/*temp*/ int result5;
/*temp*/ int muxtemp;
/*temp*/ int result6;
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
result2= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result2);
{
state=(&&THEN_ELSE);
goto *(state);
}
THEN_ELSE:;
result4=andtemp;
result4;
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
result1=ortemp;
result1;
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
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
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
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result3=muxtemp;
result3;
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