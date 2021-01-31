#include "isystemc.h"

class id_0;
class id_0 : public sc_module
{
public:
SC_CTOR( id_0 )
{
SC_METHOD(id_7);
}
enum id_1
{
id_2 = 0U,
id_3 = 2U,
id_4 = 1U,
id_5 = 4U,
id_6 = 3U,
};
void id_7();
private:
unsigned int id_10;
};
id_0 id_12("id_12");
int id_11;
int id_8;
int id_9;

void id_0::id_7()
{
if( (sc_delta_count())==(0U) )
{
 ::id_8=(1);
 ::id_9=(0);
next_trigger(SC_ZERO_TIME);
 ::id_0::id_10=((!( ::id_9<(4))) ?  ::id_0::id_4 :  ::id_0::id_2);
return ;
}
if(  ::id_0::id_2== ::id_0::id_10 )
{
 ::id_8+= ::id_9;
 ::id_11=(0);
 ::id_0::id_10=((!( ::id_11<(3))) ?  ::id_0::id_5 :  ::id_0::id_3);
}
if(  ::id_0::id_3== ::id_0::id_10 )
{
next_trigger(SC_ZERO_TIME);
 ::id_0::id_10= ::id_0::id_6;
return ;
}
if(  ::id_0::id_6== ::id_0::id_10 )
{
 ::id_8++;
 ::id_11++;
 ::id_0::id_10=(( ::id_11<(3)) ?  ::id_0::id_3 :  ::id_0::id_5);
}
if(  ::id_0::id_5== ::id_0::id_10 )
{
 ::id_8*=(2);
 ::id_9++;
 ::id_0::id_10=(( ::id_9<(4)) ?  ::id_0::id_2 :  ::id_0::id_4);
}
if(  ::id_0::id_4== ::id_0::id_10 )
{
cease(  ::id_8 );
return ;
}
next_trigger(SC_ZERO_TIME);
}
