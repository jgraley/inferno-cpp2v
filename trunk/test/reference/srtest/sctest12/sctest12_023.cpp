#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
void *link;
public:
int x;
/*temp*/ void *f_link;
void T();
/*temp*/ void *f_link_1;
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
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
goto BREAK_1;
case 4:;
 ::TopLevel::x=(44);
case 0:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
BREAK:;
}
BREAK_1:;
}
{
{
switch( 2 )
{
case 1:;
 ::TopLevel::x=(99);
goto BREAK_3;
case 2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=({ /*temp*/ int muxtemp; if( false )
muxtemp=(88);
else
muxtemp=({ {
{
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
}
LINK:;
}
 ::TopLevel::f_return; });
muxtemp; });
goto BREAK_2;
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
 ::TopLevel::link= ::TopLevel::f_link_1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
