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
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result;
/*temp*/ bool result4;
/*temp*/ int result5;
/*temp*/ int result3;
/*temp*/ int result1;
/*temp*/ int result6;
/*temp*/ int result2;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result5);
goto THEN_ELSE;
THEN_ELSE:;
result4=andtemp;
result4;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result1);
PROCEED_ELSE:;
result=ortemp;
result;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result2);
goto ELSE;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
ELSE:;
result3=muxtemp;
result3;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
