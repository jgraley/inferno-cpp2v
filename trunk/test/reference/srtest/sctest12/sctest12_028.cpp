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
void T();
/*temp*/ void *f_link;
/*temp*/ int f_return;
/*temp*/ void *f_link1;
private:
void *link;
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
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
{
auto int switch_value;
switch_value=(0);
{
goto *(((0)==switch_value) ? (&&CASE2) : (&&PROCEED));
PROCEED:;
goto *((switch_value==(4)) ? (&&CASE1) : (&&PROCEED1));
PROCEED1:;
goto *((switch_value==(1)) ? (&&CASE) : (&&PROCEED2));
PROCEED2:;
CASE:;
 ::TopLevel::x=(99);
goto BREAK1;
CASE1:;
 ::TopLevel::x=(44);
CASE2:;
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
}
BREAK:;
}
BREAK1:;
}
{
{
{
auto int switch_value1;
switch_value1=(2);
{
goto *((switch_value1==(2)) ? (&&CASE4) : (&&PROCEED3));
PROCEED3:;
goto *((switch_value1==(1)) ? (&&CASE3) : (&&PROCEED4));
PROCEED4:;
CASE3:;
 ::TopLevel::x=(99);
goto BREAK3;
CASE4:;
{
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN1) : (&&PROCEED5));
PROCEED5:;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
goto *((!(false)) ? (&&THEN) : (&&PROCEED6));
PROCEED6:;
muxtemp=(88);
goto ELSE;
THEN:;
muxtemp=({ {
{
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
ELSE:;
}
muxtemp; });
goto ELSE1;
THEN1:;
;
ELSE1:;
}
goto BREAK2;
}
}
BREAK2:;
}
BREAK3:;
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}