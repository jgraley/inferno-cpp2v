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
int x;
/*temp*/ int f_return;
/*temp*/ void *f_link;
void (f)();
/*temp*/ void *f_link1;
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
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 4:;
 ::TopLevel::x=(44);
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch( 2 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 2:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : ({ {
{
 ::TopLevel::f_link1=(&&LINK);
 ::TopLevel::f();
}
LINK:;
}
 ::TopLevel::f_return; }));
break;
}
cease(  ::TopLevel::x );
return ;
}

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link=link;
return ;
}
}
}