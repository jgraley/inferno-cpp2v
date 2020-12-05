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
void *link;
public:
int x;
/*temp*/ void *f_link1;
/*temp*/ void *f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto int switch_value;
auto int switch_value1;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ int result;
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch_value=(0);
goto *(((0)==switch_value) ? (&&CASE) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value) ? (&&CASE1) : (&&PROCEED1));
PROCEED1:;
goto *(((1)==switch_value) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE1:;
 ::TopLevel::x=(44);
goto CASE;
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value1=(2);
goto *(((2)==switch_value1) ? (&&CASE2) : (&&PROCEED2));
PROCEED2:;
goto *(((1)==switch_value1) ? (&&PROCEED_CASE1) : (&&PROCEED_CASE1));
PROCEED_CASE1:;
 ::TopLevel::x=(99);
goto THEN_ELSE_BREAK;
CASE2:;
goto *((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED3));
PROCEED3:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED4));
PROCEED4:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
goto ELSE;
ELSE:;
result1=muxtemp;
 ::TopLevel::x=result1;
goto THEN_ELSE_BREAK;
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
