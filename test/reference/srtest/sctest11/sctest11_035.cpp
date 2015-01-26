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
int x;
int y;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
private:
int i;
public:
/*temp*/ int f_i;
private:
void *link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ bool ortemp;
/*temp*/ int result1;
/*temp*/ bool andtemp;
/*temp*/ int result2;
/*temp*/ void *temp_link;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ bool result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN_ELSE) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result2= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result2);
goto THEN_ELSE;
THEN_ELSE:;
result5=andtemp;
result5;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
ELSE:;
result6=ortemp;
result6;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&THEN_ELSE1) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result1);
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
LINK3:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
ELSE1:;
result4=muxtemp;
result4;
goto *(( ::TopLevel::x<(2)) ? (&&PROCEED_NEXT) : (&&PROCEED3));
PROCEED3:;
goto THEN_ELSE1;
THEN_ELSE1:;
cease(  ::TopLevel::y );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
