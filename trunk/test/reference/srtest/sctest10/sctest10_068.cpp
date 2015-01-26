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
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 2U,
};
private:
char k;
public:
void T();
private:
int i;
int t;
public:
int x;
/*temp*/ int f_i;
private:
short j;
public:
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link1;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
auto unsigned int state;
/*temp*/ int temp_k;
/*temp*/ int result2;
static const unsigned int (lmap[]) = { &&, &&, &&, && };
/*temp*/ int temp_k1;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_j;
/*temp*/ int result5;
/*temp*/ int temp_j1;
/*temp*/ int result6;
/*temp*/ int temp_i1;
/*temp*/ int temp_k2;
/*temp*/ int temp_i2;
/*temp*/ int result7;
/*temp*/ int temp_j2;
/*temp*/ int result8;
/*temp*/ unsigned int temp_link;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result11;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_k1=(8);
temp_j1=(6);
temp_j=(0);
temp_i2=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result9= ::TopLevel::f_return;
result7=result9;
result10=result7;
result6=result10;
temp_i=result6;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result8= ::TopLevel::f_return;
result3=result8;
result11=result3;
result5=result11;
 ::TopLevel::x=result5;
temp_i1=(1);
temp_k2=(3);
temp_j2=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result4= ::TopLevel::f_return;
result1=result4;
result2=result1;
result=result2;
cease( ((2)*result)+ ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
