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
int x;
private:
void *link;
public:
void T();
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
void (f)();
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
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 4:;
 ::TopLevel::x=(44);
case 0:;
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch( 2 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 2:;
if( ((0)== ::TopLevel::x)||( ::TopLevel::x==(2)) )
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
 ::TopLevel::link= ::TopLevel::f_link1;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
return ;
}
}
}