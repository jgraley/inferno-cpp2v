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
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
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
if( (0)==switch_value )
goto CASE;
goto PROCEED;
PROCEED:;
if( (4)==switch_value )
goto CASE1;
goto PROCEED1;
PROCEED1:;
if( (1)==switch_value )
goto CASE2;
goto PROCEED2;
PROCEED2:;
CASE2:;
 ::TopLevel::x=(99);
goto BREAK;
CASE1:;
 ::TopLevel::x=(44);
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK1;
}
}
BREAK1:;
}
BREAK:;
}
{
{
{
auto int switch_value1;
switch_value1=(2);
{
if( (2)==switch_value1 )
goto CASE3;
goto PROCEED3;
PROCEED3:;
if( (1)==switch_value1 )
goto CASE4;
goto PROCEED4;
PROCEED4:;
CASE4:;
 ::TopLevel::x=(99);
goto BREAK2;
CASE3:;
{
if( !(((0)== ::TopLevel::x)||((2)== ::TopLevel::x)) )
goto THEN;
goto PROCEED5;
PROCEED5:;
 ::TopLevel::x=({ /*temp*/ int muxtemp; {
if( !(false) )
goto THEN1;
goto PROCEED6;
PROCEED6:;
muxtemp=(88);
goto ELSE;
THEN1:;
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
THEN:;
;
ELSE1:;
}
goto BREAK3;
}
}
BREAK3:;
}
BREAK2:;
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
