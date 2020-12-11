#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ unsigned int temp_link;
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
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
/*temp*/ int f_return;
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
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i=(0);
 ::TopLevel::temp_k=(0);
 ::TopLevel::temp_j=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
return ;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::result1= ::TopLevel::result;
 ::TopLevel::result2= ::TopLevel::result1;
 ::TopLevel::result3= ::TopLevel::result2;
 ::TopLevel::temp_i1= ::TopLevel::result3;
 ::TopLevel::temp_k1=(8);
 ::TopLevel::temp_j1=(6);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1== ::TopLevel::state )
{
 ::TopLevel::result4= ::TopLevel::f_return;
 ::TopLevel::result5= ::TopLevel::result4;
 ::TopLevel::result6= ::TopLevel::result5;
 ::TopLevel::result7= ::TopLevel::result6;
 ::TopLevel::x= ::TopLevel::result7;
 ::TopLevel::temp_i2=(1);
 ::TopLevel::temp_j2=(2);
 ::TopLevel::temp_k2=(3);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result8= ::TopLevel::f_return;
 ::TopLevel::result9= ::TopLevel::result8;
 ::TopLevel::result10= ::TopLevel::result9;
 ::TopLevel::result11= ::TopLevel::result10;
cease(  ::TopLevel::x+((2)* ::TopLevel::result11) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
next_trigger(SC_ZERO_TIME);
}
