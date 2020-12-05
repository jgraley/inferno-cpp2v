#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int temp_link;
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
unsigned int state;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
/*temp*/ int result7;
/*temp*/ int result5;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result8;
/*temp*/ int result2;
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int result6;
/*temp*/ int result9;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
/*temp*/ int temp_i;
/*temp*/ int temp_j;
/*temp*/ int temp_j2;
/*temp*/ int temp_j1;
/*temp*/ int temp_k;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i1=(0);
 ::TopLevel::temp_k=(0);
 ::TopLevel::temp_j2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK1== ::TopLevel::state )
{
 ::TopLevel::result10= ::TopLevel::f_return;
 ::TopLevel::result8= ::TopLevel::result10;
 ::TopLevel::result2= ::TopLevel::result8;
 ::TopLevel::result4= ::TopLevel::result2;
 ::TopLevel::temp_i2= ::TopLevel::result4;
 ::TopLevel::temp_j=(6);
 ::TopLevel::temp_k1=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result9= ::TopLevel::f_return;
 ::TopLevel::result6= ::TopLevel::result9;
 ::TopLevel::result11= ::TopLevel::result6;
 ::TopLevel::result3= ::TopLevel::result11;
 ::TopLevel::x= ::TopLevel::result3;
 ::TopLevel::temp_i=(1);
 ::TopLevel::temp_j1=(2);
 ::TopLevel::temp_k2=(3);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::result5= ::TopLevel::result1;
 ::TopLevel::result= ::TopLevel::result5;
 ::TopLevel::result7= ::TopLevel::result;
cease(  ::TopLevel::x+((2)* ::TopLevel::result7) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
