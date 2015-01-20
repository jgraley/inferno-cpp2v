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
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
/*temp*/ int f_return;
private:
void *link;
public:
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
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
{
{
{
auto int switch_value;
switch_value=(0);
{
goto *(((0)==switch_value) ? (&&CASE_2) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value) ? (&&CASE_1) : (&&PROCEED_1));
PROCEED_1:;
goto *((switch_value==(1)) ? (&&CASE) : (&&PROCEED_2));
PROCEED_2:;
CASE:;
 ::TopLevel::x=(99);
goto BREAK_1;
CASE_1:;
 ::TopLevel::x=(44);
CASE_2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
}
BREAK:;
}
BREAK_1:;
}
{
{
{
auto int switch_value_1;
switch_value_1=(2);
{
goto *((switch_value_1==(2)) ? (&&CASE_4) : (&&PROCEED_3));
PROCEED_3:;
goto *(((1)==switch_value_1) ? (&&CASE_3) : (&&PROCEED_4));
PROCEED_4:;
CASE_3:;
 ::TopLevel::x=(99);
goto BREAK_3;
CASE_4:;
{
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_1) : (&&PROCEED_5));
PROCEED_5:;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
goto *((!(false)) ? (&&THEN) : (&&PROCEED_6));
PROCEED_6:;
muxtemp=(88);
goto ELSE;
THEN:;
muxtemp=({ {
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
ELSE:;
}
muxtemp; });
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
goto BREAK_2;
}
}
BREAK_2:;
}
BREAK_3:;
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
