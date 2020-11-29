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
T_STATE_BREAK = 5U,
T_STATE_CASE2 = 3U,
T_STATE_CASE1 = 4U,
T_STATE_CASE = 8U,
T_STATE_ELSE = 13U,
T_STATE_ENTER_f = 15U,
T_STATE_LINK = 12U,
T_STATE_PROCEED3 = 0U,
T_STATE_PROCEED2 = 1U,
T_STATE_PROCEED = 6U,
T_STATE_PROCEED1 = 9U,
T_STATE_PROCEED4 = 10U,
T_STATE_PROCEED_CASE1 = 2U,
T_STATE_PROCEED_CASE = 7U,
T_STATE_THEN = 11U,
T_STATE_THEN_ELSE_BREAK = 14U,
};
void T();
private:
unsigned int link;
public:
int x;
/*temp*/ unsigned int f_link1;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
, &&
#error identifier ENTER_f_THEN_ELSE_BREAK_ELSE_LINK_THEN_PROCEED_PROCEED_CASE_PROCEED_CASE_PROCEED_BREAK_CASE_CASE_PROCEED_CASE_PROCEED_PROCEED undeclared not supported in RenderIdentifier
 };
auto unsigned int state;
auto int switch_value1;
auto int switch_value;
/*temp*/ int muxtemp;
/*temp*/ int result1;
/*temp*/ int result;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch_value1=(0);
wait(SC_ZERO_TIME);
state=((switch_value1==(0)) ?  ::TopLevel::T_STATE_CASE1 :  ::TopLevel::T_STATE_PROCEED3);
continue;
}
if( state== ::TopLevel::T_STATE_PROCEED3 )
{
state=((switch_value1==(4)) ?  ::TopLevel::T_STATE_CASE2 :  ::TopLevel::T_STATE_PROCEED2);
}
if( state== ::TopLevel::T_STATE_PROCEED2 )
{
state=((switch_value1==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE1 :  ::TopLevel::T_STATE_PROCEED_CASE1);
}
if( state== ::TopLevel::T_STATE_PROCEED_CASE1 )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE2 )
{
 ::TopLevel::x=(44);
state= ::TopLevel::T_STATE_CASE1;
}
if( state== ::TopLevel::T_STATE_CASE1 )
{
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
state= ::TopLevel::T_STATE_BREAK;
}
if( state== ::TopLevel::T_STATE_BREAK )
{
switch_value=(2);
state=((switch_value==(2)) ?  ::TopLevel::T_STATE_CASE :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
state=((switch_value==(1)) ?  ::TopLevel::T_STATE_PROCEED_CASE :  ::TopLevel::T_STATE_PROCEED_CASE);
}
if( state== ::TopLevel::T_STATE_PROCEED_CASE )
{
 ::TopLevel::x=(99);
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( state== ::TopLevel::T_STATE_CASE )
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ?  ::TopLevel::T_STATE_THEN_ELSE_BREAK :  ::TopLevel::T_STATE_PROCEED1);
}
if( state== ::TopLevel::T_STATE_PROCEED1 )
{
state=((!(false)) ?  ::TopLevel::T_STATE_THEN :  ::TopLevel::T_STATE_PROCEED4);
}
if( state== ::TopLevel::T_STATE_PROCEED4 )
{
muxtemp=(88);
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_THEN )
{
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result= ::TopLevel::f_return;
muxtemp=result;
state= ::TopLevel::T_STATE_ELSE;
}
if( state== ::TopLevel::T_STATE_ELSE )
{
result1=muxtemp;
 ::TopLevel::x=result1;
state= ::TopLevel::T_STATE_THEN_ELSE_BREAK;
}
if( state== ::TopLevel::T_STATE_THEN_ELSE_BREAK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
