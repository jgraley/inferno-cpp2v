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
T_STATE_ENTER_f = 1,
T_STATE_LINK = 0,
};
void T();
private:
int i;
unsigned int link;
unsigned int state;
public:
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int temp_i;
do
{
if( (sc_delta_count())==(0) )
{
temp_i=(1);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
result= ::TopLevel::f_return;
result_1=result;
cease( result_1 );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::f_return= ::TopLevel::i;
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
