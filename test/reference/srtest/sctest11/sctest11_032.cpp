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
/*temp*/ bool result2;
/*temp*/ bool result3;
/*temp*/ int result6;
/*temp*/ int result4;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result5;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result6= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result6);
goto ELSE;
THEN:;
;
ELSE:;
result3=andtemp;
result3;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
;
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result);
ELSE1:;
result2=ortemp;
result2;
 ::TopLevel::x=(0);
goto *((!( ::TopLevel::x<(2))) ? (&&THEN3) : (&&PROCEED2));
PROCEED2:;
NEXT:;
goto *((!( ::TopLevel::x++)) ? (&&THEN2) : (&&PROCEED3));
PROCEED3:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
goto ELSE2;
THEN2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result1);
ELSE2:;
result4=muxtemp;
result4;
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
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
