#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_i;
/*temp*/ void *f_link;
private:
void *link;
public:
/*temp*/ void *f_link1;
int y;
int x;
private:
int i;
public:
void T();
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
({ /*temp*/ bool andtemp; andtemp=(++ ::TopLevel::x); if( andtemp )
andtemp=( ::TopLevel::y=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
}
LINK:;
}
 ::TopLevel::f_return; }));
andtemp; });
(!(++ ::TopLevel::x))||( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
}
}
LINK1:;
}
 ::TopLevel::f_return; }));
{
 ::TopLevel::x=(0);
if(  ::TopLevel::x<(2) )
{
NEXT:;
{
( ::TopLevel::x++) ? ( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
}
}
LINK2:;
}
 ::TopLevel::f_return; })) : ( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK3);
goto ENTER_f;
}
}
LINK3:;
}
 ::TopLevel::f_return; }));
CONTINUE:;
;
}
CONTINUE1:;
if(  ::TopLevel::x<(2) )
goto NEXT;
}
}
cease(  ::TopLevel::y );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
{
 ::TopLevel::f_return=((100)/ ::TopLevel::i);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}