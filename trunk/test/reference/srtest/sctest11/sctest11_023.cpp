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
int x;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
private:
int i;
public:
int y;
void T();
private:
void *link;
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
 ::TopLevel::f_link_1=(&&LINK);
goto ENTER_f;
}
}
LINK:;
}
 ::TopLevel::f_return; }));
andtemp; });
({ /*temp*/ bool ortemp; ortemp=(!(++ ::TopLevel::x)); if( ortemp )
;
else
ortemp=( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link_1=(&&LINK_1);
goto ENTER_f;
}
}
LINK_1:;
}
 ::TopLevel::f_return; }));
ortemp; });
{
 ::TopLevel::x=(0);
if(  ::TopLevel::x<(2) )
{
NEXT:;
{
({ /*temp*/ int muxtemp; if(  ::TopLevel::x++ )
muxtemp=( ::TopLevel::y+=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link_1=(&&LINK_2);
goto ENTER_f;
}
}
LINK_2:;
}
 ::TopLevel::f_return; }));
else
muxtemp=( ::TopLevel::y-=({ {
{
 ::TopLevel::f_i= ::TopLevel::x;
{
 ::TopLevel::f_link_1=(&&LINK_3);
goto ENTER_f;
}
}
LINK_3:;
}
 ::TopLevel::f_return; }));
muxtemp; });
CONTINUE:;
;
}
CONTINUE_1:;
if(  ::TopLevel::x<(2) )
goto NEXT;
}
}
cease(  ::TopLevel::y );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link_1;
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
