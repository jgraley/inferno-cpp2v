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
int i;
public:
int x;
/*temp*/ int f_return;
/*temp*/ int f_i;
int y;
void T();
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ bool result3;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ int result6;
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ void *temp_link;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result2= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result2);
goto THEN_ELSE;
THEN_ELSE:;
result3=andtemp;
result3;
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
result5=ortemp;
result5;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result);
goto ELSE;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
ELSE:;
result4=muxtemp;
result4;
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
