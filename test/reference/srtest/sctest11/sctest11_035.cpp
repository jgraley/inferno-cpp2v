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
/*temp*/ int f_return;
int y;
/*temp*/ int f_i;
/*temp*/ void *f_link;
private:
void *link;
int i;
public:
/*temp*/ void *f_link_1;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool result;
/*temp*/ int result_1;
/*temp*/ bool andtemp;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int muxtemp;
/*temp*/ int result_5;
/*temp*/ bool ortemp;
/*temp*/ bool result_6;
/*temp*/ void *temp_link;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
LINK:;
result_4= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_4);
goto THEN_ELSE;
THEN_ELSE:;
result_6=andtemp;
result_6;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1=(&&LINK_1);
goto ENTER_f;
LINK_1:;
result_5= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_5);
ELSE:;
result=ortemp;
result;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1=(&&LINK_2);
goto ENTER_f;
LINK_2:;
result_3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_3);
goto ELSE_1;
THEN_1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link_1=(&&LINK_3);
goto ENTER_f;
LINK_3:;
result_1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_1);
ELSE_1:;
result_2=muxtemp;
result_2;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_3));
PROCEED_3:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
cease(  ::TopLevel::y );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
