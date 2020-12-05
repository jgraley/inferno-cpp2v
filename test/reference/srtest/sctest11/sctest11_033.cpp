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
/*temp*/ void *f_link1;
/*temp*/ void *f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ bool andtemp;
/*temp*/ int muxtemp;
/*temp*/ bool ortemp;
/*temp*/ bool result5;
/*temp*/ int result;
/*temp*/ bool result6;
/*temp*/ int result1;
/*temp*/ int result4;
/*temp*/ int result2;
/*temp*/ int result3;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
goto ELSE;
THEN:;
ELSE:;
result5=andtemp;
result5;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result1= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result1);
ELSE1:;
result6=ortemp;
result6;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&THEN2) : (&&PROCEED2));
PROCEED2:;
NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN3) : (&&PROCEED3));
PROCEED3:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result2= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result2);
goto ELSE2;
THEN3:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
LINK3:;
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result3);
ELSE2:;
result4=muxtemp;
result4;
CONTINUE:;
CONTINUE1:;
goto *(( ::TopLevel::x<(2)) ? (&&NEXT) : (&&PROCEED4));
PROCEED4:;
goto ELSE3;
THEN2:;
ELSE3:;
cease(  ::TopLevel::y );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
