#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
void *link;
public:
/*temp*/ void *f_link;
int y;
/*temp*/ void *f_link_1;
private:
int i;
public:
void T();
/*temp*/ int f_i;
/*temp*/ int f_return;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool ortemp;
/*temp*/ bool result;
/*temp*/ void *temp_link;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int muxtemp;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ bool result_5;
/*temp*/ int result_6;
/*temp*/ bool andtemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result_1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_1);
goto THEN_ELSE;
THEN_ELSE:;
result=andtemp;
result;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_1);
goto ENTER_f;
LINK_1:;
result_3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_3);
PROCEED_ELSE:;
result_5=ortemp;
result_5;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN_1) : (&&PROCEED_1));
PROCEED_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_2);
goto ENTER_f;
LINK_2:;
result_4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_4);
goto ELSE;
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK_3);
goto ENTER_f;
LINK_3:;
result_2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_2);
ELSE:;
result_6=muxtemp;
result_6;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
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
