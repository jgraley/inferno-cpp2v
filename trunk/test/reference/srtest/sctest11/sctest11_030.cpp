#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
/*temp*/ int f_return;
void T();
int x;
/*temp*/ void *f_link1;
private:
void *link;
public:
/*temp*/ int f_i;
int y;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
{
/*temp*/ bool andtemp;
/*temp*/ bool result;
andtemp=(++ ::TopLevel::x);
{
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
/*temp*/ int result1;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
}
LINK:;
}
result1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result1);
}
goto ELSE;
THEN:;
;
ELSE:;
}
result=andtemp;
result;
}
{
/*temp*/ bool result2;
/*temp*/ bool ortemp;
ortemp=(!(++ ::TopLevel::x));
{
goto *((!ortemp) ? (&&THEN1) : (&&PROCEED1));
PROCEED1:;
;
goto ELSE1;
THEN1:;
{
/*temp*/ int result3;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
}
}
LINK1:;
}
result3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result3);
}
ELSE1:;
}
result2=ortemp;
result2;
}
{
 ::TopLevel::x=(0);
{
goto *((!( ::TopLevel::x<(2))) ? (&&THEN3) : (&&PROCEED2));
PROCEED2:;
{
NEXT:;
{
{
/*temp*/ int result4;
/*temp*/ int muxtemp;
{
goto *((!( ::TopLevel::x++)) ? (&&THEN2) : (&&PROCEED3));
PROCEED3:;
{
/*temp*/ int result5;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
}
}
LINK2:;
}
result5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result5);
}
goto ELSE2;
THEN2:;
{
/*temp*/ int result6;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
}
}
LINK3:;
}
result6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result6);
}
ELSE2:;
}
result4=muxtemp;
result4;
}
CONTINUE:;
;
}
CONTINUE1:;
goto *(( ::TopLevel::x<(2)) ? (&&NEXT) : (&&PROCEED4));
PROCEED4:;
}
goto ELSE3;
THEN3:;
;
ELSE3:;
}
}
cease(  ::TopLevel::y );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
{
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
