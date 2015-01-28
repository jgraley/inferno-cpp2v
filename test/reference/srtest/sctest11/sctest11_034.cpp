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
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ void *f_link1;
private:
int i;
void *link;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int muxtemp;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ bool ortemp;
/*temp*/ bool result4;
/*temp*/ int result5;
/*temp*/ bool andtemp;
/*temp*/ bool result6;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result5= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result5);
goto ELSE;
THEN:;
ELSE:;
result6=andtemp;
result6;
ortemp=(!(++ ::TopLevel::x));
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
goto ELSE1;
THEN1:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
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
result1= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result1);
goto ELSE2;
THEN2:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK3);
goto ENTER_f;
LINK3:;
result= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result);
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
