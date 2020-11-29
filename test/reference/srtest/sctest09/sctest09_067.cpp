#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
recurser_stack_index(0U)
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_ENTER_recurser = 1U,
T_STATE_PROCEED = 2U,
T_STATE_LINK1 = 3U,
T_STATE_LINK_THEN_ELSE = 4U,
};
int x;
void T();
private:
unsigned int recurser_stack_index;
unsigned int (link_stack[10U]);
int (i_stack[10U]);
public:
/*temp*/ unsigned int recurser_link;
/*temp*/ int recurser_i;
/*temp*/ unsigned int recurser_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&
#error identifier LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK undeclared not supported in RenderIdentifier
, &&
#error identifier LINK_THEN_ELSE_LINK_PROCEED_ENTER_recurser_LINK undeclared not supported in RenderIdentifier
 };
auto unsigned int state;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
/*temp*/ int temp_i;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i2=(1);
 ::TopLevel::recurser_i=temp_i2;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_recurser;
continue;
}
if( state== ::TopLevel::T_STATE_LINK )
{
cease(  ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_ENTER_recurser )
{
 ::TopLevel::recurser_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_link1;
( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])= ::TopLevel::recurser_i;
 ::TopLevel::x++;
state=((!(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])<(5))) ?  ::TopLevel::T_STATE_LINK_THEN_ELSE :  ::TopLevel::T_STATE_PROCEED);
}
if( state== ::TopLevel::T_STATE_PROCEED )
{
temp_i1=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i1;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
temp_i=(( ::TopLevel::i_stack[ ::TopLevel::recurser_stack_index])+(1));
 ::TopLevel::recurser_i=temp_i;
 ::TopLevel::recurser_link1= ::TopLevel::T_STATE_LINK_THEN_ELSE;
state= ::TopLevel::T_STATE_ENTER_recurser;
}
if( state== ::TopLevel::T_STATE_LINK_THEN_ELSE )
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::recurser_stack_index]);
 ::TopLevel::recurser_stack_index--;
state=temp_link;
}
}
while( true );
}
