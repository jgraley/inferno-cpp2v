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
/*temp*/ void *f_link1;
private:
void *link;
public:
/*temp*/ int f_return;
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
if( ((0)== ::TopLevel::x)||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
{
auto int switch_value;
switch_value=(0);
{
if( switch_value==(0) )
goto CASE2;
goto PROCEED;
PROCEED:;
if( switch_value==(4) )
goto CASE1;
goto PROCEED1;
PROCEED1:;
if( (1)==switch_value )
goto CASE;
goto PROCEED2;
PROCEED2:;
CASE:;
 ::TopLevel::x=(99);
goto BREAK1;
CASE1:;
 ::TopLevel::x=(44);
CASE2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
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
if( switch_value1==(2) )
goto CASE4;
goto PROCEED3;
PROCEED3:;
if( switch_value1==(1) )
goto CASE3;
goto PROCEED4;
PROCEED4:;
CASE3:;
 ::TopLevel::x=(99);
goto BREAK3;
CASE4:;
{
if( !(( ::TopLevel::x==(0))||( ::TopLevel::x==(2))) )
goto THEN1;
goto PROCEED5;
PROCEED5:;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
if( !(false) )
goto THEN;
goto PROCEED6;
PROCEED6:;
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