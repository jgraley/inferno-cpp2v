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
/*temp*/ int f_return;
private:
void *link;
public:
int y;
int x;
/*temp*/ void *f_link_1;
/*temp*/ int f_i;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool andtemp;
/*temp*/ bool result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ bool ortemp;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ bool result_6;
/*temp*/ int muxtemp;
/*temp*/ void *temp_link;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result_4= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_4);
goto THEN_ELSE;
THEN_ELSE:;
result_6=andtemp;
result_6;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
goto PROCEED_ELSE;
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_1);
goto ENTER_f;
LINK_1:;
result_2= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_2);
PROCEED_ELSE:;
result=ortemp;
result;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN_1) : (&&PROCEED_1));
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_2);
goto ENTER_f;
LINK_2:;
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_1);
goto ELSE;
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_3);
goto ENTER_f;
LINK_3:;
result_5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_5);
ELSE:;
result_3=muxtemp;
result_3;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
