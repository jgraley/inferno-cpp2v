#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int y;
/*temp*/ int f_i;
void T();
/*temp*/ void *f_link;
private:
void *link;
public:
int x;
private:
int i;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link_1;
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
/*temp*/ int result_1;
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
result_1= ::TopLevel::f_return;
andtemp=( ::TopLevel::y=result_1);
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
/*temp*/ bool result_2;
/*temp*/ bool ortemp;
ortemp=(!(++ ::TopLevel::x));
{
goto *((!ortemp) ? (&&THEN_1) : (&&PROCEED_1));
PROCEED_1:;
;
goto ELSE_1;
THEN_1:;
{
/*temp*/ int result_3;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_1);
goto ENTER_f;
}
}
LINK_1:;
}
result_3= ::TopLevel::f_return;
ortemp=( ::TopLevel::y+=result_3);
}
ELSE_1:;
}
result_2=ortemp;
result_2;
}
{
 ::TopLevel::x=(0);
{
goto *((!( ::TopLevel::x<(2))) ? (&&THEN_3) : (&&PROCEED_2));
PROCEED_2:;
{
NEXT:;
{
{
/*temp*/ int result_4;
/*temp*/ int muxtemp;
{
goto *((!( ::TopLevel::x++)) ? (&&THEN_2) : (&&PROCEED_3));
PROCEED_3:;
{
/*temp*/ int result_5;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_2);
goto ENTER_f;
}
}
LINK_2:;
}
result_5= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y+=result_5);
}
goto ELSE_2;
THEN_2:;
{
/*temp*/ int result_6;
{
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_3);
goto ENTER_f;
}
}
LINK_3:;
}
result_6= ::TopLevel::f_return;
muxtemp=( ::TopLevel::y-=result_6);
}
ELSE_2:;
}
result_4=muxtemp;
result_4;
}
CONTINUE:;
;
}
CONTINUE_1:;
goto *(( ::TopLevel::x<(2)) ? (&&NEXT) : (&&PROCEED_4));
PROCEED_4:;
}
goto ELSE_3;
THEN_3:;
;
ELSE_3:;
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
