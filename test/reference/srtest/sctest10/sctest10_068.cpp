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
T_STATE_LINK = 0U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 2U,
};
void T();
private:
char k;
short j;
int i;
int t;
public:
int x;
private:
unsigned int link;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ unsigned int f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
auto unsigned int state;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_k;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i=(0);
temp_k=(0);
temp_j=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
result1=result;
result2=result1;
result3=result2;
temp_i1=result3;
temp_k1=(8);
temp_j1=(6);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result4= ::TopLevel::f_return;
result5=result4;
result6=result5;
result7=result6;
 ::TopLevel::x=result7;
temp_i2=(1);
temp_j2=(2);
temp_k2=(3);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2==state )
{
result8= ::TopLevel::f_return;
result9=result8;
result10=result9;
result11=result10;
cease(  ::TopLevel::x+((2)*result11) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
