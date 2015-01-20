#include "isystemc.h"

class TopLevel;
int j;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
/*temp*/ int helper_n;
/*temp*/ unsigned int otherhelper_link;
/*temp*/ unsigned int helper_link;
private:
unsigned int (link_stack[10U]);
int (n_stack[10U]);
public:
/*temp*/ unsigned int helper_link_1;
private:
unsigned int helper_stack_index;
public:
void T();
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 8U,
T_STATE_YIELD = 6U,
T_STATE_PROCEED_NEXT = 5U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_NEXT_1 = 0U,
T_STATE_YIELD_1 = 2U,
T_STATE_ENTER_otherhelper = 9U,
T_STATE_PROCEED_THEN_ELSE_1 = 7U,
T_STATE_ENTER_helper = 4U,
};
private:
unsigned int link;
public:
/*temp*/ unsigned int otherhelper_link_1;
};
int i;
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ unsigned int temp_link;
/*temp*/ unsigned int temp_link_1;
auto unsigned int state;
static const unsigned int (lmap[]) = { &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT, &&ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT };
ENTER_otherhelper_LINK_PROCEED_THEN_ELSE_YIELD_PROCEED_NEXT_ENTER_helper_PROCEED_THEN_ELSE_YIELD_LINK_PROCEED_NEXT:;
if( (0U)==(sc_delta_count()) )
{
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
state=((!( ::i<(4))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED_NEXT_1);
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT_1 )
{
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link_1= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_LINK )
{
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD_1;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD_1 )
{
 ::i=((1)+ ::i);
state=(( ::i<(4)) ?  ::TopLevel::T_STATE_PROCEED_NEXT_1 :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE )
{
cease(  ::gvar );
return ;
state= ::TopLevel::T_STATE_ENTER_helper;
}
if( state== ::TopLevel::T_STATE_ENTER_helper )
{
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
state=((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ?  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 :  ::TopLevel::T_STATE_PROCEED_NEXT);
}
if( state== ::TopLevel::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::TopLevel::T_STATE_YIELD )
{
 ::gvar=((1)+ ::gvar);
 ::j=( ::j+(1));
state=(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ?  ::TopLevel::T_STATE_PROCEED_NEXT :  ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1);
}
if( state== ::TopLevel::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::TopLevel::otherhelper_link_1= ::TopLevel::T_STATE_LINK_1;
state= ::TopLevel::T_STATE_ENTER_otherhelper;
}
if( state== ::TopLevel::T_STATE_LINK_1 )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
state=temp_link;
}
if( state== ::TopLevel::T_STATE_ENTER_otherhelper )
{
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
state=temp_link_1;
}
goto *(lmap[state]);
}
