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
void T();
/*temp*/ void *f_link_1;
private:
int i;
public:
/*temp*/ int f_i;
/*temp*/ int f_return;
private:
void *link;
public:
int x;
int y;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ bool result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ bool andtemp;
auto void *state;
/*temp*/ bool result_4;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ void *temp_link;
/*temp*/ int result_6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
wait(SC_ZERO_TIME);
{
state=((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
goto *(state);
}
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
{
state=(&&THEN_ELSE);
goto *(state);
}
THEN_ELSE:;
result_1=andtemp;
result_1;
ortemp=(!(++ ::TopLevel::x));
{
state=((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
goto *(state);
}
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_1);
{
state=(&&ENTER_f);
goto *(state);
}
LINK_1:;
result_2= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_2);
{
state=(&&PROCEED_ELSE);
goto *(state);
}
PROCEED_ELSE:;
result_4=ortemp;
result_4;
 ::TopLevel::x=(0);
{
state=((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
{
state=((!( ::TopLevel::x++)) ? (&&THEN_1) : (&&PROCEED_1));
goto *(state);
}
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_2);
{
state=(&&ENTER_f);
goto *(state);
}
LINK_2:;
result_3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_3);
{
state=(&&ELSE);
goto *(state);
}
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_3);
{
state=(&&ENTER_f);
goto *(state);
}
LINK_3:;
result_5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_5);
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result_6=muxtemp;
result_6;
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
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
