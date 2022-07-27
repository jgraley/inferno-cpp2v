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
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int temp_i;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0) )
{
 ::TopLevel::temp_i=(1);
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_i= ::TopLevel::temp_i;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_LINK== ::TopLevel::state) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::result_1= ::TopLevel::result;
cease(  ::TopLevel::result_1 );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state) )
{
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return= ::TopLevel::i;
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
