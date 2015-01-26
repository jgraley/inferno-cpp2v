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
char k;
public:
int x;
/*temp*/ int f_i;
private:
short j;
public:
/*temp*/ unsigned int f_link;
private:
int i;
public:
/*temp*/ short f_j;
private:
unsigned int link;
int t;
public:
/*temp*/ unsigned int f_link1;
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
};
/*temp*/ char f_k;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_k;
/*temp*/ int result;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int result1;
auto unsigned int state;
/*temp*/ int temp_i;
/*temp*/ int result2;
/*temp*/ int temp_i1;
/*temp*/ int result3;
/*temp*/ int temp_k1;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_j2;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_k2;
/*temp*/ int result9;
/*temp*/ int temp_i2;
/*temp*/ int result10;
/*temp*/ int result11;
ENTER_f_LINK_LINK_LINK:;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_k2=(0);
temp_i=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result8= ::TopLevel::f_return;
result11=result8;
result1=result11;
result6=result1;
temp_i1=result6;
temp_k=(8);
temp_j1=(6);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result3= ::TopLevel::f_return;
result9=result3;
result7=result9;
result4=result7;
 ::TopLevel::x=result4;
temp_j=(2);
temp_k1=(3);
temp_i2=(1);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result2= ::TopLevel::f_return;
result5=result2;
result10=result5;
result=result10;
cease(  ::TopLevel::x+((2)*result) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
