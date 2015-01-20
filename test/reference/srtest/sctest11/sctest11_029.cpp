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
/*temp*/ int f_i;
/*temp*/ void *f_link_1;
private:
int i;
public:
int x;
int y;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
({ /*temp*/ bool andtemp; andtemp=(++ ::TopLevel::x); {
goto *((!andtemp) ? (&&THEN) : (&&PROCEED));
PROCEED:;
andtemp=( ::TopLevel::y=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
}
LINK:;
}
 ::TopLevel::f_return; }));
goto ELSE;
THEN:;
;
ELSE:;
}
andtemp; });
({ /*temp*/ bool ortemp; ortemp=(!(++ ::TopLevel::x)); {
goto *((!ortemp) ? (&&THEN_1) : (&&PROCEED_1));
PROCEED_1:;
;
goto ELSE_1;
THEN_1:;
ortemp=( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_1);
goto ENTER_f;
}
}
LINK_1:;
}
 ::TopLevel::f_return; }));
ELSE_1:;
}
ortemp; });
{
 ::TopLevel::x=(0);
{
goto *((!( ::TopLevel::x<(2))) ? (&&THEN_3) : (&&PROCEED_2));
PROCEED_2:;
{
NEXT:;
{
({ /*temp*/ int muxtemp; {
goto *((!( ::TopLevel::x++)) ? (&&THEN_2) : (&&PROCEED_3));
PROCEED_3:;
muxtemp=( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_2);
goto ENTER_f;
}
}
LINK_2:;
}
 ::TopLevel::f_return; }));
goto ELSE_2;
THEN_2:;
muxtemp=( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK_3);
goto ENTER_f;
}
}
LINK_3:;
}
 ::TopLevel::f_return; }));
ELSE_2:;
}
muxtemp; });
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
