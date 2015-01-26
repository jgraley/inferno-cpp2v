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
/*temp*/ unsigned int f_link;
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
};
private:
char k;
public:
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ int f_i;
private:
int i;
int t;
public:
/*temp*/ int f_return;
private:
short j;
public:
int x;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int temp_k1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ unsigned int temp_link;
/*temp*/ int result4;
/*temp*/ int temp_k2;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_j1;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_j2;
/*temp*/ int result9;
auto unsigned int state;
/*temp*/ int result10;
/*temp*/ int result11;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_k=(8);
temp_j2=(6);
temp_j=(0);
temp_i=(0);
temp_k1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result9= ::TopLevel::f_return;
result5=result9;
result3=result5;
result8=result3;
temp_i2=result8;
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result2= ::TopLevel::f_return;
result11=result2;
result=result11;
result10=result;
 ::TopLevel::x=result10;
temp_i1=(1);
temp_k2=(3);
temp_j1=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result7= ::TopLevel::f_return;
result1=result7;
result4=result1;
result6=result4;
cease(  ::TopLevel::x+(result6*(2)) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(((5)* ::TopLevel::k)+ ::TopLevel::t);
temp_link= ::TopLevel::link;
state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
