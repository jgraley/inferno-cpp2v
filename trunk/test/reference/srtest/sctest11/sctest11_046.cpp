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
int x;
/*temp*/ void *f_link;
int y;
private:
void *link;
public:
/*temp*/ int f_i;
private:
int i;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ bool result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ void *temp_link;
/*temp*/ int result5;
/*temp*/ bool result6;
/*temp*/ bool ortemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result3);
goto THEN_ELSE;
THEN_ELSE:;
result2=andtemp;
result2;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED_ELSE));
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result4= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result4);
goto PROCEED_ELSE;
PROCEED_ELSE:;
result6=ortemp;
result6;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
goto ELSE;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
LINK3:;
result1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result1);
goto ELSE;
ELSE:;
result=muxtemp;
result;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::TopLevel::y );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}