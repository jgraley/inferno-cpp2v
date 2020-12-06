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
int i;
public:
int x;
int y;
private:
void *link;
public:
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(4);
({ /*temp*/ bool andtemp; andtemp=(++ ::TopLevel::x); {
if( !andtemp )
goto THEN;
goto PROCEED;
PROCEED:;
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
goto ELSE;
THEN:;
;
ELSE:;
}
andtemp; });
({ /*temp*/ bool ortemp; ortemp=(!(++ ::TopLevel::x)); {
if( !ortemp )
goto THEN1;
goto PROCEED1;
PROCEED1:;
;
goto ELSE1;
THEN1:;
ortemp=( ::TopLevel::y+=({ {
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
ELSE1:;
}
ortemp; });
{
 ::TopLevel::x=(0);
{
if( !( ::TopLevel::x<(2)) )
goto THEN2;
goto PROCEED2;
PROCEED2:;
{
NEXT:;
{
({ /*temp*/ int muxtemp; {
if( !( ::TopLevel::x++) )
goto THEN3;
goto PROCEED3;
PROCEED3:;
muxtemp=( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
}
}
LINK2:;
}
 ::TopLevel::f_return; }));
goto ELSE2;
THEN3:;
muxtemp=( ::TopLevel::y-=({ {
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
ELSE2:;
}
muxtemp; });
CONTINUE:;
;
}
CONTINUE1:;
if(  ::TopLevel::x<(2) )
goto NEXT;
goto PROCEED4;
PROCEED4:;
}
goto ELSE3;
THEN2:;
;
ELSE3:;
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
