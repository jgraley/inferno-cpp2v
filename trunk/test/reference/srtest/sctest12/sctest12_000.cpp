#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int (f)();
int x;
void T();
};
TopLevel top_level("top_level");

int (TopLevel::f)()
{
return 3;
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
 ::TopLevel::x=((false) ? (88) : ( ::TopLevel::f()));
break;
}
cease(  ::TopLevel::x );
}