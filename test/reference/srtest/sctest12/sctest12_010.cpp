#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (f)(auto void *link);
void T();
int x;
/*temp*/ int f_return;
/*temp*/ void *f_link;
};
TopLevel top_level("top_level");

void (TopLevel::f)(void *link)
{
/*temp*/ void *temp_link;
{
 ::TopLevel::f_return=(3);
{
temp_link=link;
return ;
}
}
}

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
 ::TopLevel::f(&&LINK);
LINK:;
}
 ::TopLevel::f_return; }));
break;
}
cease(  ::TopLevel::x );
return ;
}