#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
recurser_stack_index(0)
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_recurser = 4,
T_STATE_LINK = 1,
T_STATE_LINK_1 = 2,
T_STATE_LINK_THEN_ELSE = 3,
T_STATE_PROCEED = 0,
};
void T();
private:
int (i_stack[10]);
unsigned int (link_stack[10]);
public:
int x;
private:
unsigned int recurser_stack_index;
public:
/*temp*/ int recurser_i;
/*temp*/ unsigned int recurser_link;
/*temp*/ unsigned int recurser_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK, &&LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK };
auto unsigned int state;
/*temp*/ int temp_i;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK:;
if( (sc_delta_count())==(0) )
{
 ::TopLevel::x=(0);
temp_i=(1);
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
goto *(lmap[state]);
}
if(  ::TopLevel::T_STATE_LINK_1==state )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::T_STATE_ENTER_recurser==state )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if(  ::TopLevel::T_STATE_PROCEED==state )
{
temp_i_1=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i_1;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
temp_i_2=((1)+( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index]));
 ::TopLevel::recurser_i=temp_i_2;
 ::TopLevel::recurser_link= ::TopLevel::T_STATE_LINK_THEN_ELSE;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if(  ::TopLevel::T_STATE_LINK_THEN_ELSE==state )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
state=temp_link;
}
goto *(lmap[state]);
}
