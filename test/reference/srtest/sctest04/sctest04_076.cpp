#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_5);
}
enum id_1
{
id_2 = 0U,
id_3 = 1U,
id_4 = 2U,
};
void id_5();
private:
unsigned int id_9;
};
id_0 id_10("id_10");
int id_7;
int id_8;

void id_0::id_5()
{
/*temp*/ bool id_6 = true;
if( (sc_delta_count())==(0U) )
{
 ::id_7=(1);
 ::id_8=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_9=((!( ::id_8<(5))) ?  ::id_0::id_3 :  ::id_0::id_2);
id_6=(false);
}
if( id_6 )
{
if(  ::id_0::id_2== ::id_0::id_9 )
{
 ::id_7+= ::id_8;
next_trigger(SC_ZERO_TIME);
 ::id_0::id_9= ::id_0::id_4;
id_6=(false);
}
}
if( id_6 )
{
if(  ::id_0::id_4== ::id_0::id_9 )
{
 ::id_7*=(2);
 ::id_8++;
 ::id_0::id_9=(( ::id_8<(5)) ?  ::id_0::id_2 :  ::id_0::id_3);
}
}
if( id_6 )
{
if(  ::id_0::id_3== ::id_0::id_9 )
{
cease(  ::id_7 );
id_6=(false);
}
}
if( id_6 )
next_trigger(SC_ZERO_TIME);
}
