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
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_i;
/*temp*/ int f_return;
private:
int i;
public:
int y;
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ bool result1;
/*temp*/ bool result2;
/*temp*/ int muxtemp;
/*temp*/ int result3;
/*temp*/ void *temp_link;
/*temp*/ bool andtemp;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result4= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result4);
goto THEN_ELSE;
THEN_ELSE:;
result2=andtemp;
result2;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
goto PROCEED_ELSE;
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result6= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result6);
PROCEED_ELSE:;
result1=ortemp;
result1;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
goto ELSE;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result3);
ELSE:;
result=muxtemp;
result;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
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
