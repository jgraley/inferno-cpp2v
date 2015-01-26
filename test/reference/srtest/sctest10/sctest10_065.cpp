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
/*temp*/ char f_k;
private:
unsigned int link;
public:
int x;
private:
int i;
public:
/*temp*/ unsigned int f_link;
private:
char k;
short j;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link1;
/*temp*/ int f_i;
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
T_STATE_ENTER_f = 3U,
};
private:
int t;
public:
/*temp*/ short f_j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int temp_j;
/*temp*/ unsigned int temp_link;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int temp_j1;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_k;
auto unsigned int state;
/*temp*/ int temp_k1;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
/*temp*/ int temp_i;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_k2;
/*temp*/ int temp_j2;
/*temp*/ int temp_i1;
/*temp*/ int result9;
/*temp*/ int temp_i2;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k=(8);
temp_j1=(6);
temp_j=(0);
temp_i1=(0);
temp_k2=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
ENTER_f_LINK_LINK_LINK:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result3= ::TopLevel::f_return;
result4=result3;
result6=result4;
result9=result6;
temp_i=result9;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result8= ::TopLevel::f_return;
result5=result8;
result10=result5;
result2=result10;
 ::TopLevel::x=result2;
temp_i2=(1);
temp_k1=(3);
temp_j2=(2);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result1= ::TopLevel::f_return;
result=result1;
result7=result;
result11=result7;
cease(  ::TopLevel::x+(result11*(2)) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
