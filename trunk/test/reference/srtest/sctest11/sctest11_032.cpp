#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
int y;
/*temp*/ int f_return;
private:
void *link;
public:
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ void *f_link1;
private:
int i;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ bool result2;
/*temp*/ int muxtemp;
/*temp*/ bool andtemp;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ bool result5;
/*temp*/ int result6;
/*temp*/ void *temp_link;
/*temp*/ bool ortemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result3= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result3);
goto ELSE;
THEN:;
;
ELSE:;
result2=andtemp;
result2;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
;
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result4= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result4);
ELSE1:;
result5=ortemp;
result5;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&THEN3) : (&&PROCEED2));
PROCEED2:;
NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN2) : (&&PROCEED3));
PROCEED3:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result6);
goto ELSE2;
THEN2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
LINK3:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
ELSE2:;
result1=muxtemp;
result1;
CONTINUE:;
;
CONTINUE1:;
goto *(( ::TopLevel::x<(2)) ? (&&NEXT) : (&&PROCEED4));
PROCEED4:;
goto ELSE3;
THEN3:;
;
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
