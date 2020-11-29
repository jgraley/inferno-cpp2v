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
/*temp*/ bool result4;
/*temp*/ bool result1;
/*temp*/ int result;
/*temp*/ int result2;
/*temp*/ int result5;
/*temp*/ int result3;
/*temp*/ int result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result);
goto ELSE;
THEN:;
ELSE:;
result1=andtemp;
result1;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result5= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result5);
ELSE1:;
result4=ortemp;
result4;
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
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result6);
goto ELSE2;
THEN2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result3= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result3);
ELSE2:;
result2=muxtemp;
result2;
goto *(( ::TopLevel::x<(2)) ? (&&NEXT) : (&&PROCEED4));
PROCEED4:;
goto ELSE3;
THEN3:;
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
