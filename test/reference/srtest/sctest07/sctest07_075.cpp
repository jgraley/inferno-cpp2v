#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_8);
}
enum id_1
{
id_2 = 0U,
id_3 = 1U,
id_4 = 3U,
id_5 = 5U,
id_6 = 2U,
id_7 = 4U,
};
void id_8();
private:
unsigned int id_11;
};
id_0 id_13("id_13");
int id_10;
int id_12;
int id_9;

void id_0::id_8()
{
if( (sc_delta_count())==(0U) )
{
 ::id_9=(1);
 ::id_10=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11=((!( ::id_10<(4))) ?  ::id_0::id_5 :  ::id_0::id_2);
return ;
}
if(  ::id_0::id_2== ::id_0::id_11 )
{
 ::id_9+= ::id_10;
 ::id_12=(0);
 ::id_0::id_11=((!( ::id_12<(3))) ?  ::id_0::id_4 :  ::id_0::id_3);
}
if(  ::id_0::id_3== ::id_0::id_11 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11= ::id_0::id_6;
return ;
}
if(  ::id_0::id_6== ::id_0::id_11 )
{
 ::id_9++;
 ::id_12++;
 ::id_0::id_11=(( ::id_12<(3)) ?  ::id_0::id_3 :  ::id_0::id_4);
}
if(  ::id_0::id_4== ::id_0::id_11 )
{
 ::id_9*=(2);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_11= ::id_0::id_7;
return ;
}
if(  ::id_0::id_7== ::id_0::id_11 )
{
 ::id_10++;
 ::id_0::id_11=(( ::id_10<(4)) ?  ::id_0::id_2 :  ::id_0::id_5);
}
if(  ::id_0::id_5== ::id_0::id_11 )
{
cease(  ::id_9 );
return ;
}
next_trigger(SC_ZERO_TIME);
}
