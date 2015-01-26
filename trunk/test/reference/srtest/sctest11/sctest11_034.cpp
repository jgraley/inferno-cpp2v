#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_return;
void T();
private:
void *link;
public:
/*temp*/ void *f_link;
private:
int i;
public:
int y;
int x;
/*temp*/ void *f_link1;
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ bool ortemp;
/*temp*/ void *temp_link;
/*temp*/ bool result5;
/*temp*/ bool andtemp;
/*temp*/ int result6;
/*temp*/ int muxtemp;
 ::TopLevel::x=(4);
andtemp=(++ ::TopLevel::x);
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
 ::TopLevel::f_i= ::TopLevel::x;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result2= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result2);
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
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
ELSE1:;
result=ortemp;
result;
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
result4= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result4);
ELSE2:;
result1=muxtemp;
result1;
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
