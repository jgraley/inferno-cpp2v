#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_LINK = 2U,
};
void T();
private:
int i;
short j;
char k;
unsigned int link;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
auto unsigned int state;
/*temp*/ int result4;
/*temp*/ int result3;
/*temp*/ int result8;
/*temp*/ int result2;
/*temp*/ int result6;
/*temp*/ int result11;
/*temp*/ int result5;
/*temp*/ int result7;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int temp_i;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_k2;
/*temp*/ int temp_k1;
/*temp*/ int temp_k;
ENTER_f_LINK_LINK_LINK:;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i2=(0);
temp_k2=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_j=temp_j1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result5= ::TopLevel::f_return;
result6=result5;
result11=result6;
result2=result11;
temp_i1=result2;
temp_j=(6);
temp_k1=(8);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_j=temp_j;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2==state )
{
result10= ::TopLevel::f_return;
result9=result10;
result7=result9;
result8=result7;
 ::TopLevel::x=result8;
temp_i=(1);
temp_j2=(2);
temp_k=(3);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_j=temp_j2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result1= ::TopLevel::f_return;
result3=result1;
result=result3;
result4=result;
cease(  ::TopLevel::x+((2)*result4) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
